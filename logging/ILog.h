/*
 * ILog.h
 *
 *  Created on: Oct 19, 2013
 *      Author: Georgios Dimitriadis
 *
 * Copyright (c) 2013, Georgios Dimitriadis
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 *   Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
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
