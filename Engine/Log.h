//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

#pragma once

namespace Kodiak
{

enum class LogLevel
{
	Error,
	Warning,
	Info,
	Debug
};


std::string LogLevelToString(LogLevel level);

void InitializeLogging();
void ShutdownLogging();


struct LogMessage
{
	std::string message;
};


void PostLogMessage(const LogMessage& message);


template <LogLevel TLevel>
class Logger
{
public:
	Logger() {}

	~Logger()
	{
		std::string message(LogLevelToString(TLevel));
		message += ": ";
		message += m_stream.str();
		message += "\n";

		PostLogMessage({ message });
	}

	std::ostringstream& MessageStream() { return m_stream; }

private:
	std::ostringstream m_stream;
};


#define LOG(level) Logger<level>().MessageStream()
#define LOG_ERROR Logger<LogLevel::Error>().MessageStream()
#define LOG_WARNING Logger<LogLevel::Warning>().MessageStream()
#define LOG_INFO Logger<LogLevel::Info>().MessageStream()
#define LOG_DEBUG Logger<LogLevel::Debug>().MessageStream()

} // namespace Kodiak