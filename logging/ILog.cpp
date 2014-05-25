/*
 * ILog.cpp
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
#include "ILog.h"
#include <mutex>

using namespace std;
using namespace elf;

Severity ILog::getMaxSeverity() const {
	return _maxSeverity;
}

void ILog::setMaxSeverity(Severity severity) {
	_maxSeverity = severity;
}

void ILog::addEntry(const LogEntry& entry) {
	if(entry.severity<=_maxSeverity) {
		lock_guard<mutex> lock(_mutex);
		this->addEntryImpl(entry);
	}
}

string elf::to_string(Severity severity) {
	switch(severity) {
	case EMERGENCY:		return "EMERGENCY";
	case ALERT:			return "ALERT";
	case CRITICAL:		return "CRITICAL";
	case ERROR:			return "ERROR";
	case WARNING:		return "WARNING";
	case NOTICE:		return "NOTICE";
	case INFO:			return "INFO";
	case DEBUG:			return "DEBUG";
	}

	return "UNKNOWN_LOG_LEVEL";
}
