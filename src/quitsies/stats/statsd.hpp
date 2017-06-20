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

#ifndef DATASIFT_STATS_STATSD_HPP_
#define DATASIFT_STATS_STATSD_HPP_

#include <chrono>

#include <boost/asio.hpp>

#include "aggregator.hpp"

namespace quitsies { namespace stats {

class statsd
{
public:
	// Constructor, you must specify a named host, and a port to connect to.
	statsd(std::string const& host, std::string const& port, std::string const& prefix);

	// Destructor.
	~statsd();

	// Record a counter with the given name, the value is the increment of
	// the counter. If you specify a value for sample, it will allow you to
	// specify that the counter was sampled. For example, to record a
	// counter with the name "my_counter" with a value of 1, sampled 1/10
	// of the time, you would type:
	// ...
	// Stats::getInstance().counter("my_counter", 1, 0.1f);
	// ...
	void counter(std::string const& name, value_t const value);

	// Record a timer statistic with the given name, and the value, which
	// should be specified in milliseconds.
	void timer(std::string const& name, uvalue_t const value);

	// Record a gauge statistic with the given name, and the given value. It
	// is possible to record relative changes to a gauge by specifying
	// relative = true. In this case the value given will be used to either
	// increment or decrement the value of the gauge, based on whether the
	// value is positive or negative.
	void gauge(std::string const& name, uvalue_t const value);

	// Record a set statistic with the given name and the given value.
	void set(std::string const& name, value_t value);

private:
	static const int max_buffer_size = 256;

	std::string                           _prefix;
	boost::asio::io_service               _io_service;
	boost::asio::ip::udp::socket          _socket;
	boost::asio::ip::udp::resolver        _resolver;
	boost::asio::ip::udp::resolver::query _query;
	boost::asio::ip::udp::endpoint        _endpoint;

	// TODO: perfect forwarding, why did that break this code?
	template<typename... Args>
	inline bool send_stat(std::string const& mask, Args... args)
	{
		char buffer[max_buffer_size];
		std::string full_path = _prefix + mask;

		int bytes = snprintf( buffer, max_buffer_size, full_path.c_str(), args... );

		if (bytes <= 0 || bytes >= max_buffer_size) { throw std::runtime_error( "stat message larger than max buffer size" ); }

		return _socket.send_to(boost::asio::buffer(buffer, bytes), _endpoint) == static_cast<size_t>( bytes );
	}
};

} } // namespace

#endif // DATASIFT_STATS_STATSD_HPP_
