/*
 * task_batch.h
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
 
#ifndef TASK_BATCH_H_
#define TASK_BATCH_H_
#include "task_control/named_task.h"
#include <chrono>
#include <vector>
#include <list>
#include <string>
#include <map>

namespace task_control {

template<typename T>
class task_batch {
public:
	typedef std::unique_ptr<named_task<T>> task_ptr_t;
	typedef std::function<void(const named_task<T>&)> failure_reporter_t;

	task_batch() { };
	task_batch(task_batch&& other) : tasks_{std::move(other.tasks_)} { }
	task_batch(const task_batch&) = delete;
	task_batch& operator = (const task_batch&) = delete;


	void add(task_ptr_t&& task_func);
	void start(const std::chrono::milliseconds& max_wait, failure_reporter_t report=nullptr);
	void stop(const std::chrono::milliseconds& max_wait, failure_reporter_t report=nullptr);
	void inspect(const std::chrono::milliseconds& max_wait, failure_reporter_t report) const;

private:
	std::list<task_ptr_t> tasks_;
};

template<typename T>
void task_batch<T>::add(task_ptr_t&& task_func) {
	tasks_.emplace_back(std::move(task_func));
}

template<typename T>
void task_batch<T>::start(const std::chrono::milliseconds& max_wait, failure_reporter_t report) {
	std::map<named_task<T>*, std::future<bool>> spawned_tasks;
	for(auto& task : tasks_)
		spawned_tasks[task.get()] = task->start();

	auto deadline = std::chrono::system_clock::now() + max_wait;
	for(auto& spawned_task : spawned_tasks) {
		if ((spawned_task.second.wait_until(deadline) != std::future_status::ready) || !spawned_task.second.get()) {
			if(report)
				report(*spawned_task.first);
		}
	}
}

template<typename T>
void task_batch<T>::stop(const std::chrono::milliseconds& max_wait, failure_reporter_t report) {
	std::map<named_task<T>*, std::future<int>> stopped_tasks;
	for(auto& task : tasks_)
		stopped_tasks[task.get()] = task->stop();

	auto deadline = std::chrono::system_clock::now() + max_wait;
	for(auto& stopped_task : stopped_tasks) {
		if(stopped_task.second.wait_until(deadline) != std::future_status::ready) {
			if(report)
				report(*stopped_task.first);
		}
	}
}

template<typename T>
void task_batch<T>::inspect(const std::chrono::milliseconds& max_wait, failure_reporter_t report) const {
	for(const auto& task : tasks_) {
		if(!task->still_running(max_wait))
			report(*task);
	}
}

} /* namespace task_control */

#endif /* TASK_BATCH_H_ */
