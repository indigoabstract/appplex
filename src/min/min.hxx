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
#include <numeric>


class mws_sender;
class mws_receiver;


// returns true is pointer i_w was initialized(it can be either valid or expired)
// returns false otherwise(if it's an empty weak_ptr)
template <class T> bool is_valid_or_expired(const mws_wp<T>& i_w)
{
   return i_w.owner_before(mws_wp<T>{}) || mws_wp<T>{}.owner_before(i_w);
}


inline std::string mws_to_str(const bool& i_input) { return std::to_string(i_input); }
inline std::string mws_to_str(const int32& i_input) { return std::to_string(i_input); }
inline std::string mws_to_str(const uint32& i_input) { return std::to_string(i_input); }
inline std::string mws_to_str(const int64& i_input) { return std::to_string(i_input); }
inline std::string mws_to_str(const uint64& i_input) { return std::to_string(i_input); }
inline std::string mws_to_str(const real32& i_input) { return std::to_string(i_input); }
inline std::string mws_to_str(const real64& i_input) { return std::to_string(i_input); }
inline std::string mws_to_str(const char* i_input) { return i_input; }
template<typename T> T mws_to(const std::string& i_input) { mws_assert(false); return T(); }
template<> int32 mws_to(const std::string& i_input);
template<> uint32 mws_to(const std::string& i_input);
template<> int64 mws_to(const std::string& i_input);
template<> uint64 mws_to(const std::string& i_input);
template<> float mws_to(const std::string& i_input);
template<> double mws_to(const std::string& i_input);
template<> bool mws_to(const std::string& i_input);

template<typename T> bool mws_safe_to(const std::string& i_input, T& i_val)
{
   bool result = true;

   mws_try
   {
      i_val = mws_to<T>(i_input);
   }
      mws_catch(...)
   {
#ifdef MWS_USES_EXCEPTIONS
      result = false;
#endif
   }

   return result;
}


struct mws_str
{
   static int32 cmp_ignore_case(const std::string& i_0, const std::string& i_1);
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
   struct math
   {
      template<typename number> number static gcd(number i_n0, number i_n1)
      {
#if CXX_VERSION >= 17

         return std::gcd(i_n0, i_n1);

#else

         if (i_n1 == 0)
         {
            return i_n0;
         }

         return gcd(i_n1, i_n0 % i_n1);

#endif
      }
   };

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


template <typename T, typename mixer> class mws_val_mixer
{
public:
   struct pos_val
   {
      float pos;
      T val;
   };

   mws_val_mixer() { clear(); }

   uint32 size() const { return pos_val_vect.size(); }

   const pos_val& operator[](uint32 i_idx) const { return pos_val_vect[i_idx]; }

   void clear()
   {
      pos_val_vect.clear();
   }

   void set_edges(T i_start, T i_end)
   {
      pos_val_vect.push_back(pos_val{ 0.f, i_start });
      pos_val_vect.push_back(pos_val{ 1.f, i_end });
   }

   // values in i_pos_val_vect must be in ascending order by pos;
   void set_values(const std::vector<pos_val>& i_pos_val_vect)
   {
      mws_assert(i_pos_val_vect.size() >= 2);
      mws_assert(i_pos_val_vect.front().pos == 0.f && i_pos_val_vect.back().pos == 1.f);
      pos_val_vect = i_pos_val_vect;
   }

   int set_val_at(T i_val, float i_position)
   {
      mws_assert(pos_val_vect.size() >= 2);
      if (i_position < 0.f || i_position > 1.f)
      {
         return -1;
      }

      // find the the closest match that's not less than i_position (can be equal)
      auto it = closest_gte_val(pos_val_vect, i_position);
      int idx = -1;

      if (it != pos_val_vect.end())
      {
         idx = it - pos_val_vect.begin();

         if (it->pos == i_position)
         {
            *it = pos_val{ i_position, i_val };
         }
         else
         {
            pos_val_vect.insert(it, pos_val{ i_position, i_val });
         }
      }

      return idx;
   }

   T get_val_at(float i_position)
   {
      if (i_position <= 0.f)
      {
         return pos_val_vect.front().val;
      }

      if (i_position >= 1.f)
      {
         return pos_val_vect.back().val;
      }

      // find the the closest match that's not less than i_position (can be equal)
      auto lim_sup = closest_gte_val(pos_val_vect, i_position);
      auto lim_inf = lim_sup - 1;
      // switch interval to [0, lim_sup - lim_inf]
      float interval = lim_sup->pos - lim_inf->pos;
      float mixf = (i_position - lim_inf->pos) / interval;

      return mixer()(lim_inf->val, lim_sup->val, mixf);
   }

   bool remove_idx(uint32 i_idx)
   {
      if (i_idx <= 0 || i_idx >= pos_val_vect.size() - 1)
      {
         return false;
      }

      pos_val_vect.erase(pos_val_vect.begin() + i_idx);

      return true;
   }

protected:
   // find the the closest match that's not less than i_position (can be equal)
   typename std::vector<pos_val>::iterator closest_gte_val(std::vector<pos_val>& i_vect, float i_position)
   {
      static auto cmp_positions = [](const pos_val& i_a, const pos_val& i_b) { return i_a.pos < i_b.pos; };
      pos_val pc;
      pc.pos = i_position;
      // i_vect is ordered, so we can do a binary search
      auto it = std::lower_bound(i_vect.begin(), i_vect.end(), pc, cmp_positions);

      return it;
   };

   std::vector<pos_val> pos_val_vect;
};


class mws_dp
{
public:
   virtual ~mws_dp() {}
   static mws_sp<mws_dp> nwi(std::string i_name);

   virtual const std::string& get_name();
   virtual bool is_type(const std::string& i_type);
   virtual bool is_processed();
   virtual void process(mws_sp<mws_receiver> i_dst);
   virtual mws_sp<mws_sender> source();
   virtual mws_sp<mws_receiver> destination();

protected:
   mws_dp(const std::string& i_name);
   virtual void set_name(const std::string& i_name);

private:
   friend class mws_sender;

   mws_wp<mws_sender> src;
   mws_wp<mws_receiver> dst;
   std::string name;
   bool processed = false;
};


class mws_ptr_evt_base : public mws_dp
{
public:
   static mws_sp<mws_ptr_evt_base> nwi();
   virtual ~mws_ptr_evt_base() {}

   static const uint32 max_touch_points = 8;

   enum e_touch_type
   {
      touch_invalid,
      touch_began,
      touch_moved,
      touch_ended,
      touch_cancelled,
      mouse_wheel,
   };

   enum e_pointer_press_type
   {
      e_not_pressed,
      e_touch_pressed,
      e_left_mouse_btn,
      e_middle_mouse_btn,
      e_right_mouse_btn,
   };

   struct touch_point
   {
      uintptr_t identifier = 0;
      float x = 0.f;
      float y = 0.f;
      bool is_changed = false;
   };

   e_touch_type type = touch_invalid;
   e_pointer_press_type press_type = e_touch_pressed;
   uint32 time = 0;
   uint32 touch_count = 0;
   touch_point points[max_touch_points];
   real32 mouse_wheel_delta = 0;

protected:
   mws_ptr_evt_base() : mws_dp("ts-") {}
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

bool ends_with(const std::string& istr, const std::string& ifind);

// trim from start
inline std::string ltrim(const std::string& is)
{
   std::string s(is);
   s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) {return !std::isspace(c); }));
   return s;
}

// trim from end
inline std::string rtrim(const std::string& is)
{
   std::string s(is);
   s.erase(std::find_if(s.rbegin(), s.rend(), [](int c) {return !std::isspace(c); }).base(), s.end());
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
