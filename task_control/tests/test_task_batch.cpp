/*
 * test_task_batch.cpp
 *
 *  Created on: Mar 29, 2014
 *      Author: Georgios Dimitriadis
 *
 * Copyright (C) 2014 Georgios Dimitriadis
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 *   The above copyright notice and this permission notice shall be
 *   included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "task_control/task_batch.h"
#include "task_control/tests/testing_task_function.h"
#include <gtest/gtest.h>

using namespace task_control;
using namespace std;

TEST(test_task_batch, i_gave_you_a_simple_task) {
	task_batch batch;
	testing_task_function task_function;
	int myNumber{0};
	mutex mut;
	condition_variable cond;
	task_function.repetitive_task = [&]() {
		lock_guard<mutex> lock{mut};
		if(++myNumber > 10) cond.notify_one();
	};

	batch.add(unique_ptr<named_task>{new named_task{"my_simple_task", ref(task_function)}});
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
	task_batch batch;

	testing_task_function task_function_1;
	int myNumber1{0};
	mutex mut1;
	condition_variable cond1;
	task_function_1.repetitive_task = [&]() {
		lock_guard<mutex> lock{mut1};
		if(++myNumber1 > 10) cond1.notify_one();
	};
	batch.add(unique_ptr<named_task>{new named_task{"my_simple_task_1", ref(task_function_1)}});

	testing_task_function task_function_2;
	int myNumber2{0};
	mutex mut2;
	condition_variable cond2;
	task_function_2.repetitive_task = [&]() {
		lock_guard<mutex> lock{mut2};
		if(++myNumber2 > 10) cond2.notify_one();
	};
	batch.add(unique_ptr<named_task>{new named_task{"my_simple_task_2", ref(task_function_2)}});

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
	task_batch batch;
	testing_task_function task_function;
	vector<string> failures;
	task_function.succeed_start = false;
	batch.add(unique_ptr<named_task>{new named_task{"my_simple_task", ref(task_function)}});
	auto failure_reporter = [&](const named_task& task) { failures.push_back(task.name()); };
	batch.start(chrono::milliseconds{10}, failure_reporter);

	ASSERT_EQ(1, failures.size());
	ASSERT_EQ("my_simple_task", failures[0]);
}

TEST(test_task_batch, fail_to_start_one_out_of_two) {
	task_batch batch;

	testing_task_function task_function_1;
	task_function_1.succeed_start = false;

	testing_task_function task_function_2;
	task_function_2.succeed_start = true;

	batch.add(unique_ptr<named_task>{new named_task{"my_simple_task_1", ref(task_function_1)}});
	batch.add(unique_ptr<named_task>{new named_task{"my_simple_task_2", ref(task_function_2)}});
	vector<string> failures;
	auto failure_reporter = [&](const named_task& task) { failures.push_back(task.name()); };
	batch.start(chrono::milliseconds{10}, failure_reporter);

	ASSERT_EQ(1, failures.size());
	ASSERT_EQ("my_simple_task_1", failures[0]);
}

TEST(test_task_batch, one_out_of_two_fails_inspection) {
	task_batch batch;

	testing_task_function task_function_1;
	task_function_1.succeed_start = true;
	task_function_1.run_for = chrono::microseconds{0}; // make it stop immediately

	testing_task_function task_function_2;
	task_function_2.succeed_start = true;

	batch.add(unique_ptr<named_task>{new named_task{"my_simple_task_1", ref(task_function_1)}});
	batch.add(unique_ptr<named_task>{new named_task{"my_simple_task_2", ref(task_function_2)}});
	vector<string> failures;
	auto failure_reporter = [&](const named_task& task) { failures.push_back(task.name()); };
	batch.start(chrono::milliseconds{10}, failure_reporter);

	ASSERT_EQ(0, failures.size()); // check that all report a good start

	this_thread::sleep_for(chrono::milliseconds{1});
	batch.inspect(chrono::milliseconds{1}, failure_reporter);
	ASSERT_EQ(1, failures.size()); // check that all report a good start
	ASSERT_EQ("my_simple_task_1", failures[0]);
}
