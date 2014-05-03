/*
 * named_task.h
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

#ifndef NAMED_TASK_H_
#define NAMED_TASK_H_

#include <functional>
#include <future>
#include <string>
#include <thread>

#include <iostream>

#define TASK_FUNC_SIG void (typename task_control::named_task::init_callback_t,const bool&)

namespace task_control {

class named_task {
public:
	typedef std::function<void(bool)> init_callback_t;
	typedef std::function<void (init_callback_t,const bool&)> task_function_t;

	named_task(const std::string& name, task_function_t task);
	named_task() = delete;
	named_task(const named_task&) = delete;
	named_task(named_task&& original) = delete;
	named_task& operator = (const named_task&) = delete;

	~named_task();

	const std::string& name() const;
	std::future<bool> start();
	std::future<void> stop();
	bool still_running(const std::chrono::milliseconds& max_wait_time) const;

private:
	std::string name_;
	std::function<void (init_callback_t,const bool&, std::promise<void>&)> task_function_;
	std::promise<void> result_promise_;
	std::future<void> result_future_;
	std::thread thread_;
	std::promise<bool> init_result_;
	bool shutdown_;
};

} /* namespace task_control */

#endif /* NAMED_TASK_H_ */
