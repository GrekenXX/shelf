/*
 * named_task.cpp
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
#include <task_control/named_task.h>

using namespace task_control;
using namespace std;

named_task::named_task(const string& name, task_function_t task)
	: name_{name},
	  task_function_{task},
	  shutdown_{false} {
}

named_task::~named_task() {
	if (!shutdown_)
		stop();
	if(thread_.joinable())
		thread_.join();
}

const string& named_task::name() const {
	return name_;
}

future<bool> named_task::start() {
	if (shutdown_ && thread_.joinable())
		thread_.join();

	shutdown_ = false;

	init_result_ = promise<bool>{};
	task_ = packaged_task<void (init_callback_t,const bool&)>{ref(task_function_)};
	result_ = task_.get_future();
	auto init_callback = [&](bool success) { init_result_.set_value(success); };
	thread_ = thread{move(task_), init_callback, ref(shutdown_)};
	return init_result_.get_future();
}

future<void> named_task::stop() {
	shutdown_ = true;
	return move(result_);
}

bool named_task::still_running(const chrono::milliseconds& max_wait_time) const {
	return (result_.wait_for(max_wait_time) == future_status::timeout);
}




