/*
 * StreamLog.h
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

#ifndef STREAMLOG_H_
#define STREAMLOG_H_
#include "ILog.h"
#include <ostream>

namespace logging {

class StreamLog : public ILog {
public:
	StreamLog(std::ostream& os, Severity maxSeverity=logging::INFO) : ILog(maxSeverity), _os(os) { }

	virtual void flush() { _os.flush(); };

protected:
	virtual void addEntryImpl(const std::string& facility, const LogEntry& entry) {
		typedef std::chrono::seconds secs_t;
		typedef std::chrono::milliseconds millis_t;
		int64_t secs = std::chrono::duration_cast<secs_t>(entry.time.time_since_epoch()).count();
		int64_t millis = std::chrono::duration_cast<millis_t>(entry.time.time_since_epoch()).count();
		_os << "|" << toString(entry.severity) <<
			   "|" << secs << "." << millis <<
			   "|" << facility <<
			   "|" << entry.message <<
			   std::endl;
	}

private:
	std::ostream& _os;
};

}

#endif /* STREAMLOG_H_ */
