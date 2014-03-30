/*
 * test_task_batch.cpp
 *
 *  Created on: Mar 29, 2014
 *      Author: Georgios Dimitriadis
 *
 * Copyright (c) 2014, Georgios Dimitriadis
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

#include "task_control/task_batch.h"
#include "task_control/tests/testing_task_function.h"
#include <gtest/gtest.h>

using namespace task_control;
using namespace std;

TEST(test_task_batch, i_gave_you_a_simple_task) {
	task_batch<int> batch;
	testing_task_function task_function;
	int myNumber{0};
	mutex mut;
	condition_variable cond;
	task_function.repetitive_task = [&]() {
		lock_guard<mutex> lock{mut};
		if(++myNumber > 10) cond.notify_one();
	};

	batch.add(named_task<int>{"my_simple_task", ref(task_function)});
	batch.start(chrono::milliseconds{100});
	{
		unique_lock<mutex> lock{mut};
		while(myNumber<=10) cond.wait_for(lock, chrono::seconds{5}, [&](){return myNumber>10;});
	}
	batch.stop(chrono::milliseconds{100});
	myNumber = 0;
	batch.start(chrono::milliseconds{100});
	{
		unique_lock<mutex> lock{mut};
		while(myNumber<=10) cond.wait_for(lock, chrono::seconds{5}, [&](){return myNumber>10;});
	}
	batch.stop(chrono::milliseconds{100});

	ASSERT_LE(10, myNumber);
}

TEST(test_task_batch, i_gave_you_two_simple_tasks) {
	task_batch<int> batch;

	testing_task_function task_function_1;
	int myNumber1{0};
	mutex mut1;
	condition_variable cond1;
	task_function_1.repetitive_task = [&]() {
		lock_guard<mutex> lock{mut1};
		if(++myNumber1 > 10) cond1.notify_one();
	};
	batch.add(named_task<int>{"my_simple_task_1", ref(task_function_1)});

	testing_task_function task_function_2;
	int myNumber2{0};
	mutex mut2;
	condition_variable cond2;
	task_function_2.repetitive_task = [&]() {
		lock_guard<mutex> lock{mut2};
		if(++myNumber2 > 10) cond2.notify_one();
	};
	batch.add(named_task<int>{"my_simple_task_2", ref(task_function_2)});

	batch.start(chrono::milliseconds{100});
	{
		unique_lock<mutex> lock{mut1};
		while(myNumber1<=10) cond1.wait_for(lock, chrono::seconds{5}, [&](){return myNumber1>10;});
	}
	{
		unique_lock<mutex> lock{mut2};
		while(myNumber2<=10) cond2.wait_for(lock, chrono::seconds{5}, [&](){return myNumber2>10;});
	}
	batch.stop(chrono::milliseconds{100});

	ASSERT_LE(10, myNumber1);
	ASSERT_LE(10, myNumber2);
}

TEST(test_task_batch, fail_to_start_a_single_task) {
	task_batch<int> batch;
	testing_task_function task_function;
	vector<string> failures;
	task_function.succeed_start = false;
	batch.add(named_task<int>{"my_simple_task", ref(task_function)});
	auto failure_reporter = [&](const named_task<int>& task) { failures.push_back(task.name()); };
	batch.start(chrono::milliseconds{10}, failure_reporter);

	ASSERT_EQ(1, failures.size());
	ASSERT_EQ("my_simple_task", failures[0]);
}

TEST(test_task_batch, fail_to_start_one_out_of_two) {
	task_batch<int> batch;

	testing_task_function task_function_1;
	task_function_1.succeed_start = false;

	testing_task_function task_function_2;
	task_function_2.succeed_start = true;

	batch.add(named_task<int>{"my_simple_task_1", ref(task_function_1)});
	batch.add(named_task<int>{"my_simple_task_2", ref(task_function_2)});
	vector<string> failures;
	auto failure_reporter = [&](const named_task<int>& task) { failures.push_back(task.name()); };
	batch.start(chrono::milliseconds{10}, failure_reporter);

	ASSERT_EQ(1, failures.size());
	ASSERT_EQ("my_simple_task_1", failures[0]);
}

TEST(test_task_batch, one_out_of_two_fails_inspection) {
	task_batch<int> batch;

	testing_task_function task_function_1;
	task_function_1.succeed_start = true;
	task_function_1.run_for = chrono::microseconds{0}; // make it stop immediately

	testing_task_function task_function_2;
	task_function_2.succeed_start = true;

	batch.add(named_task<int>{"my_simple_task_1", ref(task_function_1)});
	batch.add(named_task<int>{"my_simple_task_2", ref(task_function_2)});
	vector<string> failures;
	auto failure_reporter = [&](const named_task<int>& task) { failures.push_back(task.name()); };
	batch.start(chrono::milliseconds{10}, failure_reporter);

	ASSERT_EQ(0, failures.size()); // check that all report a good start

	this_thread::sleep_for(chrono::milliseconds{1});
	batch.inspect(chrono::milliseconds{1}, failure_reporter);
	ASSERT_EQ(1, failures.size()); // check that all report a good start
	ASSERT_EQ("my_simple_task_1", failures[0]);
}
