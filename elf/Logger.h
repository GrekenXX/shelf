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
#include <type_traits>

namespace elf {

class Logger {
	template<typename T>
	struct LogWriter
	{ static void write(Logger& logger, Entry& entry, const T& data); };

public:
	typedef std::function<Logger&(Logger&,Entry&)> manipulator_t;
	typedef Logger& (manipulator_sig_t)(Logger&, Entry&);

	Logger(ILog& log, const std::string& facility, Severity defaultSeverity=elf::INFO);

	template<typename InputIterator>
	Logger(InputIterator logsBegin, InputIterator logsEnd,
			const std::string& facility, Severity defaultSeverity=elf::INFO)
				: _logs(logsBegin, logsEnd), _facility(facility), _defaultSeverity(defaultSeverity) {
	}

	Logger(const std::string& facility, Severity defaultSeverity=elf::INFO);

	Logger(const Logger& logger);

	template<typename T>
	Logger& operator<< (const T& data) {
		Entry* entry = nullptr;
		{
			std::lock_guard<std::mutex> lock(_currentEntryMutex);
			std::map<std::thread::id, Entry>::iterator it = _currentEntries.find(std::this_thread::get_id());
			if(it!=_currentEntries.end()) {
				entry = &(_currentEntries[std::this_thread::get_id()]);
			} else {
				entry = &(_currentEntries[std::this_thread::get_id()]);
				entry->severity = _defaultSeverity;
				entry->facility = _facility;
			}
		}
		LogWriter<T>::write(*this, *entry, data);
		return *this;
	}

	Logger duplicate(const std::string& facility) const;

	void addLog(ILog& log);
	void removeLog(ILog& log);
	const std::list<ILog*>& getLogs() const;
	void flush();

private:
	Logger& operator=(const Logger&);

	std::list<ILog*> _logs;

	const std::string _facility;
	const Severity _defaultSeverity;

	std::mutex _currentEntryMutex;
	std::map<std::thread::id, Entry> _currentEntries;

};

// Default behavior: append data to message
template<typename T>
inline void Logger::LogWriter<T>::write(Logger&, Entry& entry, const T& data) {
	entry.message += to_logstring(data);
}

template<>
inline void Logger::LogWriter<elf::Severity>::write(Logger&, Entry& entry, const elf::Severity& severity) {
	entry.severity = severity;
}

template<>
inline void Logger::LogWriter<Logger::manipulator_t>::write(Logger& logger, Entry& entry, const Logger::manipulator_t& manipulator) {
	manipulator(logger, entry);
}

template<>
inline void Logger::LogWriter<Logger::manipulator_sig_t>::write(Logger& logger, Entry& entry, const Logger::manipulator_sig_t& manipulator) {
	manipulator(logger, entry);
}

Logger& end_entry(Logger&, Entry&);
std::function<Logger&(Logger&, Entry&)> file(const std::string& _file);
std::function<Logger&(Logger&, Entry&)> line(int _line);
std::function<Logger&(Logger&, Entry&)> func(const std::string& _func);
std::function<Logger&(Logger&, Entry&)> location(const std::string& _func, const std::string& _file, int line);

}

#define ELF_FILE elf::file(__FILE__)
#define ELF_LINE elf::line(__LINE__)
#define ELF_FUNC elf::func(__func__)
#define ELF_LOC elf::location(__func__,__FILE__,__LINE__)

#endif /* LOGGER_H_ */
