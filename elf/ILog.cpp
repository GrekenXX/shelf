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

void ILog::handle(const Entry& entry) {
	if(entry.severity<=_maxSeverity) {
		lock_guard<mutex> lock(_mutex);
		this->addEntry(entry);
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

Location::Location() : line{-1} {
}

Location::Location(const std::string& _func, const std::string& _file, int _line) :
	function{_func},
	file{_file},
	line{_line} {
}

Location::Location(const Location& loc) :
	function{loc.function},
	file{loc.file},
	line{loc.line} {
}

Location::Location(Location&& loc) :
	function{std::move(loc.function)},
	file{std::move(loc.file)},
	line{loc.line} {
}

Location& Location::operator = (const Location& loc) {
	function = loc.function;
	file = loc.file;
	line = loc.line;
	return *this;
}

Location& Location::operator = (Location&& loc) {
	function = std::move(loc.function);
	file = std::move(loc.file);
	line = loc.line;
	return *this;
}
