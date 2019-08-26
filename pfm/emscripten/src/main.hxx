#pragma once

#include "pfm.hxx"


class emst_main : public pfm_main
{
public:
   virtual ~emst_main();
   static mws_sp<emst_main> get_instance();
   static mws_sp<pfm_impl::pfm_file_impl> new_pfm_file_impl(const std::string& ifilename, const std::string& iroot_dir);
   virtual void init() override;
   virtual void start() override;
   virtual void run() override;
   virtual key_types translate_key(int i_pfm_key_id) const override;
   virtual key_types apply_key_modifiers_impl(key_types i_key_id) const override;
   virtual float get_screen_dpi()const override;
   virtual void write_text(const char* text)const override;
   virtual void write_text_nl(const char* text)const override;
   virtual void write_text(const wchar_t* text)const override;
   virtual void write_text_nl(const wchar_t* text)const override;
   virtual void write_text_v(const char* iformat, ...)const override;
   virtual std::string get_writable_path()const override;
   virtual std::string get_timezone_id()const override;
   umf_list get_directory_listing(const std::string& idirectory, umf_list iplist, bool is_recursive);
   virtual bool is_full_screen_mode();
   virtual void set_full_screen_mode(bool ienabled);

private:
   emst_main();
   void setup_callbacks();

   umf_list plist;
   bool is_started = false;
   static mws_sp<emst_main> instance;
};
