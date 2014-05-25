/*
 * test_Logger.cpp
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
#define BOOST_TEST_MODULE test_logger
#include <boost/test/included/unit_test.hpp>
#include "../Logger.h"
#include <map>
#include <list>

using namespace std;
using namespace elf;

namespace std {
ostream& operator << (ostream& os, const logstring& lstr) {
	return (os << string{begin(lstr), end(lstr)});
}
}

BOOST_AUTO_TEST_SUITE(test_logger)

struct foo { };
function<Logger&(Logger&, LogEntry&)> set_foo(int fooVal) {
	return [=] (Logger& l, LogEntry& e) -> Logger& {
		e.set<foo>(fooVal);
		return l;
	};
}

struct bar { };
function<Logger&(Logger&, LogEntry&)> set_bar(const string& bar_val) {
	return [=] (Logger& l, LogEntry& e) -> Logger& {
		e.set<bar>(bar_val);
		return l;
	};
}

struct TestLog : public ILog {
	map<elf::Severity, vector<LogEntry> > entries;

	virtual void flush() { }
protected:
	virtual void addEntryImpl(const LogEntry& entry) {
		entries[entry.severity].push_back(entry);
	}
};

int atoi(const logstring& str) {
	int i = numeric_limits<int>::min();
	basic_istringstream<elf::log_char> iss{str};
	iss >> i;
	return i;
}

BOOST_AUTO_TEST_CASE(no_entry_on_high_level) {
	TestLog testLog;
	testLog.setMaxSeverity(elf::EMERGENCY);
	Logger logger(testLog, "TESTFACILITY", elf::INFO);
	logger << elf::DEBUG << "test entry 1" << elf::end_entry;

	BOOST_CHECK(testLog.entries.empty());
}

BOOST_AUTO_TEST_CASE(do_log_on_low_level) {
	TestLog testLog;
	testLog.setMaxSeverity(elf::DEBUG);
	Logger logger(testLog, "TESTFACILITY", elf::INFO);
	logger << elf::EMERGENCY << "test entry " << 1 << elf::end_entry;

	BOOST_REQUIRE(not testLog.entries.empty());
	BOOST_REQUIRE(not testLog.entries[elf::EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLog.entries[elf::EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK_EQUAL(testLog.entries[elf::EMERGENCY].front().message, L"test entry 1");
}

BOOST_AUTO_TEST_CASE(do_utf32_log_on_low_level) {
	TestLog testLog;
	testLog.setMaxSeverity(elf::DEBUG);
	Logger logger(testLog, "TESTFACILITY", elf::INFO);
	logger << elf::EMERGENCY << L"test entry " << 1 << elf::end_entry;

	BOOST_REQUIRE(not testLog.entries.empty());
	BOOST_REQUIRE(not testLog.entries[elf::EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLog.entries[elf::EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK_EQUAL(testLog.entries[elf::EMERGENCY].front().message, L"test entry 1");
}

BOOST_AUTO_TEST_CASE(do_utf16_log_on_low_level) {
	TestLog testLog;
	testLog.setMaxSeverity(elf::DEBUG);
	Logger logger(testLog, "TESTFACILITY", elf::INFO);
	logger << elf::EMERGENCY << L"test entry " << 1 << elf::end_entry;

	BOOST_REQUIRE(not testLog.entries.empty());
	BOOST_REQUIRE(not testLog.entries[elf::EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLog.entries[elf::EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK(testLog.entries[elf::EMERGENCY].front().message == L"test entry 1");
}

BOOST_AUTO_TEST_CASE(do_wchar_log_on_low_level) {
	TestLog testLog;
	testLog.setMaxSeverity(elf::DEBUG);
	Logger logger(testLog, "TESTFACILITY", elf::INFO);
	logger << elf::EMERGENCY << L"test entry " << 1 << elf::end_entry;

	BOOST_REQUIRE(not testLog.entries.empty());
	BOOST_REQUIRE(not testLog.entries[elf::EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLog.entries[elf::EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK_EQUAL(testLog.entries[elf::EMERGENCY].front().message, L"test entry 1");
}

BOOST_AUTO_TEST_CASE(log_file_on_low_level) {
	TestLog testLog;
	testLog.setMaxSeverity(elf::DEBUG);
	Logger logger(testLog, "TESTFACILITY", elf::INFO);
	logger << elf::EMERGENCY << elf::file(__FILE__) << elf::end_entry;

	BOOST_REQUIRE(not testLog.entries.empty());
	BOOST_REQUIRE(not testLog.entries[elf::EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLog.entries[elf::EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK_EQUAL(testLog.entries[elf::EMERGENCY].front().message, L"");
	BOOST_CHECK_EQUAL(testLog.entries[elf::EMERGENCY].front().location.file, __FILE__);
}

BOOST_AUTO_TEST_CASE(log_line_on_low_level) {
	TestLog testLog;
	testLog.setMaxSeverity(elf::DEBUG);
	Logger logger(testLog, "TESTFACILITY", elf::INFO);
	logger << elf::EMERGENCY << elf::line(__LINE__) << elf::end_entry;
	auto line = __LINE__-1;

	BOOST_REQUIRE(not testLog.entries.empty());
	BOOST_REQUIRE(not testLog.entries[elf::EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLog.entries[elf::EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK_EQUAL(testLog.entries[elf::EMERGENCY].front().message, L"");
	BOOST_CHECK_EQUAL(testLog.entries[elf::EMERGENCY].front().location.line, line);
}

BOOST_AUTO_TEST_CASE(log_func_on_low_level) {
	TestLog testLog;
	testLog.setMaxSeverity(elf::DEBUG);
	Logger logger(testLog, "TESTFACILITY", elf::INFO);
	logger << elf::EMERGENCY << elf::func(__FUNCTION__) << elf::end_entry;

	BOOST_REQUIRE(not testLog.entries.empty());
	BOOST_REQUIRE(not testLog.entries[elf::EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLog.entries[elf::EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK_EQUAL(testLog.entries[elf::EMERGENCY].front().message, L"");
	BOOST_CHECK_EQUAL(testLog.entries[elf::EMERGENCY].front().location.function, __FUNCTION__);
}

BOOST_AUTO_TEST_CASE(log_foo_on_low_level) {
	TestLog testLog;
	testLog.setMaxSeverity(elf::DEBUG);
	Logger logger(testLog, "TESTFACILITY", elf::INFO);
	logger << elf::EMERGENCY << set_foo(42) << elf::end_entry;

	BOOST_REQUIRE(not testLog.entries.empty());
	BOOST_REQUIRE(not testLog.entries[elf::EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLog.entries[elf::EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK_EQUAL(testLog.entries[elf::EMERGENCY].front().message, L"");

	int foo_val(0);
	auto got_foo = testLog.entries[elf::EMERGENCY].front().get<foo>(foo_val);
	BOOST_CHECK(got_foo);
	BOOST_CHECK_EQUAL(foo_val, 42);
}

BOOST_AUTO_TEST_CASE(log_bar_on_low_level) {
	TestLog testLog;
	testLog.setMaxSeverity(elf::DEBUG);
	Logger logger(testLog, "TESTFACILITY", elf::INFO);
	logger << elf::EMERGENCY << set_bar("din mamma") << elf::end_entry;

	BOOST_REQUIRE(not testLog.entries.empty());
	BOOST_REQUIRE(not testLog.entries[elf::EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLog.entries[elf::EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK_EQUAL(testLog.entries[elf::EMERGENCY].front().message, L"");

	string bar_val;
	auto got_bar = testLog.entries[elf::EMERGENCY].front().get<bar>(bar_val);
	BOOST_CHECK(got_bar);
	BOOST_CHECK_EQUAL(bar_val, "din mamma");
}

struct log_writer {
	log_writer(const string& mess, int nMess) : writer_message(mess), nMessages(nMess) { }

	const string writer_message;
	int nMessages;

	void operator() (Logger& logger) const {
		for(int i=0; i<nMessages; ++i )
			logger << writer_message << ":" << i << elf::end_entry;
	}
};

BOOST_AUTO_TEST_CASE(multithreaded_writes_through_same_logger) {
	TestLog testLog;
	testLog.setMaxSeverity(elf::INFO);
	Logger logger(testLog, "TEST_FACILITY", elf::INFO);

	log_writer writer1("this is writer1 writing", 100);
	log_writer writer2("this is writer2 writing", 200);
	log_writer writer3("this is writer3 writing", 300);
	log_writer writer4("this is writer4 writing", 400);

	{
		thread firstThread(writer1, ref(logger));
		thread secondThread(writer2, ref(logger));
		thread thirdThread(writer3, ref(logger));
		thread fourthThread(writer4, ref(logger));

		firstThread.join();
		secondThread.join();
		thirdThread.join();
		fourthThread.join();
	}

	BOOST_REQUIRE(not testLog.entries[elf::INFO].empty());

	int firstWrites(0), secondWrites(0), thirdWrites(0), fourthWrites(0);

	for(const auto& logEntry : testLog.entries[elf::INFO]) {
		if(logEntry.facility=="TEST_FACILITY") {
			if(logEntry.message.find(L"this is writer1 writing:")==0 && atoi(logEntry.message.substr(24))==firstWrites)
				++firstWrites;
			else if(logEntry.message.find(L"this is writer2 writing:")==0 && atoi(logEntry.message.substr(24))==secondWrites)
				++secondWrites;
			else if(logEntry.message.find(L"this is writer3 writing:")==0 && atoi(logEntry.message.substr(24))==thirdWrites)
				++thirdWrites;
			else if(logEntry.message.find(L"this is writer4 writing:")==0 && atoi(logEntry.message.substr(24))==fourthWrites)
				++fourthWrites;
		}
	}

	BOOST_CHECK_EQUAL(firstWrites, 100);
	BOOST_CHECK_EQUAL(secondWrites, 200);
	BOOST_CHECK_EQUAL(thirdWrites, 300);
	BOOST_CHECK_EQUAL(fourthWrites, 400);
}

BOOST_AUTO_TEST_CASE(multithreaded_writes_through_different_loggers) {
	TestLog testLog;
	testLog.setMaxSeverity(elf::INFO);
	Logger logger1(testLog, "TEST_FACILITY_1", elf::INFO);
	Logger logger2(testLog, "TEST_FACILITY_2", elf::INFO);
	Logger logger3(testLog, "TEST_FACILITY_3", elf::INFO);
	Logger logger4(testLog, "TEST_FACILITY_4", elf::INFO);

	log_writer writer("this is a message", 100);

	{
		thread firstThread(writer, ref(logger1));
		thread secondThread(writer, ref(logger2));
		thread thirdThread(writer, ref(logger3));
		thread fourthThread(writer, ref(logger4));

		firstThread.join();
		secondThread.join();
		thirdThread.join();
		fourthThread.join();
	}

	BOOST_REQUIRE(not testLog.entries[elf::INFO].empty());

	int firstWrites(0), secondWrites(0), thirdWrites(0), fourthWrites(0);

	for(const auto& logEntry : testLog.entries[elf::INFO]) {
		if(logEntry.facility=="TEST_FACILITY_1") {
			basic_ostringstream<elf::log_char> oss;
			oss << "this is a message" << ":" << firstWrites;
			if(logEntry.message == oss.str())
				++firstWrites;
		} else if(logEntry.facility=="TEST_FACILITY_2") {
			basic_ostringstream<elf::log_char> oss;
			oss << "this is a message" << ":" << secondWrites;
			if(logEntry.message == oss.str())
				++secondWrites;
		} else if(logEntry.facility=="TEST_FACILITY_3") {
			basic_ostringstream<elf::log_char> oss;
			oss << "this is a message" << ":" << thirdWrites;
			if(logEntry.message == oss.str())
				++thirdWrites;
		} else if(logEntry.facility=="TEST_FACILITY_4") {
			basic_ostringstream<elf::log_char> oss;
			oss << "this is a message" << ":" << fourthWrites;
			if(logEntry.message == oss.str())
				++fourthWrites;
		}
	}

	BOOST_CHECK_EQUAL(firstWrites, 100);
	BOOST_CHECK_EQUAL(secondWrites, 100);
	BOOST_CHECK_EQUAL(thirdWrites, 100);
	BOOST_CHECK_EQUAL(fourthWrites, 100);
}

BOOST_AUTO_TEST_CASE(multithreaded_writes_through_different_loggers_at_different_levels) {
	TestLog testLog;
	testLog.setMaxSeverity(elf::INFO);
	Logger logger1(testLog, "TEST_FACILITY_1", elf::INFO);
	Logger logger2(testLog, "TEST_FACILITY_2", elf::NOTICE);
	Logger logger3(testLog, "TEST_FACILITY_3", elf::WARNING);
	Logger logger4(testLog, "TEST_FACILITY_4", elf::ERROR);

	log_writer writer("this is a message", 100);

	{
		thread firstThread(writer, ref(logger1));
		thread secondThread(writer, ref(logger2));
		thread thirdThread(writer, ref(logger3));
		thread fourthThread(writer, ref(logger4));

		firstThread.join();
		secondThread.join();
		thirdThread.join();
		fourthThread.join();
	}

	int firstWrites(0), secondWrites(0), thirdWrites(0), fourthWrites(0);

	for(const auto& logEntry : testLog.entries[elf::INFO]) {
		if(logEntry.facility=="TEST_FACILITY_1") {
			basic_ostringstream<elf::log_char> oss;
			oss << "this is a message" << ":" << firstWrites;
			if(logEntry.message == oss.str())
				++firstWrites;
		}
	}


	for(const auto& logEntry : testLog.entries[elf::NOTICE]) {
		if(logEntry.facility=="TEST_FACILITY_2") {
			basic_ostringstream<elf::log_char> oss;
			oss << "this is a message" << ":" << secondWrites;
			if(logEntry.message == oss.str())
				++secondWrites;
		}
	}

	for(const auto& logEntry : testLog.entries[elf::WARNING]) {
		if(logEntry.facility=="TEST_FACILITY_3") {
			basic_ostringstream<elf::log_char> oss;
			oss << "this is a message" << ":" << thirdWrites;
			if(logEntry.message == oss.str())
				++thirdWrites;
		}
	}

	for(const auto& logEntry : testLog.entries[elf::ERROR]) {
		if(logEntry.facility=="TEST_FACILITY_4") {
			basic_ostringstream<elf::log_char> oss;
			oss << "this is a message" << ":" << fourthWrites;
			if(logEntry.message == oss.str())
				++fourthWrites;
		}
	}

	BOOST_CHECK_EQUAL(firstWrites, 100);
	BOOST_CHECK_EQUAL(secondWrites, 100);
	BOOST_CHECK_EQUAL(thirdWrites, 100);
	BOOST_CHECK_EQUAL(fourthWrites, 100);
}

BOOST_AUTO_TEST_CASE(write_to_two_logs_from_one_logger_added_separetely) {
	TestLog testLogA, testLogB;
	Logger logger(testLogA, "TESTFACILITY");
	logger.addLog(testLogB);

	logger << elf::EMERGENCY << "this should be in both logs" << elf::end_entry;

	BOOST_REQUIRE(not testLogA.entries[elf::EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLogA.entries[elf::EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK(testLogA.entries[elf::EMERGENCY].front().message == L"this should be in both logs");

	BOOST_REQUIRE(not testLogB.entries[elf::EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLogB.entries[elf::EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK(testLogB.entries[elf::EMERGENCY].front().message == L"this should be in both logs");
}

BOOST_AUTO_TEST_CASE(write_to_two_logs_from_one_logger_added_at_ctor) {
	TestLog testLogA;
	TestLog testLogB;
	vector<TestLog*> logs = {&testLogA, &testLogB};
	Logger logger(logs.begin(), logs.end(), "TESTFACILITY");

	logger << elf::EMERGENCY << "this should be in both logs" << elf::end_entry;

	BOOST_REQUIRE(not testLogA.entries[elf::EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLogA.entries[elf::EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK(testLogA.entries[elf::EMERGENCY].front().message == L"this should be in both logs");

	BOOST_REQUIRE(not testLogB.entries[elf::EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLogB.entries[elf::EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK(testLogB.entries[elf::EMERGENCY].front().message == L"this should be in both logs");
}

BOOST_AUTO_TEST_SUITE_END()
