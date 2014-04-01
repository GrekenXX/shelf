/*
 * task_batch.h
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
