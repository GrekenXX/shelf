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

TEST(test_task_batch_stack, single_task) {
	testing_task_function task_function;
	int myNumber{0};
	mutex mut;
	condition_variable cond;
	task_function.repetitive_task = [&]() {
		lock_guard<mutex> lock{mut};
		if(++myNumber > 10) cond.notify_one();
	};

	task_batch<int> batch;
	batch.add(named_task<int>{"my_simple_task", ref(task_function)});

	task_batch_stack<int> stack;
	stack.push_top(move(batch));

	ASSERT_EQ(0,stack.inspect(chrono::milliseconds{100}));
	stack.set_level(1);
	ASSERT_EQ(1,stack.inspect(chrono::milliseconds{100}));

	{
		unique_lock<mutex> lock{mut};
		while(myNumber<=10) cond.wait_for(lock, chrono::seconds{5}, [&](){return myNumber>10;});
	}

	stack.set_level(0);
	ASSERT_EQ(0,stack.inspect(chrono::milliseconds{100}));
}
