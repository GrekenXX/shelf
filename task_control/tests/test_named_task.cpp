/*
 * test_named_task.cpp
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
		init_callback_wait{1000},
		stop_wait{1000},
		task{"my_named_task", ref(task_function)} {
	}

	testing_task_function task_function;

	chrono::microseconds init_callback_wait;
	chrono::microseconds stop_wait;

	named_task<int> task;
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

TEST_F(test_named_task, stop_successful) {
	task_function.succeed_start = true;
	task_function.timeout_stop = false;
	auto init_result = task.start();
	ASSERT_TRUE(init_result.get());

	auto task_result = task.stop();
	ASSERT_EQ(future_status::ready, task_result.wait_for(stop_wait));
	ASSERT_EQ(task_function.return_value, task_result.get());
}

TEST_F(test_named_task, stop_timeout) {
	task_function.succeed_start = true;
	task_function.timeout_stop = true;
	auto init_result = task.start();
	ASSERT_TRUE(init_result.get());

	auto task_result = task.stop();
	ASSERT_EQ(future_status::timeout, task_result.wait_for(stop_wait));
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
	task_function.run_for = std::chrono::seconds{100};
	auto init_result = task.start();
	ASSERT_TRUE(init_result.get());

	auto task_result = task.stop();
	ASSERT_EQ(future_status::ready, task_result.wait_for(stop_wait));

	init_result = task.start();
	ASSERT_TRUE(init_result.get());

	task_result = task.stop();
	ASSERT_EQ(future_status::ready, task_result.wait_for(stop_wait));
}
