#pragma once

#include "pfm.hpp"
#include <boost/filesystem.hpp>
#include <string>


inline const unicodestring& path2string(const boost::filesystem::path& p)
{
#if defined UNICODE_USING_STD_STRING

	return p.string();

#elif defined UNICODE_USING_STD_WSTRING

	return p.wstring();

#endif
}
