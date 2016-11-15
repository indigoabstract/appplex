#pragma once

#include "pfm.hpp"
#include <boost/program_options.hpp>


#if defined UNICODE_USING_STD_STRING

	#define unicodevalue					boost::program_options::value
	#define unicode_command_line_parser		boost::program_options::command_line_parser
	#define unicode_parsed_options			boost::program_options::parsed_options

#elif defined UNICODE_USING_STD_WSTRING

	#define unicodevalue					boost::program_options::wvalue
	#define unicode_command_line_parser		boost::program_options::wcommand_line_parser
	#define unicode_parsed_options			boost::program_options::wparsed_options

#endif
