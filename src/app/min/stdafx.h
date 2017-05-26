#pragma once

#if defined WIN32

#define _UNICODE
#define UNICODE
#define NOMINMAX
#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN 1
#endif

#include <boost/any.hpp>
#include <boost/config.hpp>
#include <boost/cstdint.hpp>
#include <boost/chrono.hpp>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/preprocessor.hpp>
#include <boost/process.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <boost/type_traits.hpp>
#include <boost/unordered_map.hpp>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <locale>
#include <memory>
#include <string>
#include <vector>

#endif
