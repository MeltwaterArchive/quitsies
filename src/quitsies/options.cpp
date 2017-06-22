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

#include <quitsies/options.hpp>

#include <iostream>

namespace quitsies {

bool
parse_arg_options(int argc, char* argv[], option_list &options) {
	auto help_func = [&options, &argv]() {
		std::cout << "Usage: " << argv[0] << " [OPTION]..."
			<< std::endl << std::endl;
		std::cout << "General options: " << std::endl << std::endl
			<< "-h, --help, Show this help page." << std::endl;
		for ( auto section : options ) {
			std::cout << std::endl << std::get<0>(section) << " options:"
				<< std::endl << std::endl;
			for ( auto option : std::get<1>(section) ) {
				if ( option->short_name() != '?' ) {
					std::cout << "-" << option->short_name() << ", ";
				}
				std::cout << "--" << option->name()
					<< ", " << option->description();
				if ( option->default_value().length() > 0 ) {
					std::cout << " (default: " << option->default_value() << ")";
				}
				std::cout << std::endl;
			}
		}
	};

	try {
		OptionHandler::Handler h = OptionHandler::Handler(argc, argv);

		for ( auto section : options ) {
			for ( auto option : std::get<1>(section) ) {
				option->register_option(h);
			}
		}
		h.add_option('h', "help", OptionHandler::NONE, false);

		if ( h.get_option("help") ) {
			help_func();
			return false;
		}

		for ( auto section : options ) {
			for ( auto option : std::get<1>(section) ) {
				option->read_option(h);
			}
		}
	} catch (std::exception & e) {
		std::cout << "Parse input flags error: " << e.what() << std::endl;
		std::cout << "Use -h, --help for a list of options" << std::endl;
		return false;
	}

	return true;
}

} // namespace quitsies
