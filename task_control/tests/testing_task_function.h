/*
 * testing_task_function.h
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
		throw_on_exit{false},
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

	void operator () (task_control::named_task::init_callback_t cb, const bool& stop) {
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
		if (throw_on_exit)
			throw "WTF";
	}

	bool timeout_start;
	bool succeed_start;
	bool timeout_stop;
	bool throw_on_exit;
	std::chrono::microseconds run_for;
	std::function<void()> repetitive_task;
	std::function<void()> pre_lock;
	std::function<void()> post_lock;
private:
	bool force_stop_;
	std::mutex loop_mutex_;
};

#endif /* TESTING_TASK_FUNCTION_H_ */
