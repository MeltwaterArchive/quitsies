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

#ifndef NULL_AGGREGATOR
#define NULL_AGGREGATOR

#include <quitsies/stats/aggregator.hpp>

namespace quitsies { namespace stats {

// null_aggregator does nothing, for when we want to disable metrics.
class null_aggregator : public aggregator {
public:
	inline void counter(std::string const& name, value_t const value) {}
	inline void timer(std::string const& name, uvalue_t const value) {}
	inline void gauge(std::string const& name, uvalue_t const value) {}
	inline void set(std::string const& name, value_t value) {}
	inline void on_epoch(std::function<void()> call) {}
};

} } // namespace

#endif // NULL_AGGREGATOR
