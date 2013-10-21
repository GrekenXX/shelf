/*
 * Logger.h
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
		LogEntry* entry = nullptr;
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
