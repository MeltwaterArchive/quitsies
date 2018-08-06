/*
The MIT License (MIT)

Copyright (c) 2017 MediaSift Ltd.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <rocksdb/table.h>
#include <rocksdb/utilities/backupable_db.h>

#include <boost/filesystem.hpp>

#include <quitsies/db/rocks.hpp>

namespace quitsies { namespace db {

void
rocks::register_options(option_list & options)
{
	options.push_back(std::make_tuple("DB", option_array({
		option_ptr(new str_option('?', "db_path", "Path to store DB files.", &_path)),
		option_ptr(new int_option('?', "db_ttl", "A TTL for all DB items. WARN: This applies to old records.", &_ttl)),
		option_ptr(new int_option('?', "db_max_open_files", "Max # of open files, -1 is unlimited.", &_max_files)),
		option_ptr(new int_option('?', "db_memtable", "Set the memtable size. Higher == faster writes.", &_memtable)),
		option_ptr(new int_option('?', "db_shard_bits", "Set the block cache shard bits. 4 == 16 shards.", &_shard_bits)),
		option_ptr(new int_option('?', "db_block_cap", "Set the cap to the block_cache. Higher == faster reads.", &_block_cap)),
		option_ptr(new bool_option('d', "db_debug", "Collect more granular RocksDB metrics at a small cost to performance.", &_debug)),
		option_ptr(new bool_option('?', "db_write_mode", "Optimize RocksDB compaction for write amp.", &_write_mode)),
		option_ptr(new bool_option('?', "db_restore_backup", "Restore DB from a backup.", &_restore))
	})));
}

void
rocks::register_endpoints(served::multiplexer & mux)
{
	mux.handle("/stats").get([this](served::response & res, const served::request & req) {
		uint64_t num_keys = 0;
		_db->GetAggregatedIntProperty("rocksdb.estimate-num-keys", &num_keys);
		res << "rocksdb.estimate-num-keys COUNT : " << std::to_string(num_keys) << "\n";

		if ( _rocks_stats ) {
			res << _rocks_stats->ToString();
		}
	});

	mux.handle("/key/{key}")
		.get([this](served::response & res, const served::request & req) {
			std::string value;
			auto status = get(req.params["key"], &value);
			if ( status.ok() ) {
				res << value;
			} else if ( status.is_not_found() ) {
				res.set_status(served::status_4XX::NOT_FOUND);
			} else {
				res.set_status(served::status_5XX::INTERNAL_SERVER_ERROR);
				res << status.to_string();
				_log->error("failed to obtain key {}: {}", req.params["key"], status.to_string());
			}
		})
		.put([this](served::response & res, const served::request & req) {
			auto status = put(req.params["key"], req.body());
			if ( !status.ok() ) {
				res.set_status(served::status_5XX::INTERNAL_SERVER_ERROR);
				res << status.to_string();
				_log->error("failed to set key {}: {}", req.params["key"], status.to_string());
			}
		})
		.post([this](served::response & res, const served::request & req) {
			auto status = put(req.params["key"], req.body());
			if ( !status.ok() ) {
				res.set_status(served::status_5XX::INTERNAL_SERVER_ERROR);
				res << status.to_string();
				_log->error("failed to set key {}: {}", req.params["key"], status.to_string());
			}
		});

	mux.handle("/backup_create")
		.post([this](served::response & res, const served::request & req) {
			std::string backup_path = _path + "_backup";
			_log->info("attempting to create new backup at: {}", backup_path);

			rocksdb::BackupEngine* backup_engine;
			auto status = rocksdb::BackupEngine::Open(
					rocksdb::Env::Default(),
					rocksdb::BackupableDBOptions(backup_path),
					&backup_engine
			);
			if ( status.ok() ) {
				status = backup_engine->CreateNewBackup(_db);
				delete backup_engine;
			}
			if ( !status.ok() ) {
				_local_stats->counter("rocksdb.backup.create.error", 1);
				res.set_status(served::status_5XX::INTERNAL_SERVER_ERROR);
				res << status.ToString();
				_log->error("failed to create new backup: {}", status.ToString());
			} else {
				_local_stats->counter("rocksdb.backup.create.success", 1);
				res << "Success";
				_log->info("created new backup at: {}", backup_path);
			}
		});

	mux.handle("/backup_clean")
		.post([this](served::response & res, const served::request & req) {
			std::string backup_path = _path + "_backup";
			_log->info("attempting to purge old backups at: {}", backup_path);

			rocksdb::BackupEngine* backup_engine;
			auto status = rocksdb::BackupEngine::Open(
					rocksdb::Env::Default(),
					rocksdb::BackupableDBOptions(backup_path),
					&backup_engine
			);
			if ( status.ok() ) {
				status = backup_engine->PurgeOldBackups(1);
				delete backup_engine;
			}
			if ( !status.ok() ) {
				_local_stats->counter("rocksdb.backup.clean.error", 1);
				res.set_status(served::status_5XX::INTERNAL_SERVER_ERROR);
				res << status.ToString();
				_log->error("failed to purge backups: {}", status.ToString());
			} else {
				_local_stats->counter("rocksdb.backup.clean.success", 1);
				res << "Success";
				_log->info("purged old backups at: {}", backup_path);
			}
		});

	mux.handle("/backup_info")
		.get([this](served::response & res, const served::request & req) {
			std::string backup_path = _path + "_backup";

			rocksdb::BackupEngine* backup_engine;
			auto status = rocksdb::BackupEngine::Open(
					rocksdb::Env::Default(),
					rocksdb::BackupableDBOptions(backup_path),
					&backup_engine
			);
			if ( !status.ok() ) {
				res.set_status(served::status_5XX::INTERNAL_SERVER_ERROR);
				res << status.ToString();
				_log->error("failed to read backup: {}", status.ToString());
				return;
			}

			std::vector<rocksdb::BackupInfo> backup_info;
			backup_engine->GetBackupInfo(&backup_info);

			std::stringstream ss;
			ss << "[";
			for ( auto info : backup_info ) {
				ss << "{\"id\":\"" << info.backup_id << "\""
					<< ", \"timestamp\":" << time_t(info.timestamp)
					<< ", \"size\":" << info.size
					<< "},";
			}
			if ( backup_info.size() > 0 ) {
				ss.seekp(-1, std::ios_base::end);
			}
			ss << "]";
			res << ss.str();

			delete backup_engine;
		});

	mux.handle("/endpoints")
		.get([&mux](served::response & res, const served::request & req) {
			const served::served_endpoint_list endpoints = mux.get_endpoint_list();
			for (auto& endpoint : endpoints) {
				for (auto& method : std::get<1>(endpoint.second)) {
					res << method << " " << endpoint.first << "\n";
				}
			}
		});
}

void
rocks::open(log::logger log, stats::aggregator_ptr stats)
{
	_local_stats = stats;
	_log = log;

	_log->info("setting up DB at {}", _path);

	// If we are restoring a backup.
	if ( _restore ) {
		std::cout << "Warning: restoring a backup over a populated DB will"
			<< " destroy any unbacked existing data." << std::endl
			<< "Are you sure you wish to proceed? y/n" << std::endl;
		if ( std::cin.peek() != 121 && std::cin.peek() != 89 ) {
			throw std::runtime_error("Aborting backup restore");
		}

		rocksdb::BackupEngineReadOnly* backup_engine;
		auto status = rocksdb::BackupEngineReadOnly::Open(
			rocksdb::Env::Default(),
			rocksdb::BackupableDBOptions(_path + "_backup"),
			&backup_engine
		);
		if ( status.ok() ) {
			status = backup_engine->RestoreDBFromLatestBackup(_path, _path);
			delete backup_engine;
		}
		if ( !status.ok() ) {
			_log->error("failed to restore backup: {}", status.ToString());
			throw std::runtime_error("failed to restore backup");
		}
		_log->info("successfully restored backup.");
	}

	// Double check the TTL setting as this can be dangerous.
	if ( _ttl == 0 ) {
		_ttl = -1; // 0 is interpretted by rocks as now.
	}

	// Configure DB.
	rocksdb::ColumnFamilyOptions cf_options;
	if ( _write_mode ) {
		cf_options.OptimizeUniversalStyleCompaction(_memtable);
	} else {
		cf_options.OptimizeLevelStyleCompaction(_memtable);
	}

	rocksdb::BlockBasedTableOptions table_options;
	table_options.block_cache = rocksdb::NewLRUCache(_block_cap, _shard_bits);

	rocksdb::Options db_options(rocksdb::DBOptions(), cf_options);
	db_options.IncreaseParallelism();
	db_options.create_if_missing = true;
	db_options.table_factory.reset(rocksdb::NewBlockBasedTableFactory(table_options));
	db_options.max_open_files = _max_files;

	if ( _debug ) {
		_log->info("DEBUG MODE: Collecting granular rocksdb metrics. This will have a small impact on performance.");
		_rocks_stats = rocksdb::CreateDBStatistics();
		db_options.statistics = _rocks_stats;
	}

	// Open our RocksDB instance.
	auto db_status = rocksdb::DBWithTTL::Open(db_options, _path, &_db, _ttl);
	if ( !db_status.ok() ) {
		_db = nullptr;
		throw std::runtime_error("Database failed to open at " + _path + ": "
			+ db_status.ToString());
	}

	if ( _local_stats ) {
		_local_stats->on_epoch([this](){
			uint64_t num_keys = 0;
			_db->GetAggregatedIntProperty("rocksdb.estimate-num-keys", &num_keys);
			_local_stats->gauge("rocksdb.estimate-num-keys", num_keys);

			if ( _rocks_stats ) {
				for ( auto target_pair : rocksdb::TickersNameMap ) {
					uint64_t s = _rocks_stats->getTickerCount(target_pair.first);
					_local_stats->gauge(target_pair.second, s);
				}
			}

			stats::uvalue_t folder_size = 0;
			get_folder_size(_path, folder_size);

			_local_stats->gauge("rocksdb.db-size", folder_size);
			_local_stats->gauge("rocksdb.db-size-mb", (folder_size/1000000));
		});
	}
}

void
rocks::get_folder_size(std::string root, stats::uvalue_t & file_size)
{
	boost::filesystem::path folder_path(root);

	if ( boost::filesystem::exists(folder_path) ) {
		boost::filesystem::directory_iterator end_itr;

		for ( boost::filesystem::directory_iterator dir_ite(root); dir_ite != end_itr; ++dir_ite ) {
			boost::filesystem::path file_path(dir_ite->path());
			try {
				if ( !boost::filesystem::is_directory(dir_ite->status()) ) {
					file_size = file_size + boost::filesystem::file_size(file_path);
				} else {
					get_folder_size(file_path.string(), file_size);
				}
			} catch(std::exception& e) {
				_log->debug("Failed to evaluate folder size for metrics: {}", e.what());
			}
		}
	}
}

status
rocks::del(std::string const & key)
{
	auto s = _db->Delete(rocksdb::WriteOptions(), key);
	bool isNotFound = s.IsNotFound();
	if ( isNotFound ) {
		_local_stats->counter("rocksdb.delete.not_found", 1);
	} else if ( s.ok() ) {
		_local_stats->counter("rocksdb.delete.success", 1);
	} else {
		_local_stats->counter("rocksdb.delete.error", 1);
	}
	return status(s.ok(), isNotFound);
}

status
rocks::get(std::string const & key, std::string * value)
{
	auto s = _db->Get(rocksdb::ReadOptions(), key, value);
	if ( s.ok() ) {
		_local_stats->counter("rocksdb.get.success", 1);
		return status(true);
	}
	bool isNotFound = s.IsNotFound();
	if ( isNotFound ) {
		_local_stats->counter("rocksdb.get.not_found", 1);
	} else {
		_local_stats->counter("rocksdb.get.error", 1);
	}
	return status(s.ok(), isNotFound);
}

status
rocks::put(std::string const & key, std::string const & value)
{
	auto s = _db->Put(rocksdb::WriteOptions(), key, value);
	if ( s.ok() ) {
		_local_stats->counter("rocksdb.put.success", 1);
		return status(true);
	}
	_local_stats->counter("rocksdb.put.error", 1);
	return status(false, false, s.ToString());
}

} } // namespace

