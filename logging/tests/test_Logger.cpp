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

struct TestLog : public ILog {
	map<Severity, vector<Entry> > entries;

	void flush() override { }
protected:
	void addEntry(const Entry& entry) override {
		entries[entry.severity].push_back(entry);
	}
};

struct foo { };
function<Logger&(Logger&, Entry&)> set_foo(int fooVal) {
	return [=] (Logger& l, Entry& e) -> Logger& {
		e.set<foo>(fooVal);
		return l;
	};
}

struct bar { };
function<Logger&(Logger&, Entry&)> set_bar(const string& bar_val) {
	return [=] (Logger& l, Entry& e) -> Logger& {
		e.set<bar>(bar_val);
		return l;
	};
}

int atoi(const logstring& str) {
	int i = numeric_limits<int>::min();
	basic_istringstream<log_char> iss{str};
	iss >> i;
	return i;
}

BOOST_AUTO_TEST_CASE(no_entry_on_high_level) {
	TestLog testLog;
	testLog.setMaxSeverity(EMERGENCY);
	Logger logger(testLog, "TESTFACILITY", INFO);
	logger << DEBUG << "test entry 1" << end_entry;

	BOOST_CHECK(testLog.entries.empty());
}

BOOST_AUTO_TEST_CASE(do_log_on_low_level) {
	TestLog testLog;
	testLog.setMaxSeverity(Severity::DEBUG);
	Logger logger(testLog, "TESTFACILITY", Severity::INFO);
	logger << EMERGENCY << "test entry " << 1 << end_entry;

	BOOST_REQUIRE(not testLog.entries.empty());
	BOOST_REQUIRE(not testLog.entries[EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLog.entries[EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK_EQUAL(testLog.entries[EMERGENCY].front().message, L"test entry 1");
}

BOOST_AUTO_TEST_CASE(do_utf32_log_on_low_level) {
	TestLog testLog;
	testLog.setMaxSeverity(DEBUG);
	Logger logger(testLog, "TESTFACILITY", INFO);
	logger << EMERGENCY << L"test entry " << 1 << end_entry;

	BOOST_REQUIRE(not testLog.entries.empty());
	BOOST_REQUIRE(not testLog.entries[EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLog.entries[EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK_EQUAL(testLog.entries[EMERGENCY].front().message, L"test entry 1");
}

BOOST_AUTO_TEST_CASE(do_utf16_log_on_low_level) {
	TestLog testLog;
	testLog.setMaxSeverity(DEBUG);
	Logger logger(testLog, "TESTFACILITY", INFO);
	logger << EMERGENCY << L"test entry " << 1 << end_entry;

	BOOST_REQUIRE(not testLog.entries.empty());
	BOOST_REQUIRE(not testLog.entries[EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLog.entries[EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK(testLog.entries[EMERGENCY].front().message == L"test entry 1");
}

BOOST_AUTO_TEST_CASE(do_wchar_log_on_low_level) {
	TestLog testLog;
	testLog.setMaxSeverity(DEBUG);
	Logger logger(testLog, "TESTFACILITY", INFO);
	logger << EMERGENCY << L"test entry " << 1 << end_entry;

	BOOST_REQUIRE(not testLog.entries.empty());
	BOOST_REQUIRE(not testLog.entries[EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLog.entries[EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK_EQUAL(testLog.entries[EMERGENCY].front().message, L"test entry 1");
}

BOOST_AUTO_TEST_CASE(log_file_on_low_level) {
	TestLog testLog;
	testLog.setMaxSeverity(DEBUG);
	Logger logger(testLog, "TESTFACILITY", INFO);
	logger << EMERGENCY << file(__FILE__) << end_entry;

	BOOST_REQUIRE(not testLog.entries.empty());
	BOOST_REQUIRE(not testLog.entries[EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLog.entries[EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK_EQUAL(testLog.entries[EMERGENCY].front().message, L"");
	BOOST_CHECK_EQUAL(testLog.entries[EMERGENCY].front().location.file, __FILE__);
}

BOOST_AUTO_TEST_CASE(log_line_on_low_level) {
	TestLog testLog;
	testLog.setMaxSeverity(DEBUG);
	Logger logger(testLog, "TESTFACILITY", INFO);
	logger << EMERGENCY << line(__LINE__) << end_entry;
	auto line = __LINE__-1;

	BOOST_REQUIRE(not testLog.entries.empty());
	BOOST_REQUIRE(not testLog.entries[EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLog.entries[EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK_EQUAL(testLog.entries[EMERGENCY].front().message, L"");
	BOOST_CHECK_EQUAL(testLog.entries[EMERGENCY].front().location.line, line);
}

BOOST_AUTO_TEST_CASE(log_func_on_low_level) {
	TestLog testLog;
	testLog.setMaxSeverity(DEBUG);
	Logger logger(testLog, "TESTFACILITY", INFO);
	logger << EMERGENCY << func(__FUNCTION__) << end_entry;

	BOOST_REQUIRE(not testLog.entries.empty());
	BOOST_REQUIRE(not testLog.entries[EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLog.entries[EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK_EQUAL(testLog.entries[EMERGENCY].front().message, L"");
	BOOST_CHECK_EQUAL(testLog.entries[EMERGENCY].front().location.function, __FUNCTION__);
}

BOOST_AUTO_TEST_CASE(log_foo_on_low_level) {
	TestLog testLog;
	testLog.setMaxSeverity(DEBUG);
	Logger logger(testLog, "TESTFACILITY", INFO);
	logger << EMERGENCY << set_foo(42) << end_entry;

	BOOST_REQUIRE(not testLog.entries.empty());
	BOOST_REQUIRE(not testLog.entries[EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLog.entries[EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK_EQUAL(testLog.entries[EMERGENCY].front().message, L"");

	int foo_val(0);
	auto got_foo = testLog.entries[EMERGENCY].front().get<foo>(foo_val);
	BOOST_CHECK(got_foo);
	BOOST_CHECK_EQUAL(foo_val, 42);
}

BOOST_AUTO_TEST_CASE(log_bar_on_low_level) {
	TestLog testLog;
	testLog.setMaxSeverity(DEBUG);
	Logger logger(testLog, "TESTFACILITY", INFO);
	logger << EMERGENCY << set_bar("din mamma") << end_entry;

	BOOST_REQUIRE(not testLog.entries.empty());
	BOOST_REQUIRE(not testLog.entries[EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLog.entries[EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK_EQUAL(testLog.entries[EMERGENCY].front().message, L"");

	string bar_val;
	auto got_bar = testLog.entries[EMERGENCY].front().get<bar>(bar_val);
	BOOST_CHECK(got_bar);
	BOOST_CHECK_EQUAL(bar_val, "din mamma");
}

struct log_writer {
	log_writer(const string& mess, int nMess) : writer_message(mess), nMessages(nMess) { }

	const string writer_message;
	int nMessages;

	void operator() (Logger& logger) const {
		for(int i=0; i<nMessages; ++i )
			logger << writer_message << ":" << i << end_entry;
	}
};

BOOST_AUTO_TEST_CASE(multithreaded_writes_through_same_logger) {
	TestLog testLog;
	testLog.setMaxSeverity(INFO);
	Logger logger(testLog, "TEST_FACILITY", INFO);

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

	BOOST_REQUIRE(not testLog.entries[INFO].empty());

	int firstWrites(0), secondWrites(0), thirdWrites(0), fourthWrites(0);

	for(const auto& logEntry : testLog.entries[INFO]) {
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
	testLog.setMaxSeverity(INFO);
	Logger logger1(testLog, "TEST_FACILITY_1", INFO);
	Logger logger2(testLog, "TEST_FACILITY_2", INFO);
	Logger logger3(testLog, "TEST_FACILITY_3", INFO);
	Logger logger4(testLog, "TEST_FACILITY_4", INFO);

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

	BOOST_REQUIRE(not testLog.entries[INFO].empty());

	int firstWrites(0), secondWrites(0), thirdWrites(0), fourthWrites(0);

	for(const auto& logEntry : testLog.entries[INFO]) {
		if(logEntry.facility=="TEST_FACILITY_1") {
			basic_ostringstream<log_char> oss;
			oss << "this is a message" << ":" << firstWrites;
			if(logEntry.message == oss.str())
				++firstWrites;
		} else if(logEntry.facility=="TEST_FACILITY_2") {
			basic_ostringstream<log_char> oss;
			oss << "this is a message" << ":" << secondWrites;
			if(logEntry.message == oss.str())
				++secondWrites;
		} else if(logEntry.facility=="TEST_FACILITY_3") {
			basic_ostringstream<log_char> oss;
			oss << "this is a message" << ":" << thirdWrites;
			if(logEntry.message == oss.str())
				++thirdWrites;
		} else if(logEntry.facility=="TEST_FACILITY_4") {
			basic_ostringstream<log_char> oss;
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
	testLog.setMaxSeverity(INFO);
	Logger logger1(testLog, "TEST_FACILITY_1", INFO);
	Logger logger2(testLog, "TEST_FACILITY_2", NOTICE);
	Logger logger3(testLog, "TEST_FACILITY_3", WARNING);
	Logger logger4(testLog, "TEST_FACILITY_4", ERROR);

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

	for(const auto& logEntry : testLog.entries[INFO]) {
		if(logEntry.facility=="TEST_FACILITY_1") {
			basic_ostringstream<log_char> oss;
			oss << "this is a message" << ":" << firstWrites;
			if(logEntry.message == oss.str())
				++firstWrites;
		}
	}


	for(const auto& logEntry : testLog.entries[NOTICE]) {
		if(logEntry.facility=="TEST_FACILITY_2") {
			basic_ostringstream<log_char> oss;
			oss << "this is a message" << ":" << secondWrites;
			if(logEntry.message == oss.str())
				++secondWrites;
		}
	}

	for(const auto& logEntry : testLog.entries[WARNING]) {
		if(logEntry.facility=="TEST_FACILITY_3") {
			basic_ostringstream<log_char> oss;
			oss << "this is a message" << ":" << thirdWrites;
			if(logEntry.message == oss.str())
				++thirdWrites;
		}
	}

	for(const auto& logEntry : testLog.entries[ERROR]) {
		if(logEntry.facility=="TEST_FACILITY_4") {
			basic_ostringstream<log_char> oss;
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

	logger << EMERGENCY << "this should be in both logs" << end_entry;

	BOOST_REQUIRE(not testLogA.entries[EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLogA.entries[EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK(testLogA.entries[EMERGENCY].front().message == L"this should be in both logs");

	BOOST_REQUIRE(not testLogB.entries[EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLogB.entries[EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK(testLogB.entries[EMERGENCY].front().message == L"this should be in both logs");
}

BOOST_AUTO_TEST_CASE(write_to_two_logs_from_one_logger_added_at_ctor) {
	TestLog testLogA;
	TestLog testLogB;
	vector<TestLog*> logs = {&testLogA, &testLogB};
	Logger logger(logs.begin(), logs.end(), "TESTFACILITY");

	logger << EMERGENCY << "this should be in both logs" << end_entry;

	BOOST_REQUIRE(not testLogA.entries[EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLogA.entries[EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK(testLogA.entries[EMERGENCY].front().message == L"this should be in both logs");

	BOOST_REQUIRE(not testLogB.entries[EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLogB.entries[EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK(testLogB.entries[EMERGENCY].front().message == L"this should be in both logs");
}

BOOST_AUTO_TEST_SUITE_END()
