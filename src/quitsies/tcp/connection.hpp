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

#ifndef QUITSIES_CONNECTION_HPP
#define QUITSIES_CONNECTION_HPP

#include <boost/asio.hpp>

#include <rocksdb/db.h>

#include <quitsies/tcp/request.hpp>
#include <quitsies/log/logger.hpp>
#include <quitsies/stats/aggregator.hpp>

#include <array>
#include <memory>

namespace quitsies { namespace tcp {

class connection_manager;

/*
 * Manages the lifecycle of a single TCP connection.
 *
 * A connection is created by the server each time a new client connects.
 */
class connection
	: public std::enable_shared_from_this<connection>
{
	boost::asio::io_service &    _io_service;
	boost::asio::ip::tcp::socket _socket;
	connection_manager &         _connection_manager;
	log::logger                  _log;
	stats::aggregator_ptr        _stats;
	request                      _request;
	std::array<char, 8192>       _buffer;
	int                          _read_timeout;
	int                          _write_timeout;
	boost::asio::deadline_timer  _read_timer;
	boost::asio::deadline_timer  _write_timer;

public:
	connection(connection&) = delete;

	connection& operator=(const connection&) = delete;

	explicit connection( boost::asio::io_service &    io_service
	                   , boost::asio::ip::tcp::socket socket
	                   , connection_manager &         manager
	                   , db::store_ptr                db
	                   , log::logger                  log
	                   , stats::aggregator_ptr        stats
	                   , size_t                       max_request_size_bytes
	                   , int                          read_timeout
	                   , int                          write_timeout );

	/*
	 * Prompts the connection to start reading from its TCP socket.
	 */
	void start();

	/*
	 * Prompts the connection to close the TCP connection early.
	 */
	void stop();

private:
	/*
	 * An asynchronous call that triggers a TCP read from the socket.
	 */
	void do_read();

	/*
	 * An asynchronous call that triggers a TCP write to the socket.
	 */
	void do_write();
};

typedef std::shared_ptr<connection> connection_ptr;

} } // tcp, quitsies

#endif // QUITSIES_CONNECTION_HPP
