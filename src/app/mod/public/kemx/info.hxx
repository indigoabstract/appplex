#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_KEMX

#include "mod.hxx"

enum basictypes
{
	SINT_8,
	UINT_8,
	SINT_16,
	UINT_16,
	SINT_32,
	UINT_32,
	SINT_64,
	UINT_64,
	REAL_32,
	REAL_64,
	TEXT,
};

class info
{
public:
	info();
	~info();
};

#endif
