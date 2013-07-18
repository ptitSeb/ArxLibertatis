/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ARX_PLATFORM_PROFILER_H
#define ARX_PLATFORM_PROFILER_H

#include "platform/Platform.h"
#include "platform/Thread.h"
#include "platform/Time.h"

#include "io/fs/FilePath.h"
#include "io/fs/FileStream.h"

#include <iomanip>
#include <map>
#include <vector>

class Profiler {

public:
	
	static Profiler& instance() {
		static Profiler instance;
		return instance;
	}

	Profiler() {
		writeIndex = 0;
		canWrite = true;
		memset(profilePoints, 0, sizeof(profilePoints));
	}

	static void init() {
		registerThread("main");
	}

	/*!
	 * Shut down the crash handler.
	 */
	static void shutdown() {
	}

	void reset() {
		writeIndex = 0;
		memset(profilePoints, 0, sizeof(profilePoints));
		recordedVariables.clear();
	}

	static void registerThread(const std::string& threadName) {
		thread_id_type threadId = Thread::getCurrentThreadId();
		ThreadInfo& threadInfo = instance().threadsInfo[threadId];
		threadInfo.threadName = threadName;
		threadInfo.threadId = threadId;
		threadInfo.startTime = Time::getUs();
		threadInfo.endTime = threadInfo.startTime;
	}
	
	static void unregisterThread() {
		thread_id_type threadId = Thread::getCurrentThreadId();
		ThreadInfo& threadInfo = instance().threadsInfo[threadId];
		threadInfo.endTime = Time::getUs();
	}

	static void addProfilePoint(const char* tag, thread_id_type threadId, u64 startTime, u64 endTime) {

		while(!instance().canWrite);

		u32 pos = InterlockedIncrement(&instance().writeIndex) - 1;

		ProfilePoint& point = instance().profilePoints[pos % NB_POINTS];
		point.tag = tag;
		point.threadId = threadId;
		point.startTime = startTime;
		point.endTime = endTime;
	}

	void flush() {

		canWrite = false;

		writeProfileLog();
		writeRecordedVariables();

		reset();

		canWrite = true;
	}

	static void recordVariable(const char* name, float value) {
		instance().recordedVariables[name].push_back(value);
	}

private:
	static const u32 NB_POINTS = 4 * 1024;

	struct ProfilePoint {
		const char*    tag;
		thread_id_type threadId;
		u64            startTime;
		u64            endTime;
	};

	struct ThreadInfo {
		std::string    threadName;
		thread_id_type threadId;
		u64            startTime;
		u64            endTime;
	};

	typedef std::map<std::string, std::vector<float> > VariablesRecord;

private:
	void writeProfileLog() {
		/*
		// libboost_date_time ? NO WAY!
		std::string dateTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
		std::replace(dateTime.begin(), dateTime.end(), ' ', '.');
		std::replace(dateTime.begin(), dateTime.end(), '-', '.');
		std::replace(dateTime.begin(), dateTime.end(), ':', '.');
		std::replace(dateTime.begin(), dateTime.end(), 'T', '.'); // boost date-time separator is 'T'

		std::string filename = "arx-" + dateTime + ".perf";
		*/
		
		std::string filename = "arx-dateTime.perf"; // TODO......
		fs::ofstream out(fs::path(filename), std::ios::binary | std::ios::out);

		u32 numItems;

		// Threads info
		numItems = threadsInfo.size();
		out.write((const char*)&numItems, sizeof(numItems));
		for(std::map<thread_id_type, ThreadInfo>::const_iterator it = threadsInfo.begin(); it != threadsInfo.end(); ++it) {
			const ThreadInfo& threadInfo = it->second;
			numItems = threadInfo.threadName.length();
			out.write((const char*)&numItems, sizeof(numItems));
			out.write(threadInfo.threadName.c_str(), numItems);
			out.write((const char*)&threadInfo.threadId, sizeof(threadInfo.threadId));
			out.write((const char*)&threadInfo.startTime, sizeof(threadInfo.startTime));
			out.write((const char*)&threadInfo.endTime, sizeof(threadInfo.endTime));
		}

		// Profile points
		u32 index = 0;
		numItems = writeIndex;

		if(writeIndex >= NB_POINTS) {
			index = writeIndex;
			numItems = NB_POINTS;
		}

		out.write((const char*)&numItems, sizeof(numItems));
		for(u32 i = 0; i < numItems; ++i, ++index) {
			ProfilePoint& point = profilePoints[index % NB_POINTS];

			u32 len = strlen(point.tag);
			out.write((const char*)&len, sizeof(len));
			out.write((const char*)point.tag, len);
			out.write((const char*)&point.threadId, sizeof(point.threadId));
			out.write((const char*)&point.startTime, sizeof(point.startTime));
			out.write((const char*)&point.endTime, sizeof(point.endTime));
		}

		out.close();
	}

	void writeRecordedVariables() {
		/*
		// libboost_date_time ? NO WAY!
		std::string dateTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
		std::replace(dateTime.begin(), dateTime.end(), ' ', '.');
		std::replace(dateTime.begin(), dateTime.end(), '-', '.');
		std::replace(dateTime.begin(), dateTime.end(), ':', '.');
		std::replace(dateTime.begin(), dateTime.end(), 'T', '.'); // boost date-time separator is 'T'

		std::string filename = "arx-" + dateTime + ".csv";
		*/

		if(recordedVariables.empty())
			return;

		std::string filename = "arx-dateTime.csv";
		fs::ofstream out(fs::path(filename), std::ios::out);

		u32 maxNumEntries = 0;

		// Tell Excel that our separator is ';'
		out << "sep=;" << std::endl;

		// Write the header
		for(VariablesRecord::const_iterator it = recordedVariables.begin(); it != recordedVariables.end(); ++it) {
			out << it->first;
			out << ';';

			if(it->second.size() > maxNumEntries)
				maxNumEntries = it->second.size();
		}

		out << std::endl;

		// Write all entries
		for(u32 i = 0; i < maxNumEntries; i++) {

			for(VariablesRecord::const_iterator it = recordedVariables.begin(); it != recordedVariables.end(); ++it) {
				
				if(it->second.size() < i) {
					continue;
				}

				out << std::fixed << std::setprecision(5) << it->second[i];
				out << ';';
			}
			out << std::endl;
		}
	}

private:
	ProfilePoint profilePoints[NB_POINTS];
	std::map<thread_id_type, ThreadInfo> threadsInfo;

	VariablesRecord recordedVariables;

	volatile u32 writeIndex;
	bool canWrite;
};

class ProfileScope {
public:
	ProfileScope(const char* _tag)
		: tag(_tag)
		, startTime(Time::getUs()) {
		
		arx_assert(_tag != 0 && _tag != "");
	}

	~ProfileScope() {
		Profiler::addProfilePoint(tag, Thread::getCurrentThreadId(), startTime, Time::getUs());
	}

private:
	const char* tag;
	u64 startTime;
};

#define ARX_PROFILE(tag)           ProfileScope profileScope##__LINE__(#tag)

#define ARX_PROFILE_FUNC()         ProfileScope profileScope##__LINE__(__FUNCTION__)

#define ARX_PROFILE_RECORD(name)   Profiler::recordVariable(#name, name)

#endif // ARX_PLATFORM_PROFILE_H
