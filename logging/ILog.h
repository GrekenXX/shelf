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
#include <sstream>
#include <typeindex>
#include <unordered_map>
#include <boost/optional.hpp>
#include <boost/any.hpp>

namespace elf {

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

std::string to_string(Severity severity);

typedef wchar_t log_char;
typedef std::basic_string<log_char> logstring;
typedef std::chrono::time_point<std::chrono::high_resolution_clock> logtime;

template<typename T>
struct is_char_literal{ static const bool value = false; };
#define SET_IS_CHAR(TYPE) template<> struct is_char_literal<TYPE>{ static const bool value = true; };
#define IF_IS_CHAR(TYPE, RET) typename std::enable_if<is_char_literal<TYPE>::value, RET>::type
#define IF_IS_NOT_CHAR(TYPE, RET) typename std::enable_if<!is_char_literal<TYPE>::value, RET>::type

SET_IS_CHAR(char)
SET_IS_CHAR(wchar_t)
SET_IS_CHAR(char16_t)
SET_IS_CHAR(char32_t)

template<typename T>
struct logstring_cast {
	static logstring cast(const T& t) {
		std::basic_ostringstream<log_char> oss;
		oss << t;
		return oss.str();
	}
};

template<typename CharT>
struct logstring_cast<std::basic_string<CharT>> {
	static logstring cast(const std::basic_string<CharT>& str) {
		return logstring{std::begin(str), std::end(str)};
	}
};

template<typename CharT, std::size_t N>
struct logstring_cast<CharT[N]> {
	static IF_IS_CHAR(CharT, logstring) cast(const CharT* str) {
		return logstring{str, str + N - 1};
	}
};

template<>
struct logstring_cast<logstring> {
	static logstring cast(const logstring& str) {
		return str;
	}
};

template<typename T>
static logstring to_logstring(const T& t) {
	typedef typename std::remove_reference<typename std::remove_cv<T>::type>::type clean_type;
	return logstring_cast<clean_type>::cast(t);
}

struct Entry {
	struct Location {
		std::string file;
		std::string function;
		int line;
	};

	Entry() :
		severity{DEBUG},
		time{std::chrono::high_resolution_clock::now()}
	{
	}

	template<typename KeyTag, typename T>
	void set(const T& t) { _custom_data[typeid(KeyTag)] = t; }

	template<typename KeyTag, typename T>
	bool get(T& t) {
		auto it = _custom_data.find(typeid(KeyTag));
		if (it!=_custom_data.end() && it->second.type()==typeid(T)) {
			t = boost::any_cast<T>(it->second);
			return true;
		}

		return false;
	}

	template<typename KeyTag>
	bool contains() {
		return (_custom_data.find(typeid(KeyTag)) != _custom_data.end());
	}

	Severity severity;
	logtime time;
	std::string facility;
	Location location;
	logstring message;

private:
	std::unordered_map<std::type_index, boost::any> _custom_data;
};

class ILog {
public:
	ILog(Severity maxSeverity=elf::INFO) : _maxSeverity(maxSeverity) { }
	virtual ~ILog() { }

	Severity getMaxSeverity() const;
	void setMaxSeverity(Severity maxSeverity);

	void handle(const Entry& entry);
	virtual void flush() = 0;

protected:
	virtual void addEntry(const Entry& entry) = 0;

private:
	Severity _maxSeverity;
	std::mutex _mutex;
};

}

#endif /* ILOG_H_ */
