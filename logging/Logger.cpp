/*
 * Logger.cpp
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
#include "Logger.h"
#include <sstream>
#include <algorithm>

using namespace std;
using namespace logging;

Logger& logging::end_entry(Logger& logger) {
	logger.flush();
	return logger;
}

Logger::Logger(ILog& log, const string& facility, Severity defaultSeverity)
	: _logs(1,&log), _facility(facility), _defaultSeverity(defaultSeverity) {

}

Logger::Logger(const string& facility, Severity defaultSeverity)
	: _logs(), _facility(facility), _defaultSeverity(defaultSeverity) {

}

Logger::Logger(const Logger& logger) :
	_logs(logger._logs.begin(), logger._logs.end()),
	_facility(logger._facility),
	_defaultSeverity(logger._defaultSeverity) {
}

Logger Logger::duplicate(const std::string& facility) const {
	return Logger(_logs.begin(), _logs.end(), facility, _defaultSeverity);
}

void Logger::addLog(ILog& log) {
	if(std::find(_logs.begin(), _logs.end(), &log)==_logs.end())
		_logs.push_back(&log);
}

void Logger::removeLog(ILog& log) {
	_logs.erase(std::find(_logs.begin(), _logs.end(), &log));
}

const std::list<ILog*>& Logger::getLogs() const {
	return _logs;
}

void Logger::flush() {
	std::lock_guard<std::mutex> lock_current_entry(_currentEntryMutex);
	map<std::thread::id, LogEntry>::iterator entryIt = _currentEntries.find(std::this_thread::get_id());
	if(entryIt!=_currentEntries.end()) {
		LogEntry& entry = entryIt->second;
		for(auto log : _logs)
			log->addEntry(_facility, entry);

		_currentEntries.erase(entryIt);
	}
}
