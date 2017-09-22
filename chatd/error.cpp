#include "error.h"

error200_info::error200_info(sptr_cstr what) : m_what(what)
{
}

const char* error200_info::what() const noexcept
{
	return m_what->c_str();
}

error200_error::error200_error(sptr_cstr what) : m_what(what)
{
}

const char* error200_error::what() const noexcept
{
	return m_what->c_str();
}

redirect_image::redirect_image(const std::string& arg_redirect, const std::string& content_type) :
	m_redirect(arg_redirect), m_content_type(content_type)
{
}

const char* redirect_image::what() const noexcept
{
	return m_redirect.c_str();
}

const char* redirect_image::content_type() const noexcept
{
	return m_content_type.c_str();
}

redirect_empty::redirect_empty(const std::string& arg_redirect) :
	m_redirect(arg_redirect)
{
}

const char* redirect_empty::what() const noexcept
{
	return m_redirect.c_str();
}

redirect_media::redirect_media(const std::string& arg_redirect, const std::string& content_type) :
	m_redirect(arg_redirect), m_content_type(content_type)
{
}

const char* redirect_media::what() const noexcept
{
	return m_redirect.c_str();
}

const char* redirect_media::content_type() const noexcept
{
	return m_content_type.c_str();
}

error200_personal::error200_personal(sptr_cstr what) : m_what(what)
{
}

const char* error200_personal::what() const noexcept
{
	return m_what->c_str();
}
