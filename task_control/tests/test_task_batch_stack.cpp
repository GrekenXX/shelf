/*
 * test_task_batch_stack.cpp
 *
 *  Created on: Mar 30, 2014
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

#include "task_control/task_batch_stack.h"
#include "task_control/tests/testing_task_function.h"
#include <gtest/gtest.h>

using namespace task_control;
using namespace std;

TEST(test_task_batch_stack, single_task_in_single_batch) {
	testing_task_function task_function;
	task_function.run_for = chrono::milliseconds{50};

	task_batch<int> batch;
	batch.add(unique_ptr<named_task<int>>{new named_task<int>{"my_simple_task", ref(task_function)}});

	task_batch_stack<int> stack;
	stack.push_top(move(batch));

	ASSERT_EQ(0,stack.inspect(chrono::milliseconds{10}));

	ASSERT_EQ(1,stack.set_level(1));
	ASSERT_EQ(1,stack.inspect(chrono::milliseconds{10}));

	ASSERT_EQ(0,stack.set_level(0));
	ASSERT_EQ(0,stack.inspect(chrono::milliseconds{10}));

	ASSERT_EQ(1,stack.set_level(1));
	ASSERT_EQ(1,stack.inspect(chrono::milliseconds{10}));

	ASSERT_EQ(0,stack.set_level(0));
	ASSERT_EQ(0,stack.inspect(chrono::milliseconds{10}));
}

TEST(test_task_batch_stack, two_tasks_in_single_batch) {
	task_batch<int> batch;

	testing_task_function task_function_1;
	task_function_1.run_for = chrono::milliseconds{50};
	batch.add(unique_ptr<named_task<int>>{new named_task<int>{"my_simple_task_1", ref(task_function_1)}});

	testing_task_function task_function_2;
	task_function_2.run_for = chrono::milliseconds{50};
	batch.add(unique_ptr<named_task<int>>{new named_task<int>{"my_simple_task_2", ref(task_function_2)}});

	task_batch_stack<int> stack;
	stack.push_top(move(batch));

	ASSERT_EQ(0,stack.inspect(chrono::milliseconds{10}));

	ASSERT_EQ(1,stack.set_level(1));
	ASSERT_EQ(1,stack.inspect(chrono::milliseconds{10}));

	ASSERT_EQ(0,stack.set_level(0));
	ASSERT_EQ(0,stack.inspect(chrono::milliseconds{10}));

	ASSERT_EQ(1,stack.set_level(1));
	ASSERT_EQ(1,stack.inspect(chrono::milliseconds{10}));

	ASSERT_EQ(0,stack.set_level(0));
	ASSERT_EQ(0,stack.inspect(chrono::milliseconds{10}));
}

TEST(test_task_batch_stack, two_tasks_in_two_batches) {
	testing_task_function task_function_1;
	task_function_1.run_for = chrono::milliseconds{100000};
	task_batch<int> batch_1;
	batch_1.add(unique_ptr<named_task<int>>{new named_task<int>{"my_simple_task_1", ref(task_function_1)}});

	testing_task_function task_function_2;
	task_function_2.run_for = chrono::milliseconds{100000};
	task_batch<int> batch_2;
	batch_2.add(unique_ptr<named_task<int>>{new named_task<int>{"my_simple_task_2", ref(task_function_2)}});

	task_batch_stack<int> stack;
	stack.push_top(move(batch_1));
	stack.push_top(move(batch_2));

	ASSERT_EQ(0,stack.inspect(chrono::milliseconds{10}));

	ASSERT_EQ(1,stack.set_level(1));
	ASSERT_EQ(1,stack.inspect(chrono::milliseconds{10}));

	ASSERT_EQ(0,stack.set_level(0));
	ASSERT_EQ(0,stack.inspect(chrono::milliseconds{10}));

	ASSERT_EQ(2,stack.set_level(2));
	ASSERT_EQ(2,stack.inspect(chrono::milliseconds{10}));

	ASSERT_EQ(0,stack.set_level(0));
	ASSERT_EQ(0,stack.inspect(chrono::milliseconds{10}));
}

TEST(test_task_batch_stack, six_tasks_in_three_batches) {
	vector<testing_task_function> task_functions{6};
	for(auto& task_function : task_functions)
		task_function.run_for = chrono::milliseconds{100000};

	vector<task_batch<int>> batches{3};
	for(int i=0; i<3; ++i) {
		batches[i].add(unique_ptr<named_task<int>>{new named_task<int>{"my_simple_task_" + to_string(2*i), ref(task_functions[2*i])}});
		batches[i].add(unique_ptr<named_task<int>>{new named_task<int>{"my_simple_task_" + to_string(2*i+1), ref(task_functions[2*i+1])}});
	}

	task_batch_stack<int> stack;
	stack.push_top(move(batches[0]));
	stack.push_top(move(batches[1]));
	stack.push_top(move(batches[2]));

	ASSERT_EQ(0,stack.inspect(chrono::milliseconds{10}));

	ASSERT_EQ(1,stack.set_level(1));
	ASSERT_EQ(1,stack.inspect(chrono::milliseconds{10}));

	ASSERT_EQ(0,stack.set_level(0));
	ASSERT_EQ(0,stack.inspect(chrono::milliseconds{10}));

	ASSERT_EQ(2,stack.set_level(2));
	ASSERT_EQ(2,stack.inspect(chrono::milliseconds{10}));

	ASSERT_EQ(0,stack.set_level(0));
	ASSERT_EQ(0,stack.inspect(chrono::milliseconds{10}));

	ASSERT_EQ(3,stack.set_level(3));
	ASSERT_EQ(3,stack.inspect(chrono::milliseconds{10}));

	ASSERT_EQ(0,stack.set_level(0));
	ASSERT_EQ(0,stack.inspect(chrono::milliseconds{10}));
}

