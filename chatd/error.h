#ifndef ERROR_H
#define ERROR_H

#include <stdexcept>
#include <string>

#include "types.h"

template <int N>
class error : public std::exception
{
	sptr_cstr m_what;

public:
	explicit error(sptr_cstr what) : m_what(what) {}

	const char* what() const noexcept
	{
		return m_what->c_str();
	}
};

// Исключения для особых ответов
class error200_info : public std::exception
{
	sptr_cstr m_what;

public:
	explicit error200_info(sptr_cstr what);
	const char* what() const noexcept;
};

class error200_error : public std::exception
{
	sptr_cstr m_what;

public:
	explicit error200_error(sptr_cstr what);
	const char* what() const noexcept;
};

class redirect_image : public std::exception
{
	const std::string m_redirect;
	const std::string m_content_type;
public:
	explicit redirect_image(const std::string& arg_redirect, const std::string& content_type);
	const char* what() const noexcept;
	const char* content_type() const noexcept;
};

class redirect_media : public std::exception
{
	const std::string m_redirect;
	const std::string m_content_type;
public:
	explicit redirect_media(const std::string& arg_redirect, const std::string& content_type);
	const char* what() const noexcept;
	const char* content_type() const noexcept;
};

class redirect_empty : public std::exception
{
	const std::string m_redirect;
public:
	explicit redirect_empty(const std::string& arg_redirect);
	const char* what() const noexcept;
};

class error200_personal : public std::exception
{
	sptr_cstr m_what;

public:
	explicit error200_personal(sptr_cstr what);
	const char* what() const noexcept;
};

#endif
