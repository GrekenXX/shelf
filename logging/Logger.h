/*
 * Logger.h
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

#ifndef LOGGER_H_
#define LOGGER_H_
#include "ILog.h"
#include <thread>
#include <mutex>

#include <map>
#include <list>
#include <sstream>

namespace logging {

class Logger {
	template<typename T>
	struct LogWriter {
		static void write(Logger& logger, LogEntry& entry, const T& data);
	};

public:
	typedef Logger& (manipulator_t)(Logger&);
	friend Logger& end_entry(Logger&);

	Logger(ILog& log, const std::string& facility, Severity defaultSeverity=logging::INFO);

	template<typename InputIterator>
	Logger(InputIterator logsBegin, InputIterator logsEnd,
			const std::string& facility, Severity defaultSeverity=logging::INFO)
				: _logs(logsBegin, logsEnd), _facility(facility), _defaultSeverity(defaultSeverity) {
	}

	Logger(const std::string& facility, Severity defaultSeverity=logging::INFO);

	Logger(const Logger& logger);

	template<typename T>
	Logger& operator<< (const T& data) {
		LogEntry* entry = NULL;
		{
			std::lock_guard<std::mutex> lock(_currentEntryMutex);
			std::map<std::thread::id, LogEntry>::iterator it = _currentEntries.find(std::this_thread::get_id());
			if(it!=_currentEntries.end()) {
				entry = &(_currentEntries[std::this_thread::get_id()]);
			} else {
				entry = &(_currentEntries[std::this_thread::get_id()]);
				entry->severity = _defaultSeverity;
			}
		}
		LogWriter<T>::write(*this, *entry, data);
		return *this;
	}

	Logger duplicate(const std::string& facility) const;

	void addLog(ILog& log);
	void removeLog(ILog& log);
	const std::list<ILog*>& getLogs() const;

private:
	Logger& operator=(const Logger&);

	std::list<ILog*> _logs;

	const std::string _facility;
	const Severity _defaultSeverity;

	std::mutex _currentEntryMutex;
	std::map<std::thread::id, LogEntry> _currentEntries;

	void flush();
};

Logger& end_entry(Logger&);

// Default behavior: append data to message
template<typename T>
inline void Logger::LogWriter<T>::write(Logger& logger, LogEntry& entry, const T& data) {
	std::ostringstream oss;
	oss << data;
	entry.message += oss.str();
}

template<>
inline void Logger::LogWriter<logging::Severity>::write(Logger& logger, LogEntry& entry, const logging::Severity& severity) {
	entry.severity = severity;
}

template<>
inline void Logger::LogWriter<Logger::manipulator_t>::write(Logger& logger, LogEntry& entry, const Logger::manipulator_t& manipulator) {
	manipulator(logger);
}

}

#endif /* LOGGER_H_ */
