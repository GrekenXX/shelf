/*
 * StreamLog.h
 *
 *  Created on: Oct 19, 2013
 *      Author: Georgios Dimitriadis
 *
 * Copyright (c) 2013, Georgios Dimitriadis
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef STREAMLOG_H_
#define STREAMLOG_H_
#include "ILog.h"
#include <ostream>

namespace elf {

class StreamLog : public ILog {
public:
	StreamLog(std::ostream& os, Severity maxSeverity=elf::INFO) : ILog(maxSeverity), _os(os) { }

	void flush() override { _os.flush(); };

protected:
	void addEntry(const Entry& entry) override {
		typedef std::chrono::seconds secs_t;
		typedef std::chrono::milliseconds millis_t;
		int64_t secs = std::chrono::duration_cast<secs_t>(entry.time.time_since_epoch()).count();
		int64_t millis = std::chrono::duration_cast<millis_t>(entry.time.time_since_epoch()).count() - 1000*secs;
		_os << "|" << to_string(entry.severity) <<
			   "|" << secs << "." << millis <<
			   "|" << entry.facility <<
			   "|" << entry.message <<
			   std::endl;
	}

private:
	std::ostream& _os;
};

}

#endif /* STREAMLOG_H_ */
