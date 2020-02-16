#pragma once

#include "pfm.hxx"


// base class for the platform specific implementation of a mws app
class mws_app
{
public:
   // update
   virtual void init();
   virtual void start();
   virtual void run();

   // input
   // translate a key from the platform encoding into the appplex encoding
   virtual mws_key_types translate_key(int i_pfm_key_id) const = 0;
   // apply any modifiers like shift, ctrl, caps, num lock etc to get the final key value
   mws_key_types apply_key_modifiers(mws_key_types i_key_id) const;
   virtual mws_key_types apply_key_modifiers_impl(mws_key_types i_key_id) const = 0;
   // return true to exit the app
   virtual bool back_evt() const;

   // screen
   virtual bool is_full_screen_mode() const = 0;
   virtual void set_full_screen_mode(bool i_enabled) const = 0;
   virtual float get_screen_scale() const;
   virtual float get_screen_brightness() const;
   virtual void set_screen_brightness(float i_brightness) const;
   // switches between screen width and height. this only works in windowed desktop applications
   virtual void flip_screen() const;

   // screen metrix
   // horizontal and vertical screen resolution in dots(pixels)
   virtual std::pair<uint32, uint32> get_screen_res_px() const = 0;
   // average dots(pixels) per inch
   virtual float get_avg_screen_dpi() const = 0;
   // horizontal and vertical dots(pixels) per inch
   virtual std::pair<float, float> get_screen_dpi() const = 0;
   // horizontal and vertical dimensions in inch
   virtual std::pair<float, float> get_screen_dim_inch() const = 0;
   // average dots(pixels) per cm
   virtual float get_avg_screen_dpcm() const;
   // horizontal and vertical dots(pixels) per cm
   virtual std::pair<float, float> get_screen_dpcm() const = 0;
   // horizontal and vertical dimensions in cm
   virtual std::pair<float, float> get_screen_dim_cm() const = 0;

   // output
   virtual void write_text(const char* i_text) const = 0;
   virtual void write_text_nl(const char* i_text) const = 0;
   virtual void write_text(const wchar_t* i_text) const = 0;
   virtual void write_text_nl(const wchar_t* i_text) const = 0;
   virtual void write_text_v(const char* i_format, ...) const = 0;

   // filesystem
   static const std::string& res_idx_name();
   virtual mws_sp<mws_file_impl> new_mws_file_impl(const mws_path& i_path, bool i_is_internal = false) const = 0;
   virtual mws_file_map list_internal_directory() const;
   virtual std::vector<mws_sp<mws_file>> list_external_directory(const mws_path& i_directory, bool i_recursive) const;
   // writable/private/persistent files directory for the current mod
   virtual const mws_path& prv_dir() const = 0;
   // read-only/resource files directory for the current mod
   virtual const mws_path& res_dir() const = 0;
   // temporary files directory for the current mod
   virtual const mws_path& tmp_dir() const = 0;
   // reconfigures the above directories path to match the current mod
   virtual void reconfigure_directories(mws_sp<mws_mod> i_crt_mod) = 0;
   virtual std::string get_timezone_id() const = 0;
};


class mws_res_index
{
public:
   static mws_file_map read_file_map(mws_sp<mws_file> i_index_file);
   static void write_file_map(mws_sp<mws_file> i_index_file, const mws_file_map& i_file_map);
};


class mws_file_impl
{
public:
   mws_file_impl(const mws_path& i_path, bool i_is_internal = false);
   virtual ~mws_file_impl();
   virtual FILE* get_file_impl() const = 0;
   virtual bool exists();
   virtual bool is_open() const;
   virtual bool is_writable() const;
   virtual bool is_internal() const;
   virtual uint64 length() = 0;
   virtual uint64 creation_time() const = 0;
   virtual uint64 last_write_time() const = 0;
   virtual bool open(std::string i_open_mode);
   virtual void close();
   virtual void flush();
   virtual bool reached_eof() const;
   virtual void seek(uint64 i_pos);
   virtual int read(std::vector<uint8>& i_buffer);
   virtual int write(const std::vector<uint8>& i_buffer);
   virtual int read(uint8* i_buffer, int i_size);
   virtual int write(const uint8* i_buffer, int i_size);
   virtual void check_state() const;

   mws_path ppath;
   uint64 file_pos = 0;
   bool file_is_open = false;
   bool file_is_writable = false;

protected:
   bool file_is_internal = false;
   std::string open_mode;

   virtual bool open_impl(std::string i_open_mode) = 0;
   virtual void close_impl() = 0;
   virtual void flush_impl() = 0;
   virtual void seek_impl(uint64 i_pos, int i_seek_pos);
   virtual uint64 tell_impl();
   virtual int read_impl(uint8* i_buffer, int i_size);
   virtual int write_impl(const uint8* i_buffer, int i_size);
   virtual void check_file_is_writable();
};
