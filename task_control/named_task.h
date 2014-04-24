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

#define TASK_FUNC_SIG(RetType) RetType(task_control::named_task<RetType>::init_callback_t,const bool&)

namespace task_control {

template<typename T>
class named_task {
public:
	typedef std::function<void(bool)> init_callback_t;
	typedef std::function<TASK_FUNC_SIG(T)> task_function_t;

	named_task(const std::string& name, task_function_t task);
	named_task() = delete;
	named_task(const named_task&) = delete;
	named_task(named_task&& original) = delete;
	named_task& operator = (const named_task&) = delete;

	~named_task();

	const std::string& name() const;
	std::future<bool> start();
	std::future<T> stop();
	bool still_running(const std::chrono::milliseconds& max_wait_time) const;

private:
	std::string name_;
	std::function<TASK_FUNC_SIG(T)> task_function_;
	std::packaged_task<TASK_FUNC_SIG(T)> task_;
	std::future<T> result_;
	std::thread thread_;
	std::promise<bool> init_result_;
	bool shutdown_;
};

template<typename T>
named_task<T>::named_task(const std::string& name, task_function_t task)
	: name_{name},
	  task_function_{task},
	  shutdown_{false} {
}

template<typename T>
named_task<T>::~named_task() {
	if (!shutdown_)
		stop();
	if(thread_.joinable())
		thread_.join();
}

template<typename T>
const std::string& named_task<T>::name() const {
	return name_;
}

template<typename T>
std::future<bool> named_task<T>::start() {
	if (shutdown_ && thread_.joinable())
		thread_.join();

	shutdown_ = false;

	init_result_ = std::promise<bool>{};
	task_ = std::packaged_task<TASK_FUNC_SIG(T)>{std::ref(task_function_)};
	result_ = task_.get_future();
	auto init_callback = [&](bool success) { init_result_.set_value(success); };
	thread_ = std::thread{std::move(task_), init_callback, std::ref(shutdown_)};
	return init_result_.get_future();
}

template<typename T>
std::future<T> named_task<T>::stop() {
	shutdown_ = true;
	return std::move(result_);
}

template<typename T>
bool named_task<T>::still_running(const std::chrono::milliseconds& max_wait_time) const {
	return (result_.wait_for(max_wait_time) == std::future_status::timeout);
}

} /* namespace task_control */

#endif /* NAMED_TASK_H_ */
