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

#include <cstdio>
#include <cinttypes>

#include <boost/lexical_cast.hpp>

#include "statsd.hpp"

namespace detail {
	const std::string type_counter   = "%s:%d|c";
	const std::string type_timer     = "%s:%" PRIu64 "|ms";
	const std::string type_gauge     = "%s:%" PRIu64 "|g";
	const std::string type_set       = "%s:%d|s";
}

namespace quitsies { namespace stats {

statsd::statsd(std::string const& host, std::string const& port, std::string const& prefix)
	: _prefix( prefix + "." )
	, _io_service()
	, _socket(_io_service)
	, _resolver(_io_service)
	, _query( host, port )
	, _endpoint( *_resolver.resolve( _query ) )
{
	_socket.open( boost::asio::ip::udp::v4() );
}

statsd::~statsd()
{
}

void statsd::counter(std::string const& name, value_t const value)
{
	send_stat( detail::type_counter, name.c_str(), value );
}

void statsd::timer(std::string const& name, uvalue_t const value)
{
	send_stat( detail::type_timer, name.c_str(), value );
}

void statsd::gauge(std::string const& name, uvalue_t const value)
{
	send_stat( detail::type_gauge, name.c_str(), value );
}

void statsd::set(std::string const& name, value_t const  value)
{
	send_stat( detail::type_set, name.c_str(), value );
}

} } // namespaces
