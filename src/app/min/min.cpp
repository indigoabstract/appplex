#include "stdafx.h"

#include "min.hpp"
#include "unit-ctrl.hpp"
#include <regex>
#include <sstream>
#include <ctime>
#include <iomanip>


using std::string;
using std::wstring;


std::string mws_util::path::get_directory_from_path(const std::string& file_path)
{
   auto pos_0 = file_path.find_last_of('\\');
   auto pos_1 = file_path.find_last_of('/');
   int64 pos = -1;

   if (pos_0 == std::string::npos && pos_1 == std::string::npos)
   {
      pos = -1;
   }
   else if (pos_0 != std::string::npos && pos_1 != std::string::npos)
   {
      pos = (pos_0 > pos_1) ? pos_0 : pos_1;
   }
   else if (pos_0 != std::string::npos)
   {
      pos = pos_0;
   }
   else if (pos_1 != std::string::npos)
   {
      pos = pos_1;
   }

   return std::string(file_path.begin(), file_path.begin() + (size_t)pos);
}

std::string mws_util::path::get_filename_from_path(const std::string & file_path)
{
   auto pos_0 = file_path.find_last_of('\\');
   auto pos_1 = file_path.find_last_of('/');
   int64 pos = -1;

   if (pos_0 == std::string::npos && pos_1 == std::string::npos)
   {
      pos = -1;
   }
   else if (pos_0 != std::string::npos && pos_1 != std::string::npos)
   {
      pos = (pos_0 > pos_1) ? pos_0 : pos_1;
   }
   else if (pos_0 != std::string::npos)
   {
      pos = pos_0;
   }
   else if (pos_1 != std::string::npos)
   {
      pos = pos_1;
   }

   size_t idx = size_t(pos + 1);

   return std::string(file_path.begin() + idx, file_path.end());
}

std::string mws_util::path::get_filename_without_extension(const std::string & file_path)
{
   auto filename = get_filename_from_path(file_path);
   auto last_index = filename.find_last_of('.');

   if (last_index == std::string::npos)
   {
      return filename;
   }

   std::string stem = filename.substr(0, last_index);

   return stem;
}
std::string mws_util::time::get_current_date()
{
   std::time_t t = std::time(nullptr);
   std::tm tm = *std::localtime(&t);
   std::stringstream ss;

   ss << std::put_time(&tm, "%a %b %d %H:%M:%S %Y");

   auto s = ss.str();

   return s;
}

std::string mws_util::time::get_duration_as_string(uint32 i_duration)
{
   std::string duration;

   uint32 millis = i_duration % 1000;
   uint32 seconds = i_duration / 1000;
   uint32 minutes = seconds / 60;
   uint32 seconds_remainder = seconds % 60;

   duration += std::to_string(minutes);
   duration += ":";
   duration += std::to_string(seconds_remainder);
   //duration += ":";
   //duration += std::to_string(millis);

   return duration;
}



bool mws_str::starts_with(const std::string & istr, const std::string & ifind)
{
   int size = istr.length();
   int size_find = ifind.length();

   if (size_find > size)
   {
      return false;
   }

   for (int k = 0; k < size_find; k++)
   {
      if (istr[k] != ifind[k])
      {
         return false;
      }
   }

   return true;
}

bool mws_str::ends_with(const std::string & istr, const std::string & ifind)
{
   int size = istr.length();
   int size_find = ifind.length();

   if (size_find > size)
   {
      return false;
   }

   for (int k = size - size_find, l = 0; k < size; k++, l++)
   {
      if (istr[k] != ifind[l])
      {
         return false;
      }
   }

   return true;
}

std::string mws_str::ltrim(const std::string & is)
{
   std::string s(is);
   s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) {return !std::isspace(c); }));
   return s;
}

std::string mws_str::rtrim(const std::string & is)
{
   std::string s(is);
   s.erase(std::find_if(s.rbegin(), s.rend(), [](int c) {return !std::isspace(c); }).base(), s.end());
   return s;
}

std::string mws_str::trim(const std::string & is)
{
   return ltrim(rtrim(is));
}

std::string mws_str::replace_string(std::string subject, const std::string & search, const std::string & replace)
{
   size_t pos = 0;

   while ((pos = subject.find(search, pos)) != std::string::npos)
   {
      subject.replace(pos, search.length(), replace);
      pos += replace.length();
   }

   return subject;
}

template<typename T2, typename T1, class unary_operation> std::vector<T2> mws_str::map(const std::vector<T1> & original, unary_operation mapping_function)
{
   std::vector<T2> mapped;

   std::transform(begin(original), end(original), std::back_inserter(mapped), mapping_function);

   return mapped;
}

std::string mws_str::escape_char(char character)
{
   static const std::unordered_map<char, std::string> escaped_special_characters =
   {
      { '.', "\\." },{ '|', "\\|" },{ '*', "\\*" },{ '?', "\\?" },
      { '+', "\\+" },{ '(', "\\(" },{ ')', "\\)" },{ '{', "\\{" },
      { '}', "\\}" },{ '[', "\\[" },{ ']', "\\]" },{ '^', "\\^" },
      { '$', "\\$" },{ '\\', "\\\\" }
   };

   auto it = escaped_special_characters.find(character);

   if (it == escaped_special_characters.end())
   {
      return std::string(1, character);
   }

   return it->second;
}

std::string mws_str::escape_string(const std::string & str)
{
   std::stringstream stream;

   std::for_each(begin(str), end(str), [&stream](const char character) { stream << escape_char(character); });

   return stream.str();
}

std::vector<std::string> mws_str::escape_strings(const std::vector<std::string> & delimiters)
{
   return map<std::string>(delimiters, escape_string);
}

std::string mws_str::str_join(const std::vector<std::string> & tokens, const std::string & delimiter)
{
   std::stringstream stream;

   stream << tokens.front();
   std::for_each(begin(tokens) + 1, end(tokens), [&](const std::string & elem) { stream << delimiter << elem; });

   return stream.str();
}

std::vector<std::string> mws_str::str_split(const std::string & str, const std::vector<std::string> & delimiters)
{
   std::regex rgx(str_join(escape_strings(delimiters), "|"));

   std::sregex_token_iterator first{ begin(str), end(str), rgx, -1 }, last;

   return{ first, last };
}

std::vector<std::string> mws_str::str_split(const std::string & str, const std::string & delimiter)
{
   std::vector<std::string> delimiters = { delimiter };

   return str_split(str, delimiters);
}


mws_exception::mws_exception()
{
   set_msg("");
}

mws_exception::mws_exception(const std::string & i_msg)
{
   set_msg(i_msg.c_str());
}

mws_exception::mws_exception(const char* i_msg)
{
   set_msg(i_msg);
}

mws_exception::~mws_exception()
{
}

const char* mws_exception::what() const noexcept
{
   return msg.c_str();
}

void mws_exception::set_msg(const char* i_msg)
{
   msg = i_msg;

#ifndef MWS_USES_EXCEPTIONS
   mws_assert(false);
#endif
}


mws_dp::mws_dp(const std::string & i_name)
{
   set_name(i_name);
   processed = false;
}

mws_sp<mws_dp> mws_dp::nwi(std::string i_name)
{
   return mws_sp<mws_dp>(new mws_dp(i_name));
}

const std::string& mws_dp::get_name()
{
   return name;
}

bool mws_dp::is_type(const std::string & i_type)
{
   return mws_str::starts_with(get_name(), i_type);
}

bool mws_dp::is_processed()
{
   return processed;
}

void mws_dp::process()
{
   if (processed)
   {
      mws_throw mws_exception("datapacket is already processed");
   }

   processed = true;
}

mws_sp<mws_sender> mws_dp::source()
{
   return src.lock();
}

mws_sp<mws_receiver> mws_dp::destination()
{
   return dst.lock();
}

void mws_dp::set_name(const std::string & i_name)
{
   name = i_name;
}


void mws_sender::send(mws_sp<mws_receiver> i_dst, mws_sp<mws_dp> i_dp)
{
   i_dp->src = sender_inst();
   i_dp->dst = i_dst;
   i_dst->receive(i_dp);
}


void mws_broadcaster::add_receiver(mws_sp<mws_receiver> i_recv)
{
   bool exists = false;
   int size = receivers.size();

   for (int k = 0; k < size; k++)
   {
      mws_sp<mws_receiver> sr = receivers[k].lock();

      if (sr == i_recv)
      {
         exists = true;
         break;
      }
   }

   if (!exists)
   {
      receivers.push_back(i_recv);
   }
}

void mws_broadcaster::remove_receiver(mws_sp<mws_receiver> i_recv)
{
   int idx = -1;
   int k = 0;
   int size = receivers.size();

   for (int k = 0; k < size; k++)
   {
      mws_sp<mws_receiver> sr = receivers[k].lock();

      if (sr == i_recv)
      {
         idx = k;
         break;
      }
   }

   if (idx >= 0)
   {
      receivers.erase(receivers.begin() + idx);
   }
}

void mws_broadcaster::broadcast(mws_sp<mws_sender> i_src, mws_sp<mws_dp> i_dp)
{
   int size = receivers.size();

   for (int k = 0; k < size; k++)
   {
      mws_sp<mws_receiver> dst = receivers[k].lock();

      if (dst)
      {
         send(dst, i_dp);
      }
   }
}


bool ends_with(const std::string & istr, const std::string & ifind)
{
   int size = istr.length();
   int size_find = ifind.length();

   if (size_find > size)
   {
      return false;
   }

   for (int k = size - size_find, l = 0; k < size; k++, l++)
   {
      if (istr[k] != ifind[l])
      {
         return false;
      }
   }

   return true;
}


std::string replace_string(std::string subject, const std::string & search, const std::string & replace)
{
   size_t pos = 0;

   while ((pos = subject.find(search, pos)) != std::string::npos)
   {
      subject.replace(pos, search.length(), replace);
      pos += replace.length();
   }

   return subject;
}

template<typename T2, typename T1, class unary_operation> std::vector<T2> map(const std::vector<T1> & original, unary_operation mapping_function)
{
   std::vector<T2> mapped;

   std::transform(begin(original), end(original), std::back_inserter(mapped), mapping_function);

   return mapped;
}

std::string escape_char(char character)
{
   static const std::unordered_map<char, std::string> escaped_special_characters =
   {
      { '.', "\\." },{ '|', "\\|" },{ '*', "\\*" },{ '?', "\\?" },
      { '+', "\\+" },{ '(', "\\(" },{ ')', "\\)" },{ '{', "\\{" },
      { '}', "\\}" },{ '[', "\\[" },{ ']', "\\]" },{ '^', "\\^" },
      { '$', "\\$" },{ '\\', "\\\\" }
   };

   auto it = escaped_special_characters.find(character);

   if (it == escaped_special_characters.end())
   {
      return std::string(1, character);
   }

   return it->second;
}

std::string escape_string(const std::string & str)
{
   std::stringstream stream;

   std::for_each(begin(str), end(str), [&stream](const char character) { stream << escape_char(character); });

   return stream.str();
}

std::vector<std::string> escape_strings(const std::vector<std::string> & delimiters)
{
   return map<std::string>(delimiters, escape_string);
}

std::string str_join(const std::vector<std::string> & tokens, const std::string & delimiter)
{
   std::stringstream stream;

   stream << tokens.front();
   std::for_each(begin(tokens) + 1, end(tokens), [&](const std::string & elem) { stream << delimiter << elem; });

   return stream.str();
}

std::vector<std::string> str_split(const std::string & str, const std::vector<std::string> & delimiters)
{
   std::regex rgx(str_join(escape_strings(delimiters), "|"));

   std::sregex_token_iterator first{ begin(str), end(str), rgx, -1 }, last;

   return{ first, last };
}

std::vector<std::string> str_split(const std::string & str, const std::string & delimiter)
{
   std::vector<std::string> delimiters = { delimiter };

   return str_split(str, delimiters);
}
