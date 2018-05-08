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

#ifndef ROCKSDB_STORE
#define ROCKSDB_STORE

#include <rocksdb/db.h>
#include <rocksdb/utilities/db_ttl.h>
#include <rocksdb/statistics.h>

#include <quitsies/log/logger.hpp>
#include <quitsies/stats/aggregator.hpp>
#include <quitsies/stats/null_aggregator.hpp>

#include <quitsies/db/store.hpp>

#include <mutex>

namespace quitsies { namespace db {

class rocks : public store {
	std::string _path;

	long long _ttl;
	long long _memtable;
	long long _shard_bits;
	long long _block_cap;
	long long _max_files;

	bool _debug;
	bool _write_mode;
	bool _restore;

	rocksdb::DBWithTTL * _db;

	stats::aggregator_ptr                _local_stats;
	std::shared_ptr<rocksdb::Statistics> _rocks_stats;

	log::logger _log;

	std::mutex _db_mutex;

public:
	rocks()
	     : _path("/tmp/quitsies")
	     , _ttl(0)
	     , _memtable(128 << 20) // 128MB
	     , _shard_bits(4)
	     , _block_cap(8 << 20) // 8MB
	     , _max_files(-1)
	     , _debug(false)
	     , _write_mode(false)
	     , _restore(false)
	     , _local_stats(new stats::null_aggregator())
	     , _log()
	{}

	~rocks() {
		if ( _rocks_stats ) {
			_rocks_stats.reset();
		}
		if ( _db != nullptr ) {
			delete _db;
			_db = nullptr;
		}
	}

	void register_options(option_list & options);
	void register_endpoints(served::multiplexer & mux);

	// open the database.
	void open(log::logger log, stats::aggregator_ptr stats);

	// Get the value of a key.
	status get(std::string const & key, std::string * value);

	// Delete a key/value pair.
	status del(std::string const & key);

	// Store a key value pair.
	status put(std::string const & key, std::string const & value);

	void lock() {
		_db_mutex.lock();
	}

	void unlock() {
		_db_mutex.unlock();
	}

private:
	void get_folder_size(std::string path, stats::uvalue_t & size);
};

} } // namespace

#endif // ROCKSDB_STORE
