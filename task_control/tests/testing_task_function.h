/*
 * testing_task_function.h
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

#ifndef TESTING_TASK_FUNCTION_H_
#define TESTING_TASK_FUNCTION_H_
#include <task_control/named_task.h>
#include <chrono>
#include <memory>

struct testing_task_function {
	testing_task_function() :
		timeout_start{false},
		succeed_start{true},
		timeout_stop{false},
		return_value{0},
		run_for{50000},
		repetitive_task{[]{}},
		pre_lock{[]{}},
		post_lock{[]{}},
		force_stop_{false},
		loop_mutex_{} {
	}

	~testing_task_function() {
		timeout_stop = false;
		force_stop_ = true;
		loop_mutex_.lock();
		loop_mutex_.unlock();
	}

	int operator () (task_control::named_task<int>::init_callback_t cb, const bool& stop) {
		pre_lock();
		std::lock_guard<std::mutex> lock(loop_mutex_);
		post_lock();
		if(!timeout_start)
			cb(succeed_start);
		if(succeed_start) {
			auto exit_time = std::chrono::system_clock::now() + run_for;
			while((std::chrono::system_clock::now()<exit_time && !stop) || timeout_stop) {
				if(force_stop_)
					break;
				repetitive_task();
				std::this_thread::sleep_for(std::chrono::microseconds{5});
			}
		}
		return return_value;
	}

	bool timeout_start;
	bool succeed_start;
	bool timeout_stop;
	int return_value;
	std::chrono::microseconds run_for;
	std::function<void()> repetitive_task;
	std::function<void()> pre_lock;
	std::function<void()> post_lock;
private:
	bool force_stop_;
	std::mutex loop_mutex_;
};

#endif /* TESTING_TASK_FUNCTION_H_ */
