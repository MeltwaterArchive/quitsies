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

#ifndef ARG_OPTIONS_H
#define ARG_OPTIONS_H

#include <string>
#include <vector>
#include <tuple>
#include <memory>
#include <sstream>

#include <OptionHandler/option_handler.h>

namespace quitsies {

class option {
public:
	virtual std::string default_value() = 0;
	virtual std::string name() = 0;
	virtual char        short_name() = 0;
	virtual std::string description() = 0;

	virtual void register_option(OptionHandler::Handler & h) = 0;
	virtual void read_option(OptionHandler::Handler & h) = 0;
};

class bool_option : public option {
	char                _short_name;
	const std::string   _name;
	const std::string   _description;
	bool              * _value;

public:
	bool_option( char                short_name
	           , const std::string & name
	           , const std::string & description
	           , bool              * value
	           )
	      : _short_name(short_name)
	      , _name(name)
	      , _description(description)
	      , _value(value)
	{};

	std::string default_value() { return ""; }
	std::string name() { return _name; }
	char short_name() { return _short_name; }
	std::string description() { return _description; }

	void register_option(OptionHandler::Handler & h) {
		h.add_option(_short_name, _name, OptionHandler::NONE, false);
	}

	void read_option(OptionHandler::Handler & h) {
		if ( h.get_option(_name) ) {
			(*_value) = true;
		}
	}
};

class int_option : public option {
	char                _short_name;
	const std::string   _name;
	const std::string   _description;
	long long         * _value;

public:
	int_option( char                short_name
	          , const std::string & name
	          , const std::string & description
	          , long long         * value
	          )
	      : _short_name(short_name)
	      , _name(name)
	      , _description(description)
	      , _value(value)
	{};

	std::string default_value() {
		std::stringstream ss;
		ss << *_value;
		return ss.str();
	}
	std::string name() { return _name; }
	char short_name() { return _short_name; }
	std::string description() { return _description; }

	void register_option(OptionHandler::Handler & h) {
		h.add_option(_short_name, _name, OptionHandler::REQUIRED, false);
	}

	void read_option(OptionHandler::Handler & h) {
		if ( h.get_option(_name) ) {
			(*_value) = std::stoll(h.get_argument(_name));
		}
	}
};

class str_option : public option {
	char                _short_name;
	const std::string   _name;
	const std::string   _description;
	std::string       * _value;

public:
	str_option( char                short_name
	          , const std::string & name
	          , const std::string & description
	          , std::string       * value
	          )
	      : _short_name(short_name)
	      , _name(name)
	      , _description(description)
	      , _value(value)
	{};

	std::string default_value() { return *_value; }
	std::string name() { return _name; }
	char short_name() { return _short_name; }
	std::string description() { return _description; }

	void register_option(OptionHandler::Handler & h) {
		h.add_option(_short_name, _name, OptionHandler::REQUIRED, false);
	}

	void read_option(OptionHandler::Handler & h) {
		if ( h.get_option(_name) ) {
			(*_value) = h.get_argument(_name);
		}
	}
};

typedef std::shared_ptr<option>               option_ptr;
typedef std::vector<option_ptr>               option_array;
typedef std::tuple<std::string, option_array> option_section;
typedef std::vector<option_section>           option_list;

bool
parse_arg_options(int argc, char* argv[], option_list &options);

} // namespace quitsies

#endif // ARG_OPTIONS_H
