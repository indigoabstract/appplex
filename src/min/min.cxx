#include "stdafx.hxx"

#include "min.hxx"
#include "mws-mod-ctrl.hxx"
mws_push_disable_all_warnings
#include <cstring>
#include <regex>
#include <sstream>
mws_pop_disable_all_warnings


template<> int32 mws_to(const std::string& i_input) { return std::stoi(i_input); }
template<> uint32 mws_to(const std::string& i_input) { return std::stoi(i_input); }
template<> int64 mws_to(const std::string& i_input) { return std::stoll(i_input); }
template<> uint64 mws_to(const std::string& i_input) { return std::stoull(i_input); }
template<> float mws_to(const std::string& i_input) { return std::stof(i_input); }
template<> double mws_to(const std::string& i_input) { return std::stod(i_input); }
template<> bool mws_to(const std::string& i_input)
{
   if (i_input == "0" || i_input == "false")
   {
      return false;
   }
   else if (i_input == "1" || i_input == "true")
   {
      return true;
   }

   mws_throw mws_exception("mws_to<bool> failed");
   return false;
}


std::wstring mws_str::to_wstr(const std::string& i_input)
{
   size_t buffer_size = i_input.length();
   wchar_t* buffer = new wchar_t[buffer_size];
#pragma warning(suppress : 4996)
   mbstowcs(buffer, i_input.c_str(), buffer_size);
   std::wstring tmp(buffer, buffer_size);
   delete[] buffer;

   return tmp;
}


std::string mws_str::to_str(const std::wstring& i_input)
{
   size_t buffer_size = i_input.length();
   char* buffer = new char[buffer_size];
#pragma warning(suppress : 4996)
   wcstombs(buffer, i_input.c_str(), buffer_size);
   std::string tmp(buffer, buffer_size);
   delete[] buffer;

   return tmp;
}

#if defined MWS_UNICODE_USING_STD_STRING

const unicode_string& mws_str::string_to_unicode_string(const std::string& i_str)
{
   return i_str;
}

unicode_string mws_str::wstring_to_unicode_string(const std::wstring& i_str)
{
   return mws_str::to_str(i_str);
}

const std::string& mws_str::unicode_string_to_string(const unicode_string& i_str)
{
   return i_str;
}

std::wstring mws_str::unicode_string_to_wstring(const unicode_string& i_str)
{
   return mws_str::to_wstr(i_str);
}

unicode_char* mws_str::unicode_strcpy(unicode_char* i_destination, const unicode_char* i_source)
{
   return strcpy(i_destination, i_source);
}

int mws_str::unicode_strlen(const unicode_char* i_str)
{
   return strlen(i_str);
}

#elif defined MWS_UNICODE_USING_STD_WSTRING

unicode_string mws_str::string_to_unicode_string(const std::string& i_str)
{
   return mws_str::to_wstr(i_str);
}

const unicode_string& mws_str::wstring_to_unicode_string(const std::wstring& i_str)
{
   return i_str;
}

std::string mws_str::unicode_string_to_string(const unicode_string& i_str)
{
   return mws_str::to_str(i_str);
}

const std::wstring& mws_str::unicode_string_to_wstring(const unicode_string& i_str)
{
   return i_str;
}

unicode_char* mws_str::unicode_strcpy(unicode_char* i_destination, const unicode_char* i_source)
{
#pragma warning(suppress : 4996)
   return wcscpy(i_destination, i_source);
}

int mws_str::unicode_strlen(const unicode_char* i_str)
{
   return wcslen(i_str);
}

#endif

void mws_str::to_lower(std::string& i_str)
{
   std::transform(i_str.begin(), i_str.end(), i_str.begin(), ::tolower);
}

void mws_str::to_upper(std::string& i_str)
{
   std::transform(i_str.begin(), i_str.end(), i_str.begin(), ::toupper);
}

int32 mws_str::cmp_ignore_case(const std::string& i_0, const std::string& i_1)
{
   auto comp_ch = [](const char i_0, const char i_1) -> int32
   {
      if (i_0 == i_1)
      {
         return 0;
      }

      return (std::tolower(i_0) - std::tolower(i_1));
   };

   const char* s1 = (const char*)i_0.c_str();
   const char* s2 = (const char*)i_1.c_str();
   char c1, c2;
   int32 cmp_res = 0;

   do
   {
      c1 = (char)*s1++;
      c2 = (char)*s2++;
      cmp_res = comp_ch(c1, c2);

      if (c1 == '\0')
      {
         return cmp_res;
      }

   } while (cmp_res == 0);

   return cmp_res;
}

bool mws_str::starts_with(const std::string& i_str, const std::string& i_match)
{
   int size = i_str.length();
   int size_find = i_match.length();

   if (size_find > size)
   {
      return false;
   }

   for (int k = 0; k < size_find; k++)
   {
      if (i_str[k] != i_match[k])
      {
         return false;
      }
   }

   return true;
}

bool mws_str::ends_with(const std::string& i_str, const std::string& i_match)
{
   int size = i_str.length();
   int size_find = i_match.length();

   if (size_find > size)
   {
      return false;
   }

   for (int k = size - size_find, l = 0; k < size; k++, l++)
   {
      if (i_str[k] != i_match[l])
      {
         return false;
      }
   }

   return true;
}

std::string mws_str::ltrim(const std::string& i_str)
{
   std::string s(i_str);
   s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) {return !std::isspace(c); }));
   return s;
}

std::string mws_str::rtrim(const std::string& i_str)
{
   std::string s(i_str);
   s.erase(std::find_if(s.rbegin(), s.rend(), [](int c) {return !std::isspace(c); }).base(), s.end());
   return s;
}

std::string mws_str::trim(const std::string& i_str)
{
   return ltrim(rtrim(i_str));
}

bool mws_str::is_double_quoted(const std::string& i_str) { return mws_str::starts_with(i_str, "\"") && mws_str::ends_with(i_str, "\""); }

bool mws_str::is_single_quoted(const std::string& i_str) { return mws_str::starts_with(i_str, "'") && mws_str::ends_with(i_str, "'"); }

std::string mws_str::strip_enclosing_quotes(const std::string& i_text)
{
   const char single_quote = '\'';
   const char double_quote = '"';

   if (i_text.length() >= 2)
   {
      if ((i_text.front() == single_quote && i_text.back() == single_quote) || (i_text.front() == double_quote && i_text.back() == double_quote))
      {
         std::string result = i_text.substr(1, i_text.length() - 2);

         return result;
      }
   }

   return i_text;
}

std::string mws_str::replace_string(std::string subject, const std::string& search, const std::string& replace)
{
   size_t pos = 0;

   while ((pos = subject.find(search, pos)) != std::string::npos)
   {
      subject.replace(pos, search.length(), replace);
      pos += replace.length();
   }

   return subject;
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

std::string mws_str::escape_string(const std::string& str)
{
   std::stringstream stream;

   std::for_each(begin(str), end(str), [&stream](const char character) { stream << escape_char(character); });

   return stream.str();
}

std::vector<std::string> mws_str::escape_strings(const std::vector<std::string>& delimiters)
{
   return map<std::string>(delimiters, escape_string);
}

std::string mws_str::str_join(const std::vector<std::string>& tokens, const std::string& delimiter)
{
   if (tokens.empty())
   {
      return std::string();
   }

   std::stringstream stream;

   stream << tokens.front();
   std::for_each(begin(tokens) + 1, end(tokens), [&](const std::string& elem) { stream << delimiter << elem; });

   return stream.str();
}

std::vector<std::string> mws_str::str_split(const std::string& i_str, const std::vector<std::string>& delimiters)
{
   if (i_str.empty())
   {
      return {};
   }

   std::regex rgx(str_join(escape_strings(delimiters), "|"));
   std::sregex_token_iterator first{ begin(i_str), end(i_str), rgx, -1 }, last;

   return{ first, last };
}

std::vector<std::string> mws_str::str_split(const std::string& i_str, const std::string& delimiter)
{
   std::vector<std::string> delimiters = { delimiter };

   return str_split(i_str, delimiters);
}


mws_dp::mws_dp(const std::string& i_name)
{
   set_name(i_name);
}

mws_sp<mws_dp> mws_dp::nwi(std::string i_name)
{
   return mws_sp<mws_dp>(new mws_dp(i_name));
}

const std::string& mws_dp::get_name()
{
   return name;
}

bool mws_dp::is_type(const std::string& i_type)
{
   return mws_str::starts_with(get_name(), i_type);
}

bool mws_dp::is_processed()
{
   return processed;
}

void mws_dp::process(mws_sp<mws_receiver> i_dst)
{
   if (processed)
   {
      mws_throw mws_exception("datapacket is already processed");
      mws_assert(false);
      return;
   }

   dst = i_dst;
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

void mws_dp::set_name(const std::string& i_name)
{
   name = i_name;
}


void mws_sender::send(mws_sp<mws_receiver> i_dst, mws_sp<mws_dp> i_dp)
{
   i_dp->src = sender_inst();
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


bool ends_with(const std::string& i_str, const std::string& i_match)
{
   int size = i_str.length();
   int size_find = i_match.length();

   if (size_find > size)
   {
      return false;
   }

   for (int k = size - size_find, l = 0; k < size; k++, l++)
   {
      if (i_str[k] != i_match[l])
      {
         return false;
      }
   }

   return true;
}


std::string replace_string(std::string subject, const std::string& search, const std::string& replace)
{
   size_t pos = 0;

   while ((pos = subject.find(search, pos)) != std::string::npos)
   {
      subject.replace(pos, search.length(), replace);
      pos += replace.length();
   }

   return subject;
}

template<typename T2, typename T1, class unary_operation> std::vector<T2> map(const std::vector<T1>& original, unary_operation mapping_function)
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

std::string escape_string(const std::string& str)
{
   std::stringstream stream;

   std::for_each(begin(str), end(str), [&stream](const char character) { stream << escape_char(character); });

   return stream.str();
}

std::vector<std::string> escape_strings(const std::vector<std::string>& delimiters)
{
   return map<std::string>(delimiters, escape_string);
}

std::string str_join(const std::vector<std::string>& tokens, const std::string& delimiter)
{
   std::stringstream stream;

   stream << tokens.front();
   std::for_each(begin(tokens) + 1, end(tokens), [&](const std::string& elem) { stream << delimiter << elem; });

   return stream.str();
}

std::vector<std::string> str_split(const std::string& str, const std::vector<std::string>& delimiters)
{
   std::regex rgx(str_join(escape_strings(delimiters), "|"));

   std::sregex_token_iterator first{ begin(str), end(str), rgx, -1 }, last;

   return{ first, last };
}

std::vector<std::string> str_split(const std::string& str, const std::string& delimiter)
{
   std::vector<std::string> delimiters = { delimiter };

   return str_split(str, delimiters);
}
