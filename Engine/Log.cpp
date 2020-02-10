//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

#include "Stdafx.h"

#include "Log.h"

#include "Filesystem.h"

#include <concurrent_queue.h>
#include <iostream>


using namespace Kodiak;
using namespace std;


namespace
{

class LogHandler
{
public:
	static LogHandler& GetInstance()
	{
		static LogHandler instance;
		return instance;
	}

	bool IsInitialized() const { return m_initialized; }

	void Initialize()
	{
		lock_guard<mutex> lock(m_initializationMutex);

		// Only initialize if we are in uninitialized state
		if (m_initialized)
		{
			return;
		}

		CreateLogFile();

		m_haltLogging = false;

		m_workerLoop = async(launch::async,
			[&]
		{
			while (!m_haltLogging)
			{
				LogMessage message;
				if (m_messageQueue.try_pop(message))
				{
					m_file << message.message;
				}
			}
		}
		);

		m_initialized = true;
	}


	void Shutdown()
	{
		lock_guard<mutex> lock(m_initializationMutex);

		// Only shutdown if we are in initialized state
		if (!m_initialized)
		{
			return;
		}

		m_haltLogging = true;
		m_workerLoop.get();

		m_file.flush();
		m_file.close();

		m_initialized = false;
	}


	void PostLogMessage(const LogMessage& message)
	{
		m_messageQueue.push(message);
	}


private:
	LogHandler()
		: m_initialized(false)
	{}

	void CreateLogFile()
	{
		// Get the log directory path
		Filesystem::GetInstance().EnsureLogDirectory();
		auto logPath = Filesystem::GetInstance().GetLogPath();

		// Build the filename
		SYSTEMTIME dateTime;
		GetLocalTime(&dateTime);

		// Append current date+time and extension to filename
		ostringstream sstream;
		sstream << "Log-";
		sstream << dateTime.wYear << dateTime.wMonth << dateTime.wDay;
		sstream << dateTime.wHour << dateTime.wMinute << dateTime.wSecond;
		sstream << ".txt";

		// Build the full path
		auto fullPath = logPath / sstream.str();

		// Open the file stream
		m_file.open(fullPath.c_str(), ios::out | ios::trunc);
		m_file << fixed;

		// Create a hard link to the non-timestamped file
		auto logFilePath = logPath / "Log.txt";
		filesystem::remove(logFilePath);
		try
		{
			filesystem::create_hard_link(fullPath, logFilePath);
		}
		catch (filesystem::filesystem_error e)
		{
			cerr << e.what() << " " << e.path1().string() << " " << e.path2().string() << endl;
		}
	}

private:
	mutex m_initializationMutex;
	ofstream m_file;
	Concurrency::concurrent_queue<LogMessage> m_messageQueue;
	atomic<bool> m_haltLogging;
	future<void> m_workerLoop;
	atomic<bool> m_initialized;
};

} // anonymous namespace


namespace Kodiak
{

string LogLevelToString(LogLevel level)
{
	switch (level)
	{
	case LogLevel::Error:	return string("  Error");	break;
	case LogLevel::Warning:	return string("Warning");	break;
	case LogLevel::Debug:	return string("  Debug");	break;
	case LogLevel::Notice:	return string(" Notice");	break;
	default:				return string("   Info");	break;
	}
}


void PostLogMessage(const LogMessage& message)
{
	LogHandler::GetInstance().PostLogMessage(message);
}


void InitializeLogging()
{
	LogHandler::GetInstance().Initialize();
}


void ShutdownLogging()
{
	LogHandler::GetInstance().Shutdown();
}

} // namespace Kodiak