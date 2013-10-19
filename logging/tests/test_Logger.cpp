/*
 * test_Logger.cpp
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
#include <boost/test/unit_test.hpp>
#include "../Logger.h"
#include <map>
#include <list>

using namespace std;
using namespace logging;

BOOST_AUTO_TEST_SUITE(test_logger)

struct TestLog : public ILog {
	struct TestEntry {
		TestEntry(const string& facility_, const LogEntry& entry_) : facility(facility_), entry(entry_) { }
		string facility;
		LogEntry entry;
	};

	map<logging::Severity, list<TestEntry> > entries;

	virtual void flush() { }
protected:
	virtual void addEntryImpl(const string& facility, const LogEntry& entry) {
		entries[entry.severity].push_back(TestEntry(facility, entry));
	}
};

BOOST_AUTO_TEST_CASE(no_entry_on_high_level) {
	TestLog testLog;
	testLog.setMaxSeverity(logging::EMERGENCY);
	Logger logger(testLog, "TESTFACILITY", logging::INFO);
	logger << logging::DEBUG << "test entry 1" << logging::end_entry;

	BOOST_CHECK(testLog.entries.empty());
}

BOOST_AUTO_TEST_CASE(do_log_on_low_level) {
	TestLog testLog;
	testLog.setMaxSeverity(logging::DEBUG);
	Logger logger(testLog, "TESTFACILITY", logging::INFO);
	logger << logging::EMERGENCY << "test entry " << 1 << logging::end_entry;

	BOOST_REQUIRE(not testLog.entries.empty());
	BOOST_REQUIRE(not testLog.entries[logging::EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLog.entries[logging::EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK_EQUAL(testLog.entries[logging::EMERGENCY].front().entry.message, "test entry 1");
}

struct log_writer {
	log_writer(const std::string& mess, int nMess) : writer_message(mess), nMessages(nMess) { }

	const std::string writer_message;
	int nMessages;

	void operator() (Logger& logger) const {
		for(int i=0; i<nMessages; ++i )
			logger << writer_message << ":" << i << logging::end_entry;
	}
};

BOOST_AUTO_TEST_CASE(multithreaded_writes_through_same_logger) {
	TestLog testLog;
	testLog.setMaxSeverity(logging::INFO);
	Logger logger(testLog, "TEST_FACILITY", logging::INFO);

	log_writer writer1("this is writer1 writing", 100);
	log_writer writer2("this is writer2 writing", 200);
	log_writer writer3("this is writer3 writing", 300);
	log_writer writer4("this is writer4 writing", 400);

	{
		thread firstThread(writer1, std::ref(logger));
		thread secondThread(writer2, std::ref(logger));
		thread thirdThread(writer3, std::ref(logger));
		thread fourthThread(writer4, std::ref(logger));

		firstThread.join();
		secondThread.join();
		thirdThread.join();
		fourthThread.join();
	}

	BOOST_REQUIRE(not testLog.entries[logging::INFO].empty());

	int firstWrites(0), secondWrites(0), thirdWrites(0), fourthWrites(0);

	std::list<TestLog::TestEntry>::const_iterator logEntry = testLog.entries[logging::INFO].begin();
	while(logEntry!=testLog.entries[logging::INFO].end()) {
		if(logEntry->facility=="TEST_FACILITY") {
			if(logEntry->entry.message.find("this is writer1 writing:")==0 && atoi(logEntry->entry.message.substr(24).c_str())==firstWrites)
				++firstWrites;
			else if(logEntry->entry.message.find("this is writer2 writing:")==0 && atoi(logEntry->entry.message.substr(24).c_str())==secondWrites)
				++secondWrites;
			else if(logEntry->entry.message.find("this is writer3 writing:")==0 && atoi(logEntry->entry.message.substr(24).c_str())==thirdWrites)
				++thirdWrites;
			else if(logEntry->entry.message.find("this is writer4 writing:")==0 && atoi(logEntry->entry.message.substr(24).c_str())==fourthWrites)
				++fourthWrites;
		}
		++logEntry;
	}

	BOOST_CHECK_EQUAL(firstWrites, 100);
	BOOST_CHECK_EQUAL(secondWrites, 200);
	BOOST_CHECK_EQUAL(thirdWrites, 300);
	BOOST_CHECK_EQUAL(fourthWrites, 400);
}

BOOST_AUTO_TEST_CASE(multithreaded_writes_through_different_loggers) {
	TestLog testLog;
	testLog.setMaxSeverity(logging::INFO);
	Logger logger1(testLog, "TEST_FACILITY_1", logging::INFO);
	Logger logger2(testLog, "TEST_FACILITY_2", logging::INFO);
	Logger logger3(testLog, "TEST_FACILITY_3", logging::INFO);
	Logger logger4(testLog, "TEST_FACILITY_4", logging::INFO);

	log_writer writer("this is a message", 100);

	{
		thread firstThread(writer, std::ref(logger1));
		thread secondThread(writer, std::ref(logger2));
		thread thirdThread(writer, std::ref(logger3));
		thread fourthThread(writer, std::ref(logger4));

		firstThread.join();
		secondThread.join();
		thirdThread.join();
		fourthThread.join();
	}

	BOOST_REQUIRE(not testLog.entries[logging::INFO].empty());

	int firstWrites(0), secondWrites(0), thirdWrites(0), fourthWrites(0);

	std::list<TestLog::TestEntry>::const_iterator logEntry = testLog.entries[logging::INFO].begin();
	while(logEntry!=testLog.entries[logging::INFO].end()) {
		if(logEntry->facility=="TEST_FACILITY_1") {
			std::ostringstream oss; oss << "this is a message" << ":" << firstWrites;
			if(logEntry->entry.message == oss.str())
				++firstWrites;
		}
		else if(logEntry->facility=="TEST_FACILITY_2") {
			std::ostringstream oss; oss << "this is a message" << ":" << secondWrites;
			if(logEntry->entry.message == oss.str())
				++secondWrites;
		}
		else if(logEntry->facility=="TEST_FACILITY_3") {
			std::ostringstream oss; oss << "this is a message" << ":" << thirdWrites;
			if(logEntry->entry.message == oss.str())
				++thirdWrites;
		}
		else if(logEntry->facility=="TEST_FACILITY_4") {
			std::ostringstream oss; oss << "this is a message" << ":" << fourthWrites;
			if(logEntry->entry.message == oss.str())
				++fourthWrites;
		}
		++logEntry;
	}

	BOOST_CHECK_EQUAL(firstWrites, 100);
	BOOST_CHECK_EQUAL(secondWrites, 100);
	BOOST_CHECK_EQUAL(thirdWrites, 100);
	BOOST_CHECK_EQUAL(fourthWrites, 100);
}

BOOST_AUTO_TEST_CASE(multithreaded_writes_through_different_loggers_at_different_levels) {
	TestLog testLog;
	testLog.setMaxSeverity(logging::INFO);
	Logger logger1(testLog, "TEST_FACILITY_1", logging::INFO);
	Logger logger2(testLog, "TEST_FACILITY_2", logging::NOTICE);
	Logger logger3(testLog, "TEST_FACILITY_3", logging::WARNING);
	Logger logger4(testLog, "TEST_FACILITY_4", logging::ERROR);

	log_writer writer("this is a message", 100);

	{
		thread firstThread(writer, std::ref(logger1));
		thread secondThread(writer, std::ref(logger2));
		thread thirdThread(writer, std::ref(logger3));
		thread fourthThread(writer, std::ref(logger4));

		firstThread.join();
		secondThread.join();
		thirdThread.join();
		fourthThread.join();
	}

	int firstWrites(0), secondWrites(0), thirdWrites(0), fourthWrites(0);

	std::list<TestLog::TestEntry>::const_iterator logEntry = testLog.entries[logging::INFO].begin();
	while(logEntry!=testLog.entries[logging::INFO].end()) {
		if(logEntry->facility=="TEST_FACILITY_1") {
			std::ostringstream oss; oss << "this is a message" << ":" << firstWrites;
			if(logEntry->entry.message == oss.str())
				++firstWrites;
		}
		++logEntry;
	}


	logEntry = testLog.entries[logging::NOTICE].begin();
	while(logEntry!=testLog.entries[logging::NOTICE].end()) {
		if(logEntry->facility=="TEST_FACILITY_2") {
			std::ostringstream oss; oss << "this is a message" << ":" << secondWrites;
			if(logEntry->entry.message == oss.str())
				++secondWrites;
		}
		++logEntry;
	}

	logEntry = testLog.entries[logging::WARNING].begin();
	while(logEntry!=testLog.entries[logging::WARNING].end()) {
		if(logEntry->facility=="TEST_FACILITY_3") {
			std::ostringstream oss; oss << "this is a message" << ":" << thirdWrites;
			if(logEntry->entry.message == oss.str())
				++thirdWrites;
		}
		++logEntry;
	}

	logEntry = testLog.entries[logging::ERROR].begin();
	while(logEntry!=testLog.entries[logging::ERROR].end()) {
		if(logEntry->facility=="TEST_FACILITY_4") {
			std::ostringstream oss; oss << "this is a message" << ":" << fourthWrites;
			if(logEntry->entry.message == oss.str())
				++fourthWrites;
		}
		++logEntry;
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

	logger << logging::EMERGENCY << "this should be in both logs" << logging::end_entry;

	BOOST_REQUIRE(not testLogA.entries[logging::EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLogA.entries[logging::EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK_EQUAL(testLogA.entries[logging::EMERGENCY].front().entry.message, "this should be in both logs");

	BOOST_REQUIRE(not testLogB.entries[logging::EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLogB.entries[logging::EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK_EQUAL(testLogB.entries[logging::EMERGENCY].front().entry.message, "this should be in both logs");
}

BOOST_AUTO_TEST_CASE(write_to_two_logs_from_one_logger_added_at_ctor) {
	TestLog testLogA;
	TestLog testLogB;
	std::vector<TestLog*> logs = {&testLogA, &testLogB};
	Logger logger(logs.begin(), logs.end(), "TESTFACILITY");

	logger << logging::EMERGENCY << "this should be in both logs" << logging::end_entry;

	BOOST_REQUIRE(not testLogA.entries[logging::EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLogA.entries[logging::EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK_EQUAL(testLogA.entries[logging::EMERGENCY].front().entry.message, "this should be in both logs");

	BOOST_REQUIRE(not testLogB.entries[logging::EMERGENCY].empty());
	BOOST_CHECK_EQUAL(testLogB.entries[logging::EMERGENCY].front().facility, "TESTFACILITY");
	BOOST_CHECK_EQUAL(testLogB.entries[logging::EMERGENCY].front().entry.message, "this should be in both logs");
}

BOOST_AUTO_TEST_SUITE_END()
