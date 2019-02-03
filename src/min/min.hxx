#pragma once

#include "pfm-def.h"
#include <stdio.h>
#include <algorithm> 
#include <cctype>
#include <functional> 
#include <locale>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#ifdef MWS_USES_EXCEPTIONS
#include <exception>
#endif


class mws_sender;
class mws_receiver;


enum dir_types
{
   DIR_LEFT,
   DIR_UP,
   DIR_RIGHT,
   DIR_DOWN,
};


class mws_exception
#ifdef MWS_USES_EXCEPTIONS
   : public std::exception
#endif
{
public:
   mws_exception();
   mws_exception(const std::string& i_msg);
   mws_exception(const char* i_msg);
   virtual ~mws_exception();

   // returns a C-style character string describing the general cause of the current error
   virtual const char* what() const noexcept;

private:
   void set_msg(const char* i_msg);

   std::string msg;
};


template<typename T> std::string mws_from(const T& i_input) { return std::to_string(i_input); }
template<typename T> T mws_to(const std::string& i_input) { mws_throw mws_exception("mws_to<bool> failed"); return T(); }
template<> int32 mws_to(const std::string& i_input);
template<> uint32 mws_to(const std::string& i_input);
template<> int64 mws_to(const std::string& i_input);
template<> uint64 mws_to(const std::string& i_input);
template<> float mws_to(const std::string& i_input);
template<> double mws_to(const std::string& i_input);
template<> bool mws_to(const std::string& i_input);


struct mws_str
{
   static bool starts_with(const std::string& istr, const std::string& ifind);
   static bool ends_with(const std::string& istr, const std::string& ifind);
   // trim from start
   static std::string ltrim(const std::string& is);
   // trim from end
   static std::string rtrim(const std::string& is);
   // trim from both ends
   static std::string trim(const std::string& is);
   static std::string replace_string(std::string subject, const std::string& search, const std::string& replace);
   template<typename T2, typename T1, class unary_operation> std::vector<T2> static map(const std::vector<T1>& original, unary_operation mapping_function)
   {
      std::vector<T2> mapped;
      std::transform(begin(original), end(original), std::back_inserter(mapped), mapping_function);
      return mapped;
   }
   static std::string escape_char(char character);
   static std::string escape_string(const std::string& str);
   static std::vector<std::string> escape_strings(const std::vector<std::string>& delimiters);
   static std::string str_join(const std::vector<std::string>& tokens, const std::string& delimiter);
   static std::vector<std::string> str_split(const std::string& str, const std::vector<std::string>& delimiters);
   static std::vector<std::string> str_split(const std::string& str, const std::string& delimiter);
   // removes all the occurrences of the string from the input. the input sequence is modified in-place.
   template<typename sequence, typename range> void static erase_all(sequence& i_input, const range& i_search)
   {
      std::size_t search_idx = 0;
      std::size_t found_idx = 0;

      while ((found_idx = i_input.find(i_search, search_idx)) != std::string::npos)
      {
         i_input.erase(found_idx, i_search.size());
         search_idx = found_idx;
      }
   }
   // replaces all occurrences of the search string in the input with the format string. the input sequence is modified in-place.
   template<typename sequence, typename range_0, typename range_1> void static replace_all(sequence& i_input, const range_0& i_search, const range_1& i_fmt)
   {
      std::size_t search_idx = 0;
      std::size_t found_idx = 0;

      while ((found_idx = i_input.find(i_search, search_idx)) != std::string::npos)
      {
         i_input.replace(found_idx, i_search.size(), i_fmt);
         search_idx = found_idx + i_fmt.size();
      }
   }
};


class mws_util
{
public:
   struct path
   {
      static std::string get_directory_from_path(const std::string& i_file_path);
      static std::string get_filename_from_path(const std::string& i_file_path);
      static std::string get_filename_without_extension(const std::string& i_file_path);
   };

   struct time
   {
      static std::string get_current_date();
      static std::string get_duration_as_string(uint32 i_duration);
   };
};


class mws_dp
{
public:
   virtual ~mws_dp() {}
   static mws_sp<mws_dp> nwi(std::string i_name);

   virtual const std::string& get_name();
   virtual bool is_type(const std::string& i_type);
   virtual bool is_processed();
   virtual void process();
   virtual mws_sp<mws_sender> source();
   virtual mws_sp<mws_receiver> destination();

protected:
   mws_dp(const std::string& iname);
   virtual void set_name(const std::string& i_name);

private:
   friend class mws_sender;

   std::string name;
   bool processed;
   mws_wp<mws_sender> src;
   mws_wp<mws_receiver> dst;
};


class mws_receiver
{
public:
   mws_receiver() {}
   virtual ~mws_receiver() {}

   virtual void receive(mws_sp<mws_dp> i_dp) = 0;
};


class mws_sender
{
public:
   mws_sender() {}
   virtual ~mws_sender() {}

   virtual void send(mws_sp<mws_receiver> i_dst, mws_sp<mws_dp> i_dp);

protected:
   virtual mws_sp<mws_sender> sender_inst() = 0;
};


class mws_broadcaster : public mws_sender
{
public:
   mws_broadcaster() {}
   virtual ~mws_broadcaster() {}

   virtual void add_receiver(mws_sp<mws_receiver> i_recv);
   virtual void remove_receiver(mws_sp<mws_receiver> i_recv);

protected:
   virtual void broadcast(mws_sp<mws_sender> i_src, mws_sp<mws_dp> i_dp);

   std::vector<mws_wp<mws_receiver> > receivers;
};


class mws_node : public mws_sender, public mws_receiver
{
public:
   mws_node() {}
   virtual ~mws_node() {}
};


class mws_bad_any_cast
#ifdef MWS_USES_EXCEPTIONS
   : public std::bad_cast
#endif
{
public:
   virtual const char* what() const noexcept
   {
      return "ia_bad_any_cast: failed conversion using mws_any_cast";
   }
};


class mws_any
{
public:
   mws_any() = default;
   template <typename T> mws_any(T const& v) : storage_ptr(new storage<T>(v)) {}
   mws_any(mws_any const& other) : storage_ptr(other.storage_ptr ? std::move(other.storage_ptr->clone()) : nullptr) {}

   void swap(mws_any& other) { storage_ptr.swap(other.storage_ptr); }
   friend void swap(mws_any& a, mws_any& b) { a.swap(b); };
   mws_any& operator=(mws_any other) { swap(other); return *this; }
   bool empty() { return storage_ptr == nullptr; }
   void clear() { storage_ptr = nullptr; }

private:
   struct storage_base
   {
      virtual std::unique_ptr<storage_base> clone() = 0;
      virtual ~storage_base() = default;
   };

   template <typename T> struct storage : storage_base
   {
      T value;
      explicit storage(T const& v) : value(v) {}
      std::unique_ptr<storage_base> clone() { return std::unique_ptr<storage_base>(new storage<T>(value)); }
   };

   std::unique_ptr<storage_base> storage_ptr;
   template<typename T> friend T& mws_any_cast(mws_any&);
   template<typename T> friend T const& mws_any_cast(mws_any const&);
};

template <typename T> T& mws_any_cast(mws_any& a)
{
   //if (auto p = mws_dynamic_cast<mws_any::storage<T>*>(a.storage_ptr.get()))
   //{
   //   return p->value;
   //}

   //mws_throw mws_bad_any_cast();
   auto p = (mws_any::storage<T>*)(a.storage_ptr.get());
   return p->value;
}

template <typename T> T const& mws_any_cast(mws_any const& a)
{
   //if (auto p = mws_dynamic_cast<mws_any::storage<T> const*>(a.storage_ptr.get()))
   //{
   //   return p->value;
   //}

   //mws_throw mws_bad_any_cast();
   auto p = (mws_any::storage<T>*)(a.storage_ptr.get());
   return p->value;
}


#define int_vect_pass(name) name, sizeof(name) / sizeof(int)


template <class T, class TAl> inline T* begin_ptr(mws_sp<std::vector<T, TAl> > v) { return v->empty() ? 0 : &v->front(); }
template <class T, class TAl> inline T* begin_ptr(std::vector<T, TAl>* v) { return v->empty() ? 0 : &v->front(); }
template <class T, class TAl> inline T* begin_ptr(std::vector<T, TAl>& v) { return v.empty() ? 0 : &v.front(); }
template <class T, class TAl> inline const T* begin_ptr(const std::vector<T, TAl>& v) { return v.empty() ? 0 : &v.front(); }

template<typename T> T lerp(const T& start, const T& end, float t)
{
   return start * (1.f - t) + end * t;
}

inline bool is_inside_box(float x, float y, float box_x, float box_y, float box_width, float box_height)
{
   return (x >= box_x && x < (box_x + box_width)) && (y >= box_y && y < (box_y + box_height));
}

bool ends_with(const std::string & istr, const std::string & ifind);

// trim from start
inline std::string ltrim(const std::string & is)
{
   std::string s(is);
   s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) {return !std::isspace(c); }));
   return s;
}

// trim from end
inline std::string rtrim(const std::string & is)
{
   std::string s(is);
   s.erase(std::find_if(s.rbegin(), s.rend(), [](int c) {return !std::isspace(c); }).base(), s.end());
   return s;
}

// trim from both ends
inline std::string trim(const std::string & is)
{
   return ltrim(rtrim(is));
}

std::string replace_string(std::string subject, const std::string & search, const std::string & replace);
template<typename T2, typename T1, class unary_operation> std::vector<T2> map(const std::vector<T1> & original, unary_operation mapping_function);
std::string escape_char(char character);
std::string escape_string(const std::string & str);
std::vector<std::string> escape_strings(const std::vector<std::string> & delimiters);
std::string str_join(const std::vector<std::string> & tokens, const std::string & delimiter);
std::vector<std::string> str_split(const std::string & str, const std::vector<std::string> & delimiters);
std::vector<std::string> str_split(const std::string & str, const std::string & delimiter);