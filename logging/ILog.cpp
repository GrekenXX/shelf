/*
 * ILog.cpp
 *
 *  Created on: Oct 19, 2013
 *      Author: Georgios Dimitriadis
 *
 * Copyright (c) 2013, Georgios Dimitriadis
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

#include "ILog.h"
#include <mutex>

using namespace std;
using namespace logging;

Severity ILog::getMaxSeverity() const {
	return _maxSeverity;
}

void ILog::setMaxSeverity(Severity severity) {
	_maxSeverity = severity;
}

void ILog::addEntry(const std::string& facility, const LogEntry& entry) {
	if(entry.severity<=_maxSeverity) {
		lock_guard<std::mutex> lock(_mutex);
		this->addEntryImpl(facility, entry);
	}
}

std::string ILog::toString(Severity severity) {
	switch(severity) {
	case EMERGENCY:		return "EMERGENCY";
	case ALERT:			return "ALERT";
	case CRITICAL:		return "CRITICAL";
	case ERROR:			return "ERROR";
	case WARNING:		return "WARNING";
	case NOTICE:		return "NOTICE";
	case INFO:			return "INFO";
	case DEBUG:			return "DEBUG";
	}

	return "UNKNOWN_LOG_LEVEL";
}
