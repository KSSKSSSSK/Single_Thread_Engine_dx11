#pragma once
#include <exception>
#include <string>

class KSException : public std::exception
{
private:
	int line;
	std::string file;
protected:
	mutable std::string whatBuffer;
public:
	const char* what() const noexcept override;
	virtual const char* GetType() const noexcept;
	int GetLine() const noexcept;
	const std::string& GetFile() const noexcept;
	std::string GetOriginString() const noexcept;
public:
	KSException(int line, const char* file) noexcept;
};