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

#include "statsd_aggregator.hpp"

#include <chrono>

namespace quitsies { namespace stats {

void
statsd_aggregator::counter(std::string const& name, value_t const value) {
	std::lock_guard<std::mutex> guard(_counters_mutex);

	auto search = _counters.find(name);
	if ( search != _counters.end() ) {
		_counters[name] = search->second + value;
	} else {
		_counters[name] = value;
	}
}

void
statsd_aggregator::timer(std::string const& name, uvalue_t const value) {
	std::lock_guard<std::mutex> guard(_timers_mutex);
	_timers[name] = value;
}

void
statsd_aggregator::gauge(std::string const& name, uvalue_t const value) {
	std::lock_guard<std::mutex> guard(_gauges_mutex);
	_gauges[name] = value;
}

void
statsd_aggregator::set(std::string const& name, value_t value) {
	std::lock_guard<std::mutex> guard(_sets_mutex);
	_sets[name] = value;
}

void
statsd_aggregator::background_loop() {
	while ( _background_running ) {
		{
			std::lock_guard<std::mutex> guard(_epochs_mutex);
			for ( auto call : _epoch_calls ) {
				call();
			}
		}
		{
			sets s;
			{
				std::lock_guard<std::mutex> guard(_sets_mutex);
				std::swap(s, _sets);
			}
			for ( auto set : s ) {
				_statsd_client->set(set.first, set.second);
			}
		}
		{
			timers t;
			{
				std::lock_guard<std::mutex> guard(_timers_mutex);
				std::swap(t, _timers);
			}
			for ( auto timer : t ) {
				_statsd_client->timer(timer.first, timer.second);
			}
		}
		{
			gauges g;
			{
				std::lock_guard<std::mutex> guard(_gauges_mutex);
				std::swap(g, _gauges);
			}
			for ( auto gauge : g ) {
				_statsd_client->gauge(gauge.first, gauge.second);
			}
		}
		{
			counters c;
			{
				std::lock_guard<std::mutex> guard(_counters_mutex);
				std::swap(c, _counters);
			}
			for ( auto count : c ) {
				_statsd_client->counter(count.first, count.second);
			}
		}
		std::this_thread::sleep_for(std::chrono::seconds(_background_period_s));
	}
}

} } // namespace
