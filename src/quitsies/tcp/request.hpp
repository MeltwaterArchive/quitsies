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

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <sstream>
#include <vector>

#include <quitsies/db/store.hpp>
#include <quitsies/log/logger.hpp>
#include <quitsies/stats/aggregator.hpp>

namespace quitsies { namespace tcp {

class request {
public:
	enum status_type {
		COMMAND = 0,
		DELETE_KEY,
		STORAGE_KEY,
		FLAGS,
		EXPTIME,
		NBYTES,
		NOREP,
		DATA,
		RETRIEVAL_KEY,
		RETRIEVAL_KEYS,
		FINISHED,
		QUITTING
	};

	enum command_type {
		NONE = 0,
		SET,
		ADD,
		GET,
		GETS,
		DELETE,
		QUIT,
		PING
	};

private:
	db::store_ptr         _db;
	log::logger           _log;
	stats::aggregator_ptr _stats;
	size_t                _max_bytes;
	status_type           _status;
	std::stringstream     _buffer;
	std::string           _response;

	command_type             _command;
	std::vector<std::string> _keys;
	int                      _flags;
	int                      _exp_time;
	size_t                   _remaining;
	bool                     _no_reply;

public:
	request(const request&) = delete;

	request& operator=(const request&) = delete;

	explicit request( db::store_ptr         db
	                , log::logger           log
	                , stats::aggregator_ptr stats
	                , size_t                max_bytes )
		: _db(db)
		, _log(log)
		, _stats(stats)
		, _max_bytes(max_bytes)
		, _status(status_type::COMMAND)
		, _buffer()
		, _response()
		, _flags(0)
		, _exp_time(0)
		, _remaining(0)
		, _no_reply(false)
	{}

	status_type              get_status()   { return _status; }
	command_type             get_command()  { return _command; }
	std::vector<std::string> get_keys()     { return _keys; }
	int                      get_flags()    { return _flags; }
	int                      get_exp_time() { return _exp_time; }
	bool                     get_no_reply() { return _no_reply; }
	std::string              copy_buffer()  { return _buffer.str(); }

	void process(const char * data, size_t length);
	void reset() {
		_status = COMMAND;
		_command = command_type::NONE;
		_keys.clear();
		_flags = 0;
		_exp_time = 0;
		_remaining = 0;
		_no_reply = false;
		_buffer.str(std::string());
		_response.clear();
	}

	std::string get_response();

private:
	void prepare_response();
};

} } // tcp, quitsies

#endif // REQUEST_HPP
