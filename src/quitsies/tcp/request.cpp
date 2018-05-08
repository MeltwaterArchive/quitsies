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

#include <quitsies/tcp/request.hpp>

using namespace quitsies::tcp;

bool
is_white_space(char c) {
	return c == ' '
		|| c == '\t'
		|| c == '\f'
		|| c == '\r'
		|| c == '\n'
		|| c == '\v';
}

request::command_type
command_from_str(std::string const& cmd) {
	if ( cmd == "set" ) {
		return request::command_type::SET;
	}
	if ( cmd == "add" ) {
		return request::command_type::ADD;
	}
	if ( cmd == "get" ) {
		return request::command_type::GET;
	}
	if ( cmd == "gets" ) {
		return request::command_type::GETS;
	}
	if ( cmd == "delete" ) {
		return request::command_type::DELETE;
	}
	if ( cmd == "quit" ) {
		return request::command_type::QUIT;
	}
	if ( cmd == "ping" ) {
		return request::command_type::PING;
	}
	return request::command_type::NONE;
}

request::status_type
status_from_command(request::command_type const& cmd) {
	switch (cmd) {
	case request::command_type::SET:
	case request::command_type::ADD:
		return request::status_type::STORAGE_KEY;
	case request::command_type::GET:
	case request::command_type::GETS:
		return request::status_type::RETRIEVAL_KEY;
	case request::command_type::DELETE:
		return request::status_type::DELETE_KEY;
	case request::command_type::QUIT:
		return request::status_type::QUITTING;
	case request::command_type::PING:
		return request::status_type::FINISHED;
	case request::command_type::NONE:
	default:
		throw std::runtime_error("unsupported command");
	}
	return request::status_type::FINISHED;
}

bool
is_returned(std::stringstream & buffer) {
	buffer.seekg(0, std::ios::end);
	return buffer.get() == '\r';
}

void
request::prepare_response() {
	if ( _command == command_type::PING ) {
		_response = "PONG\r\n";
		_status = status_type::FINISHED;
		return;
	}
	if ( !_db ) {
		_response = "ERROR The server isn't configured with a database\r\n";
		_status = status_type::FINISHED;
		return;
	}
	if ( _keys.size() == 0 ) {
		_response = "ERROR No key was found in request\r\n";
		_status = status_type::FINISHED;
		return;
	}
	try {
		std::stringstream ss;
		switch (_command) {
		case command_type::DELETE:
			{
				std::string value;
				auto status = _db->get(_keys[0], &value);
				if ( !status.ok() ) {
					ss << "NOT_FOUND\r\n";
					break;
				}

				status = _db->del(_keys[0]);
				if ( status.ok() ) {
					ss << "DELETED\r\n";
				} else {
					ss << "ERROR ";
					ss << status.to_string();
					ss << "\r\n";
				}
			}
			break;
		case command_type::ADD:
			{
				_db->lock();
				std::string value;
				auto status = _db->get(_keys[0], &value);
				if ( !status.ok() ) {
					if ( status.is_not_found() ) {
						status = _db->put(_keys[0], _buffer.str());
						if ( status.ok() ) {
							ss << "STORED\r\n";
						} else {
							ss << "ERROR ";
							ss << status.to_string();
							ss << "\r\n";
						}
					} else {
						ss << "ERROR ";
						ss << status.to_string();
						ss << "\r\n";
					}
				} else {
					// Add is only applied if the key does not exist
					ss << "NOT_STORED\r\n";
				}
				_db->unlock();
			}
			break;
		case command_type::SET:
			{
				auto status = _db->put(_keys[0], _buffer.str());
				if ( status.ok() ) {
					ss << "STORED\r\n";
				} else {
					ss << "ERROR ";
					ss << status.to_string();
					ss << "\r\n";
				}
			}
			break;
		case command_type::GET:
		case command_type::GETS:
			for ( auto key : _keys ) {
				std::string value;
				auto status = _db->get(key, &value);
				if ( status.ok() ) {
					ss << "VALUE " << key << " 0 " << value.length() << "\r\n";
					ss << value << "\r\n";
				}
			}
			ss << "END\r\n";
			break;
		case request::command_type::NONE:
		default:
			throw std::runtime_error("unsupported command");
		}
		_response = ss.str();
	} catch (std::exception & e) {
		std::stringstream ss;
		ss << "ERROR " << e.what() << "\r\n";
		_response = ss.str();
	}
	_status = status_type::FINISHED;
}

void
request::process(const char * data, size_t length) {
	try {
		if ( _max_bytes > 0 && (length + _buffer.tellp() >= _max_bytes) ) {
			throw std::runtime_error("request size exceeded max bytes");
		}
		for ( size_t i = 0; i < length; i++ ) {
			if ( _status == status_type::DATA ) {
				if ( _remaining > 0 ) {
					_remaining--;
					_buffer << data[i];
					continue;
				} else {
					prepare_response();
					return;
				}
			}
			if ( is_white_space(data[i]) ) {
				if ( _status == status_type::COMMAND )
				{
					_command = command_from_str(_buffer.str());
					_buffer.str(std::string());
					_status = status_from_command(_command);
					if ( _status == status_type::FINISHED ) {
						prepare_response();
						return;
					}
				}
				else if ( _status == status_type::DELETE_KEY )
				{
					std::string key = _buffer.str();
					if ( key.length() == 0 ) {
						throw std::runtime_error("invalid key");
					}
					_keys.push_back(key);
					_buffer.str(std::string());
					_status = status_type::NOREP;
				}
				else if ( _status == status_type::STORAGE_KEY )
				{
					std::string key = _buffer.str();
					if ( key.length() == 0 ) {
						throw std::runtime_error("invalid key");
					}
					_keys.push_back(key);
					_buffer.str(std::string());
					_status = status_type::FLAGS;
				}
				else if ( _status == status_type::FLAGS )
				{
					_flags = std::stoi(_buffer.str());
					_buffer.str(std::string());
					_status = status_type::EXPTIME;
				}
				else if ( _status == status_type::EXPTIME )
				{
					_exp_time = std::stoi(_buffer.str());
					_buffer.str(std::string());
					_status = status_type::NBYTES;
				}
				else if ( _status == status_type::NBYTES )
				{
					_remaining = std::stoi(_buffer.str());
					_buffer.str(std::string());
					_status = status_type::NOREP;
				}
				else if ( _status == status_type::NOREP )
				{
					if ( _buffer.str() == "noreply" ) {
						_buffer.str(std::string());
						_no_reply = true;
						_status = status_type::NOREP;
						continue;
					}
					if ( _remaining > 0 ) {
						_status = status_type::DATA;
					} else {
						prepare_response();
						return;
					}
				}
				else if ( _status == status_type::RETRIEVAL_KEY
				       || _status == status_type::RETRIEVAL_KEYS )
				{
					if ( _buffer.tellp() == 0 ) {
						prepare_response();
						return;
					} else {
						std::string key = _buffer.str();
						if ( key.length() == 0 ) {
							throw std::runtime_error("invalid key");
						}
						_keys.push_back(key);
						_buffer.str(std::string());
					}
				}
				continue;
			}
			_buffer << data[i];
		}
	} catch (std::exception & e) {
		_status = status_type::FINISHED;

		std::stringstream ss;
		ss << "CLIENT_ERROR " << e.what() << "\r\n";
		_response = ss.str();
	}
}

std::string
request::get_response() {
	if ( _status != status_type::FINISHED ) {
		throw std::runtime_error("request not fully parsed");
	}
	return _response;
}
