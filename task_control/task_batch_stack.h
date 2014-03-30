/*
 * task_batch_stack.h
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

#ifndef TASK_BATCH_STACK_H_
#define TASK_BATCH_STACK_H_
#include "task_control/task_batch.h"
#include <queue>

namespace task_control {

template<typename T>
class task_batch_stack {
public:

	void push_top(task_batch<T>&& batch);
	int bring_up_to_level(int max_level);
	int bring_down_to_level(int min_level);
	int set_level(int level);
	int inspect(const std::chrono::milliseconds& max_wait);

private:
	std::deque<task_batch<T>> dormant_batches_;
	std::deque<task_batch<T>> running_batches_;
};

template<typename T>
void task_batch_stack<T>::push_top(task_batch<T>&& batch) {
	dormant_batches_.emplace_back(std::move(batch));
}

template<typename T>
int task_batch_stack<T>::bring_up_to_level(int max_level) {
	while (running_batches_.size() < max_level ) {
		auto batch = std::move(dormant_batches_.front());
		dormant_batches_.pop_front();
		std::vector<std::string> failed_inits;
		batch.start(std::chrono::milliseconds{1000}, [&](const named_task<T>& task){ failed_inits.push_back(task.name()); });
		if (failed_inits.size() > 0) {
			dormant_batches_.emplace_front(std::move(batch));
			break;
		} else {
			running_batches_.emplace_front(std::move(batch));
		}
	}

	return running_batches_.size();
}

template<typename T>
int task_batch_stack<T>::bring_down_to_level(int min_level) {
	while (running_batches_.size() > min_level ) {
		auto batch = std::move(running_batches_.front());
		running_batches_.pop_front();
		std::vector<std::string> failed_stops;
		batch.stop(std::chrono::milliseconds{1000}, [&](const named_task<T>& task){ failed_stops.push_back(task.name()); });
		if (failed_stops.size() > 0) {
			running_batches_.emplace_front(std::move(batch));
			break;
		} else {
			dormant_batches_.emplace_back(std::move(batch));
		}
	}

	return running_batches_.size();
}

template<typename T>
int task_batch_stack<T>::set_level(int level) {
	int current_level = running_batches_.size();
	if (current_level < level) {
		return bring_up_to_level(level);
	} else if (current_level > level) {
		return bring_down_to_level(level);
	}
	return level;
}

template<typename T>
int task_batch_stack<T>::inspect(const std::chrono::milliseconds& max_wait) {
	int passed_level{0};
	for (auto& batch : running_batches_) {
		bool running{true};
		batch.inspect(max_wait, [&](const named_task<T>&){ running = false; });
		if (!running)
			return passed_level;
		++passed_level;
	}
	return passed_level;
}

} /* namespace task_control */

#endif /* TASK_BATCH_STACK_H_ */
