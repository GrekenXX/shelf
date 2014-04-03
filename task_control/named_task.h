/*
 * named_task.h
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
