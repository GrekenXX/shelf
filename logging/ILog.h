/*
 * ILog.h
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
#ifndef ILOG_H_
#define ILOG_H_
#include <string>
#include <mutex>
#include <chrono>

namespace logging {

/**
 * These values conform to syslog.
 */
enum Severity {
	EMERGENCY		= 0,
	ALERT			= 1,
	CRITICAL		= 2,
	ERROR			= 3,
	WARNING			= 4,
	NOTICE			= 5,
	INFO			= 6,
	DEBUG			= 7,
};

#define LEVEL_STR(level) #level

struct LogEntry {
	LogEntry() :
		severity(DEBUG), time(std::chrono::high_resolution_clock::now()) {
	}

	Severity severity;
	std::chrono::time_point<std::chrono::high_resolution_clock> time;
	std::string message;
};

class ILog {
public:
	ILog(Severity maxSeverity=logging::INFO) : _maxSeverity(maxSeverity) { }
	virtual ~ILog() { }

	Severity getMaxSeverity() const;
	void setMaxSeverity(Severity maxSeverity);

	void addEntry(const std::string& facility, const LogEntry& entry);
	virtual void flush() = 0;

	static std::string toString(Severity severity);

protected:
	virtual void addEntryImpl(const std::string& facility, const LogEntry& entry) = 0;

private:
	Severity _maxSeverity;
	std::mutex _mutex;
};

}

#endif /* ILOG_H_ */
