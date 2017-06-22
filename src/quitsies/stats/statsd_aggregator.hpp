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

#ifndef STATSD_AGGREGATOR
#define STATSD_AGGREGATOR

#include <map>
#include <mutex>
#include <thread>
#include <atomic>

#include <quitsies/stats/aggregator.hpp>
#include <quitsies/stats/statsd.hpp>

namespace quitsies { namespace stats {

class statsd_aggregator : public aggregator {
	typedef value_t                       count;
	typedef uvalue_t                      ucount;

	typedef std::map<std::string, count>  sets;
	typedef std::map<std::string, ucount> timers;
	typedef std::map<std::string, ucount> gauges;
	typedef std::map<std::string, count>  counters;

	sets      _sets;
	timers    _timers;
	gauges    _gauges;
	counters  _counters;

	std::mutex _sets_mutex;
	std::mutex _timers_mutex;
	std::mutex _gauges_mutex;
	std::mutex _counters_mutex;

	std::unique_ptr<statsd> _statsd_client;

	long              _background_period_s;
	std::atomic<bool> _background_running;
	std::thread       _background_thread;

	std::mutex _epochs_mutex;
	std::vector<std::function<void()>> _epoch_calls;

public:
	statsd_aggregator(std::string const& host, std::string const port, std::string const& prefix, long period_s = 1)
		: _sets()
		, _timers()
		, _gauges()
		, _counters()
		, _statsd_client(new statsd(host, port, prefix))
		, _background_period_s(period_s)
		, _background_running(true)
		, _epoch_calls()
	{
		_background_thread = std::thread(&statsd_aggregator::background_loop, this);
	}

	~statsd_aggregator() {
		_background_running = false;
		_background_thread.join();
	}

	void counter(std::string const& name, value_t const value);

	// Record a timer statistic with the given name, and the value, which
	// should be specified in milliseconds.
	void timer(std::string const& name, uvalue_t const value);

	// Record a gauge statistic with the given name, and the given value.
	void gauge(std::string const& name, uvalue_t const value);

	// Record a set statistic with the given name and the given value.
	void set(std::string const& name, value_t value);

	void on_epoch(std::function<void()> call) {
		std::lock_guard<std::mutex> guard(_epochs_mutex);
		_epoch_calls.push_back(call);
	}

private:
	void background_loop();
};

} } // namespace

#endif // STATSD_AGGREGATOR
