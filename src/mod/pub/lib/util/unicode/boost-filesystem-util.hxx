#pragma once

#include "pfm.hxx"
#include <filesystem>
#include <string>


inline unicodestring path2string(const std::filesystem::path& p)
{
#if defined UNICODE_USING_STD_STRING

	return p.string();

#elif defined UNICODE_USING_STD_WSTRING

	return p.wstring();

#endif
}
