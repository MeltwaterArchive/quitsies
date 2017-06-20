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

#ifndef QUITSIES_DB_STORE
#define QUITSIES_DB_STORE

#include <string>
#include <memory>

#include <served/multiplexer.hpp>

#include <quitsies/options.hpp>
#include <quitsies/stats/aggregator.hpp>
#include <quitsies/log/logger.hpp>

namespace quitsies { namespace db {

class status {
	bool        _ok;
	bool        _is_not_found;
	std::string _msg;

public:
	status(bool ok, bool is_not_found = false, std::string msg = "")
	      : _ok(ok)
	      , _is_not_found(is_not_found)
	      , _msg(msg) {}

	bool        ok()           { return _ok; }
	bool        is_not_found() { return _is_not_found; }
	std::string to_string()    { return _msg; }
};

class store {
public:
	virtual void register_options(option_list & options) = 0;
	virtual void register_endpoints(served::multiplexer & mux) = 0;

	virtual void open(log::logger, stats::aggregator_ptr) = 0;

	// Get the value of a key, returns true if the key was found.
	virtual status get(std::string const & key, std::string * value) = 0;

	// Delete a key/value pair, returns true if the key was found and removed.
	virtual status del(std::string const & key) = 0;

	// Store a key value pair.
	virtual status put(std::string const & key, std::string const & value) = 0;
};

typedef std::shared_ptr<store> store_ptr;

} } // namespace

#endif // QUITSIES_DB_STORE
