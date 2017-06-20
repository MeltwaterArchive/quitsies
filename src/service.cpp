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

#include <thread>

#include <served/served.hpp>

#include <quitsies/options.hpp>
#include <quitsies/tcp/server.hpp>
#include <quitsies/db/rocks.hpp>
#include <quitsies/stats/statsd_aggregator.hpp>
#include <quitsies/stats/null_aggregator.hpp>
#include <quitsies/log/logger.hpp>

using namespace quitsies;

int main(int argc, char* argv[]) {
	// Input flag variables.
	std::string http_address    = "localhost", http_port      = "3058",
	            http_prefix     = "/quitsies", tcp_address    = "localhost",
	            tcp_port        = "11211",     statsd_address = "",
	            statsd_port     = "8125",      statsd_prefix  = "quitsies",
	            log_level       = "info";
	long long   n_http_threads  = 1,           n_tcp_threads  = 10;

	// Create our DB.
	db::store_ptr db(new db::rocks());

	{
		// Define our cmd flag options.
		option_list options = {{
			std::make_tuple("IO", option_array({
				option_ptr(new str_option('?', "http_address", "Address to bind to for HTTP.", &http_address)),
				option_ptr(new str_option('?', "http_port", "Port to bind to for HTTP.", &http_port)),
				option_ptr(new str_option('?', "http_prefix", "Path prefix for HTTP paths.", &http_prefix)),
				option_ptr(new int_option('?', "http_threads", "Number of HTTP threads.", &n_http_threads)),
				option_ptr(new str_option('?', "tcp_address", "Address to bind to for TCP.", &tcp_address)),
				option_ptr(new str_option('?', "tcp_port", "Port to bind to for TCP.", &tcp_port)),
				option_ptr(new int_option('?', "tcp_threads", "Number of TCP threads.", &n_tcp_threads)),
				option_ptr(new str_option('?', "statsd_address", "Address of the statsd server for sending metrics.", &statsd_address)),
				option_ptr(new str_option('?', "statsd_port", "Port of the statsd server for sending metrics.", &statsd_port)),
				option_ptr(new str_option('?', "statsd_prefix", "Prefix of statsd metrics.", &statsd_prefix)),
				option_ptr(new str_option('?', "log_level", "Level of logging (trace, debug, info, warn, err, critical, off).", &log_level))
			}))
		}};

		db->register_options(options);

		// And parse our cmd flag options.
		if ( !parse_arg_options(argc, argv, options) ) {
			return 1;
		}
	}

	// Create logger
	auto logger = log::create("quitsies", log_level);

	// Print general options.
	logger->info("REST API listening at http://{}:{}{} with {} threads.", http_address, http_port, http_prefix, n_http_threads);
	logger->info("Memcached API listening at tcp://{}:{} with {} threads.", tcp_address, tcp_port, n_tcp_threads);

	// Create our metrics aggregator.
	stats::aggregator_ptr stats(new stats::null_aggregator());
	if ( statsd_address.length() > 0 ) {
		logger->info("Attempting to connect to statsd server at {}:{}", statsd_address, statsd_port);
		stats.reset(new stats::statsd_aggregator(statsd_address, statsd_port, statsd_prefix, 1));
	}

	// Open db.
	try {
		db->open(logger, stats);
	} catch ( std::exception & e ) {
		logger->error("Failed to open database: {}, terminating application.", e.what());
		return 1;
	}

	// Create our REST API mux.
	served::multiplexer mux(http_prefix);
	db->register_endpoints(mux);

	// Create memcached API and start listening.
	tcp::server memcached_server(tcp_address, tcp_port, db, logger, stats);
	std::thread mem_thread([&memcached_server, &n_tcp_threads]() {
		memcached_server.run(n_tcp_threads);
	});

	// Start listening for HTTP requests.
	served::net::server server(http_address, http_port, mux);

	// And wait for termination.
	server.run(n_http_threads);

	logger->info("Process exiting, closing DB");
	mem_thread.join();

	return 0;
}
