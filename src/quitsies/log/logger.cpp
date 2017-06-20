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

#include "logger.hpp"

namespace quitsies { namespace log {

logger create(std::string const & name, std::string const & level) {
	spdlog::set_async_mode(8192);

	auto log = spdlog::stdout_logger_mt(name);

	if ( level == "trace" ) {
		log->set_level(spdlog::level::trace);
	} else if ( level == "debug" ) {
		log->set_level(spdlog::level::debug);
	} else if ( level == "info" ) {
		log->set_level(spdlog::level::info);
	} else if ( level == "warn" ) {
		log->set_level(spdlog::level::warn);
	} else if ( level == "err" ) {
		log->set_level(spdlog::level::err);
	} else if ( level == "critical" ) {
		log->set_level(spdlog::level::critical);
	} else if ( level == "off" ) {
		log->set_level(spdlog::level::off);
	} else {
		throw std::runtime_error("Unrecognised log level: " + level);
	}

	return log;
}

} } // namespace
