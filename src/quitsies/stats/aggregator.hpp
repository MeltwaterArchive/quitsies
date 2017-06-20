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

#ifndef QUITSIES_AGGREGATOR
#define QUITSIES_AGGREGATOR

#include <memory>
#include <functional>

namespace quitsies { namespace stats {

typedef int64_t  value_t;
typedef uint64_t uvalue_t;

class aggregator {
public:
	virtual void counter(std::string const& name, value_t const value) = 0;

	// Record a timer statistic with the given name, and the value, which
	// should be specified in milliseconds.
	virtual void timer(std::string const& name, uvalue_t const value) = 0;

	// Record a gauge statistic with the given name, and the given value.
	virtual void gauge(std::string const& name, uvalue_t const value) = 0;

	// Record a set statistic with the given name and the given value.
	virtual void set(std::string const& name, value_t value) = 0;

	// Stats are aggregated in epochs, if a metric is expensive to calculate you
	// can create them within a function that is called only once per epoch.
	virtual void on_epoch(std::function<void()> call) = 0;
};

typedef std::shared_ptr<aggregator> aggregator_ptr;

} } // namespace

#endif // QUITSIES_AGGREGATOR
