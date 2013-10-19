/*
 * Logger.cpp
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
