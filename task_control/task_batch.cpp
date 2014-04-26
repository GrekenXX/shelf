/*
 * task_batch.cpp
 *
 *  Created on: Apr 25, 2014
 *      Author: Georgios Dimitriadis
 *
 * Copyright (C) 2014, Georgios Dimitriadis
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
 * 
 */
#include <task_control/task_batch.h>

using namespace task_control;
using namespace std;


void task_batch::add(task_ptr_t&& task_func) {
	tasks_.emplace_back(std::move(task_func));
}

void task_batch::start(const std::chrono::milliseconds& max_wait, failure_reporter_t report) {
	std::map<named_task*, std::future<bool>> spawned_tasks;
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

void task_batch::stop(const std::chrono::milliseconds& max_wait, failure_reporter_t report) {
	std::map<named_task*, std::future<void>> stopped_tasks;
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

void task_batch::inspect(const std::chrono::milliseconds& max_wait, failure_reporter_t report) const {
	for(const auto& task : tasks_) {
		if(!task->still_running(max_wait))
			report(*task);
	}
}
