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

#define CATCH_CONFIG_MAIN
#include <test/catch.hpp>

#include <quitsies/tcp/request.hpp>
#include <quitsies/stats/aggregator.hpp>
#include <quitsies/stats/null_aggregator.hpp>

using namespace quitsies::tcp;
using namespace quitsies::stats;

auto mock_stats = aggregator_ptr(new null_aggregator());
auto mock_logger = quitsies::log::create("quitsies_test", "off");

TEST_CASE("request parser can parse memcached requests", "[request_parser]")
{
	SECTION("command is parsed correctly")
	{
		SECTION("check full commands")
		{
			std::vector<std::string> test_cases = {{
				"get key1\r\n",
				"add key2 20 0 11\r\nhello world\r\n",
				"add key3 20 0 11 [noreply]\r\nhello world\r\n",
				"gets key4 key5 key6\r\n",
				"set key4 0 0 5\r\nhello\r\n"
			}};

			for ( auto test_case : test_cases ) {
				request req(NULL, mock_logger, mock_stats, 0);
				req.process(test_case.c_str(), test_case.length());

				INFO("Test case: " << test_case);
				CHECK(req.get_status() == request::status_type::FINISHED);
			}
		}

		SECTION("check split commands")
		{
			std::vector<std::vector<std::string>> test_cases = {{
				{{
					"gets key",
					"4 key5 key6 ",
					"key7 key8\r\n"
				}},
				{{
					"add ke",
					"y2 20 0 11\r\n",
					"hello world\r\n"
				}},
				{{
					"add key3 2",
					"0 0 11 [nor",
					"eply]\r\nhello world\r\n"
				}},
				{{
					"s",
					"et ",
					"key4",
					" 0 0 ",
					"5\r\nhello\r\n"
				}},
				{{
					"get ",
					"key1",
					"\r\n"
				}}
			}};

			for ( auto test_case : test_cases ) {
				request req(NULL, mock_logger, mock_stats, 0);
				for ( auto part : test_case ) {
					INFO("Part: " << part);
					CHECK(req.get_status() != request::status_type::FINISHED);
					req.process(part.c_str(), part.length());
				}
				CHECK(req.get_status() == request::status_type::FINISHED);
			}
		}

		SECTION("check parse set command")
		{
			std::string cmd = "set key4 1 1 5\r\nhello\r\n";

			request req(NULL, mock_logger, mock_stats, 0);
			req.process(cmd.c_str(), cmd.length());

			INFO("Command: " << cmd);

			CHECK(req.get_status() == request::status_type::FINISHED);
			CHECK(req.get_command() == request::command_type::SET);
			REQUIRE(req.get_keys().size() == 1);
			CHECK(req.get_keys()[0] == "key4");
			CHECK(req.get_flags() == 1);
			CHECK(req.get_exp_time() == 1);
			CHECK(req.get_no_reply() == false);
			CHECK(req.copy_buffer() == "hello");
		}

		SECTION("check parse set noreply command")
		{
			std::string cmd = "set key4 1 1 5 noreply\r\nhello\r\n";

			request req(NULL, mock_logger, mock_stats, 0);
			req.process(cmd.c_str(), cmd.length());

			INFO("Command: " << cmd);

			CHECK(req.get_status() == request::status_type::FINISHED);
			CHECK(req.get_command() == request::command_type::SET);
			REQUIRE(req.get_keys().size() == 1);
			CHECK(req.get_keys()[0] == "key4");
			CHECK(req.get_flags() == 1);
			CHECK(req.get_exp_time() == 1);
			CHECK(req.get_no_reply() == true);
			CHECK(req.copy_buffer() == "hello");
		}

		SECTION("check parse get single key command")
		{
			std::string cmd = "get key5\r\n";

			request req(NULL, mock_logger, mock_stats, 0);
			req.process(cmd.c_str(), cmd.length());

			INFO("Command: " << cmd);

			CHECK(req.get_status() == request::status_type::FINISHED);
			CHECK(req.get_command() == request::command_type::GET);
			REQUIRE(req.get_keys().size() == 1);
			CHECK(req.get_keys()[0] == "key5");
			CHECK(req.get_flags() == 0);
			CHECK(req.get_exp_time() == 0);
			CHECK(req.get_no_reply() == false);
			CHECK(req.copy_buffer() == "");
		}

		SECTION("check parse get multi key command")
		{
			std::string cmd = "gets key6 key7 key8\r\n";

			request req(NULL, mock_logger, mock_stats, 0);
			req.process(cmd.c_str(), cmd.length());

			INFO("Command: " << cmd);

			CHECK(req.get_status() == request::status_type::FINISHED);
			CHECK(req.get_command() == request::command_type::GETS);
			REQUIRE(req.get_keys().size() == 3);
			CHECK(req.get_keys()[0] == "key6");
			CHECK(req.get_keys()[1] == "key7");
			CHECK(req.get_keys()[2] == "key8");
			CHECK(req.get_flags() == 0);
			CHECK(req.get_exp_time() == 0);
			CHECK(req.get_no_reply() == false);
			CHECK(req.copy_buffer() == "");
		}

		SECTION("check delete key command")
		{
			std::string cmd = "delete key5\r\n";

			request req(NULL, mock_logger, mock_stats, 0);
			req.process(cmd.c_str(), cmd.length());

			INFO("Command: " << cmd);

			CHECK(req.get_status() == request::status_type::FINISHED);
			CHECK(req.get_command() == request::command_type::DELETE);
			REQUIRE(req.get_keys().size() == 1);
			CHECK(req.get_keys()[0] == "key5");
			CHECK(req.get_no_reply() == false);
			CHECK(req.copy_buffer() == "");
		}

		SECTION("check delete key noreply command")
		{
			std::string cmd = "delete key5 noreply\r\n";

			request req(NULL, mock_logger, mock_stats, 0);
			req.process(cmd.c_str(), cmd.length());

			INFO("Command: " << cmd);

			CHECK(req.get_status() == request::status_type::FINISHED);
			CHECK(req.get_command() == request::command_type::DELETE);
			REQUIRE(req.get_keys().size() == 1);
			CHECK(req.get_keys()[0] == "key5");
			CHECK(req.get_no_reply() == true);
			CHECK(req.copy_buffer() == "");
		}

		SECTION("check ping command")
		{
			std::string cmd = "ping\r\n";

			request req(NULL, mock_logger, mock_stats, 0);
			req.process(cmd.c_str(), cmd.length());

			INFO("Command: " << cmd);

			CHECK(req.get_status() == request::status_type::FINISHED);
			CHECK(req.get_command() == request::command_type::PING);

			auto res = req.get_response();
			CHECK(res == "PONG\r\n");
		}
	}
}
