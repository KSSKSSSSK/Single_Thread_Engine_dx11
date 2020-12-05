#include "KSException.h"

#include <sstream>


KSException::KSException(int line, const char* file) noexcept
	:
	line(line),
	file(file)
{}

const char* KSException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* KSException::GetType() const noexcept
{
	return "Chili Exception";
}

int KSException::GetLine() const noexcept
{
	return line;
}

const std::string& KSException::GetFile() const noexcept
{
	return file;
}

std::string KSException::GetOriginString() const noexcept
{
	std::ostringstream oss;
	oss << "[File] " << file << std::endl
		<< "[Line] " << line;
	return oss.str();
}