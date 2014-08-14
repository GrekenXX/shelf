/*
 * test_named_task.cpp
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

#include "task_control/named_task.h"
#include "task_control/tests/testing_task_function.h"
#include <gtest/gtest.h>
#include <mutex>
#include <condition_variable>
#include <iostream>

using namespace std;
using namespace task_control;

struct test_named_task : ::testing::Test {
	test_named_task() :
		task_function{},
		init_callback_wait{50000},
		stop_wait{50000},
		task{"my_named_task", ref(task_function)} {
	}

	testing_task_function task_function;

	chrono::microseconds init_callback_wait;
	chrono::microseconds stop_wait;

	named_task task;
};

TEST_F(test_named_task, construct) {
	ASSERT_EQ("my_named_task", task.name());
}

TEST_F(test_named_task, start_successful) {
	task_function.succeed_start = true;
	auto init_result = task.start();
	ASSERT_EQ(future_status::ready, init_result.wait_for(init_callback_wait));
	ASSERT_TRUE(init_result.get());
}

TEST_F(test_named_task, start_failure) {
	task_function.succeed_start = false;
	auto init_result = task.start();
	ASSERT_EQ(future_status::ready, init_result.wait_for(init_callback_wait));
	ASSERT_FALSE(init_result.get());
}

TEST_F(test_named_task, start_timeout_no_run) {
	task_function.timeout_start = true;
	task_function.succeed_start = false;
	auto init_result = task.start();
	ASSERT_EQ(future_status::timeout , init_result.wait_for(init_callback_wait));
}

TEST_F(test_named_task, start_timeout_with_run) {
	task_function.timeout_start = true;
	task_function.succeed_start = true;
	auto init_result = task.start();
	ASSERT_EQ(future_status::timeout , init_result.wait_for(init_callback_wait));
}

TEST_F(test_named_task, exception_mid_run) {
	task_function.lap_callback = [] (int lap) {
		if (lap == 3)
			throw "WTF";
	};
	task_function.min_laps = 4;
	auto init_result = task.start();
	ASSERT_EQ(future_status::ready , init_result.wait_for(init_callback_wait));
	ASSERT_EQ(true, init_result.get());

	auto task_result = task.stop();
	ASSERT_EQ(future_status::ready, task_result.wait_for(stop_wait));
	ASSERT_ANY_THROW(task_result.get());
}

TEST_F(test_named_task, stop_successful) {
	task_function.succeed_start = true;
	task_function.timeout_stop = false;
	auto init_result = task.start();
	ASSERT_TRUE(init_result.get());

	auto task_result = task.stop();
	ASSERT_EQ(future_status::ready, task_result.wait_for(stop_wait));
	ASSERT_NO_THROW(task_result.get());
}

TEST_F(test_named_task, get_thrown_exception) {
	task_function.succeed_start = true;
	task_function.timeout_stop = false;
	task_function.throw_on_exit = true;
	auto init_result = task.start();
	ASSERT_TRUE(init_result.get());

	auto task_result = task.stop();
	ASSERT_EQ(future_status::ready, task_result.wait_for(stop_wait));
	ASSERT_ANY_THROW(task_result.get());
}

TEST_F(test_named_task, stop_timeout) {
	task_function.succeed_start = true;
	task_function.timeout_stop = true;
	auto init_result = task.start();
	ASSERT_TRUE(init_result.get());

	auto task_result = task.stop();
	ASSERT_EQ(future_status::timeout, task_result.wait_for(stop_wait));

	task_function.timeout_stop = false;
}

TEST_F(test_named_task, still_running_true) {
	task_function.succeed_start = true;
	task_function.timeout_stop = false;
	auto init_result = task.start();
	ASSERT_TRUE(init_result.get());

	ASSERT_TRUE(task.still_running(chrono::duration_cast<chrono::milliseconds>(stop_wait)));

	auto task_result = task.stop();
	ASSERT_EQ(future_status::ready, task_result.wait_for(stop_wait));
}

TEST_F(test_named_task, still_running_false) {
	task_function.succeed_start = true;
	task_function.timeout_stop = false;
	task_function.run_for = stop_wait/2;
	auto init_result = task.start();
	ASSERT_TRUE(init_result.get());

	this_thread::sleep_for(2*task_function.run_for);

	ASSERT_FALSE(task.still_running(chrono::duration_cast<chrono::milliseconds>(stop_wait)));
}

TEST_F(test_named_task, stop_and_restart) {
	task_function.succeed_start = true;
	task_function.timeout_stop = false;
	task_function.run_for = std::chrono::seconds{10};
	task_function.min_laps = 3;
	auto init_result_1 = task.start();
	ASSERT_TRUE(init_result_1.get());

	auto task_result = task.stop();
	ASSERT_EQ(future_status::ready, task_result.wait_for(stop_wait));

	auto init_result_2 = task.start();
	ASSERT_TRUE(init_result_2.get());

	task_result = task.stop();
	ASSERT_EQ(future_status::ready, task_result.wait_for(stop_wait));
}

