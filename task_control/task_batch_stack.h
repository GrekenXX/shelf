/*
 * task_batch_stack.h
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

#ifndef TASK_BATCH_STACK_H_
#define TASK_BATCH_STACK_H_
#include "task_control/task_batch.h"
#include <queue>

namespace task_control {

class task_batch_stack {
public:

	void push_top(task_batch&& batch);
	int bring_up_to_level(unsigned max_level);
	int bring_down_to_level(unsigned min_level);
	int set_level(unsigned level);
	int inspect(const std::chrono::milliseconds& max_wait);

private:
	std::deque<task_batch> dormant_batches_;
	std::deque<task_batch> running_batches_;
};

} /* namespace task_control */

#endif /* TASK_BATCH_STACK_H_ */
