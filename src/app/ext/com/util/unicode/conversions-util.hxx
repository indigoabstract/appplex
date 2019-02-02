#pragma once

#include "pfm.hxx"
#include <cstdlib>
#include <cstring>
#include <string>


inline std::wstring string2wstring(const std::string& s)
{
	size_t bufferSize = s.length();
	wchar_t* buffer = new wchar_t[bufferSize];
#pragma warning(suppress : 4996)
   mbstowcs  (buffer, s.c_str(), bufferSize);
	std::wstring tmp(buffer, bufferSize);
	delete[] buffer;

	return tmp; 
}


inline std::string wstring2string(const std::wstring& s)
{
	size_t bufferSize = s.length();
	char* buffer = new char[bufferSize];
#pragma warning(suppress : 4996)
   wcstombs (buffer, s.c_str(), bufferSize);
	std::string tmp(buffer, bufferSize);
	delete[] buffer;

	return tmp;
}


#if defined UNICODE_USING_STD_STRING

	inline const unicodestring& string2unicodestring(const std::string& s)
	{
		return s;
	}

	inline unicodestring wstring2unicodestring(const std::wstring& s)
	{
		return wstring2string(s);
	}

	inline const std::string& unicodestring2string(const unicodestring& s)
	{
		return s;
	}

	inline std::wstring unicodestring2wstring(const unicodestring& s)
	{
		return string2wstring(s);
	}

	inline unicodechar* unicodestrcpy(unicodechar* destination, const unicodechar* source)
	{
		return strcpy(destination, source);
	}

	inline int unicodestrlen(const unicodechar* str)
	{
		return strlen(str);
	}

#elif defined UNICODE_USING_STD_WSTRING

	inline unicodestring string2unicodestring(const std::string& s)
	{
		return string2wstring(s);
	}

	inline const unicodestring& wstring2unicodestring(const std::wstring& s)
	{
		return s;
	}

	inline std::string unicodestring2string(const unicodestring& s)
	{
		return wstring2string(s);
	}

	inline const std::wstring& unicodestring2wstring(const unicodestring& s)
	{
		return s;
	}

	inline unicodechar* unicodestrcpy(unicodechar* destination, const unicodechar* source)
	{
#pragma warning(suppress : 4996)
		return wcscpy(destination, source);
	}

	inline int unicodestrlen(const unicodechar* str)
	{
		return wcslen(str);
	}

#endif
