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

#include <quitsies/tcp/connection.hpp>
#include <quitsies/tcp/connection_manager.hpp>

#include <utility>
#include <vector>

using namespace quitsies::tcp;

connection::connection( boost::asio::io_service &    io_service
                      , boost::asio::ip::tcp::socket socket
                      , connection_manager &         manager
                      , db::store_ptr                db
                      , log::logger                  log
                      , stats::aggregator_ptr        stats
                      , size_t                       max_req_size_bytes
                      , int                          read_timeout
                      , int                          write_timeout
                      )
	: _io_service(io_service)
	, _socket(std::move(socket))
	, _connection_manager(manager)
	, _log(log)
	, _stats(stats)
	, _request(db, log, stats, max_req_size_bytes)
	, _read_timeout(read_timeout)
	, _write_timeout(write_timeout)
	, _read_timer(_io_service, boost::posix_time::milliseconds(read_timeout))
	, _write_timer(_io_service, boost::posix_time::milliseconds(write_timeout))
{}

void
connection::start()
{
	do_read();

	if ( _read_timeout > 0 ) {
		auto self(shared_from_this());

		_read_timer.async_wait([this, self](const boost::system::error_code& error) {
			if ( error.value() != boost::system::errc::operation_canceled ) {
				_connection_manager.stop(shared_from_this());
			}
		});
	}
}

void
connection::stop()
{
	boost::system::error_code ignored_ec;
	_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
		ignored_ec);
	_socket.close();
}

void
connection::do_read()
{
	auto self(shared_from_this());

	_socket.async_read_some(boost::asio::buffer(_buffer.data(), _buffer.size()),
		[this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
			if (!ec) {
				_request.process(_buffer.data(), bytes_transferred);

				switch ( _request.get_status() ) {
				case request::status_type::FINISHED:
					// Parsing is finished, stop reading and send response.
					_read_timer.cancel();

					if ( _write_timeout > 0 ) {
						_write_timer.async_wait([this, self](const boost::system::error_code& error) {
							if ( error.value() != boost::system::errc::operation_canceled ) {
								_connection_manager.stop(shared_from_this());
							}
						});
					}

					if ( _request.get_no_reply() ) {
						_write_timer.cancel();
						_request.reset();
						start();
					} else {
						do_write();
					}
					break;
				case request::status_type::QUITTING:
					_connection_manager.stop(shared_from_this());
					break;
				default:
					// Not finished reading response, continue.
					do_read();
				}
			} else if (ec != boost::asio::error::operation_aborted) {
				_connection_manager.stop(shared_from_this());
			}
		}
	);
}

void
connection::do_write()
{
	auto self(shared_from_this());

	boost::asio::async_write(_socket, boost::asio::buffer(_request.get_response()),
		[this, self](boost::system::error_code ec, std::size_t) {
			if ( !ec ) {
				_write_timer.cancel();
				_request.reset();
				start();
			} else if ( ec != boost::asio::error::operation_aborted ) {
				_connection_manager.stop(shared_from_this());
			}
		}
	);
}
