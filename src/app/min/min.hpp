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

using std::shared_ptr;
using std::weak_ptr;

class ia_sender;
class ia_receiver;


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
   template<typename T2, typename T1, class unary_operation> std::vector<T2> static map(const std::vector<T1>& original, unary_operation mapping_function);
   static std::string escape_char(char character);
   static std::string escape_string(const std::string& str);
   static std::vector<std::string> escape_strings(const std::vector<std::string>& delimiters);
   static std::string str_join(const std::vector<std::string>& tokens, const std::string& delimiter);
   static std::vector<std::string> str_split(const std::string& str, const std::vector<std::string>& delimiters);
   static std::vector<std::string> str_split(const std::string& str, const std::string& delimiter);
};


class mws_util
{
public:
   struct path
   {
      static std::string get_directory_from_path(const std::string& file_path);
      static std::string get_filename_from_path(const std::string& file_path);
      static std::string get_filename_without_extension(const std::string& file_path);
   };

   struct time
   {
      static std::string get_current_date();
   };
};


class ia_exception
#ifdef MWS_USES_EXCEPTIONS
   : public std::exception
#endif
{
public:
   ia_exception();
   ia_exception(const std::string& msg);
   ia_exception(const char* msg);
   virtual ~ia_exception();

   // returns a C-style character string describing the general cause of the current error
   virtual const char* what() const;

private:
   std::string exmsg;
};


class iadp
{
public:
   virtual ~iadp() {}
   static shared_ptr<iadp> new_instance(std::string iname);

   virtual const std::string& get_name();
   virtual bool is_type(const std::string& itype);
   virtual bool is_processed();
   virtual void process();
   virtual shared_ptr<ia_sender> source();
   virtual shared_ptr<ia_receiver> destination();

protected:
   iadp(const std::string& iname);
   virtual void set_name(const std::string&);

private:
   friend class ia_sender;

   std::string name;
   bool processed;
   weak_ptr<ia_sender> src;
   weak_ptr<ia_receiver> dst;
};


class ia_receiver
{
public:
   ia_receiver() {}
   virtual ~ia_receiver() {}

   virtual void receive(shared_ptr<iadp> idp) = 0;
};


class ia_sender
{
public:
   ia_sender() {}
   virtual ~ia_sender() {}

   virtual void send(shared_ptr<ia_receiver> dst, shared_ptr<iadp> idp);

protected:
   virtual shared_ptr<ia_sender> sender_inst() = 0;
};


class ia_broadcaster : public ia_sender
{
public:
   ia_broadcaster() {}
   virtual ~ia_broadcaster() {}

   virtual void add_receiver(shared_ptr<ia_receiver> ir);
   virtual void remove_receiver(shared_ptr<ia_receiver> ir);

protected:
   virtual void broadcast(shared_ptr<ia_sender> src, shared_ptr<iadp> idp);

   std::vector<weak_ptr<ia_receiver> > receivers;
};


class ia_node : public ia_sender, public ia_receiver
{
public:
   ia_node() {}
   virtual ~ia_node() {}
};


class mws_bad_any_cast
#ifdef MWS_USES_EXCEPTIONS
   : public std::bad_cast
#endif
{
public:
   virtual const char* what() const
   {
      return "ia_bad_any_cast: failed conversion using ia_any_cast";
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


template <class T, class TAl> inline T* begin_ptr(shared_ptr<std::vector<T, TAl> > v) { return v->empty() ? 0 : &v->front(); }
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

bool ends_with(const std::string& istr, const std::string& ifind);

// trim from start
inline std::string ltrim(const std::string& is)
{
   std::string s(is);
   s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
   return s;
}

// trim from end
inline std::string rtrim(const std::string& is)
{
   std::string s(is);
   s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
   return s;
}

// trim from both ends
inline std::string trim(const std::string& is)
{
   return ltrim(rtrim(is));
}

std::string replace_string(std::string subject, const std::string& search, const std::string& replace);
template<typename T2, typename T1, class unary_operation> std::vector<T2> map(const std::vector<T1>& original, unary_operation mapping_function);
std::string escape_char(char character);
std::string escape_string(const std::string& str);
std::vector<std::string> escape_strings(const std::vector<std::string>& delimiters);
std::string str_join(const std::vector<std::string>& tokens, const std::string& delimiter);
std::vector<std::string> str_split(const std::string& str, const std::vector<std::string>& delimiters);
std::vector<std::string> str_split(const std::string& str, const std::string& delimiter);
