/*
 * test_task_batch_stack.cpp
 *
 *  Created on: Mar 30, 2014
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

#include "task_control/task_batch_stack.h"
#include "task_control/tests/testing_task_function.h"
#include <gtest/gtest.h>

using namespace task_control;
using namespace std;

TEST(test_task_batch_stack, single_task_in_single_batch) {
	testing_task_function task_function;
	task_function.run_for = chrono::milliseconds{50};

	task_batch batch;
	batch.add(unique_ptr<named_task>{new named_task{"my_simple_task", ref(task_function)}});

	task_batch_stack stack;
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
	task_batch batch;

	testing_task_function task_function_1;
	task_function_1.run_for = chrono::milliseconds{50};
	batch.add(unique_ptr<named_task>{new named_task{"my_simple_task_1", ref(task_function_1)}});

	testing_task_function task_function_2;
	task_function_2.run_for = chrono::milliseconds{50};
	batch.add(unique_ptr<named_task>{new named_task{"my_simple_task_2", ref(task_function_2)}});

	task_batch_stack stack;
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
	task_batch batch_1;
	batch_1.add(unique_ptr<named_task>{new named_task{"my_simple_task_1", ref(task_function_1)}});

	testing_task_function task_function_2;
	task_function_2.run_for = chrono::milliseconds{100000};
	task_batch batch_2;
	batch_2.add(unique_ptr<named_task>{new named_task{"my_simple_task_2", ref(task_function_2)}});

	task_batch_stack stack;
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

	vector<task_batch> batches{3};
	for(int i=0; i<3; ++i) {
		batches[i].add(unique_ptr<named_task>{new named_task{"my_simple_task_" + to_string(2*i), ref(task_functions[2*i])}});
		batches[i].add(unique_ptr<named_task>{new named_task{"my_simple_task_" + to_string(2*i+1), ref(task_functions[2*i+1])}});
	}

	task_batch_stack stack;
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

