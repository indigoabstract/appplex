#pragma once

#include "pfm.hxx"


class emst_main : public pfm_main
{
public:
   virtual ~emst_main();
   static mws_sp<emst_main> get_instance();
   static mws_sp<pfm_impl::pfm_file_impl> new_pfm_file_impl(const std::string& ifilename, const std::string& iroot_dir);
   virtual void init() override;
   virtual key_types translate_key(int i_pfm_key_id) const override;
   virtual key_types apply_key_modifiers_impl(key_types i_key_id) const override;
   // screen metrix
   virtual std::pair<uint32, uint32> get_screen_res_px() const override;
   virtual float get_avg_screen_dpi() const override;
   virtual std::pair<float, float> get_screen_dpi() const override;
   virtual std::pair<float, float> get_screen_dim_inch() const override;
   virtual float get_avg_screen_dpcm() const override;
   virtual std::pair<float, float> get_screen_dpcm() const override;
   virtual std::pair<float, float> get_screen_dim_cm() const override;
   virtual void write_text(const char* text)const override;
   virtual void write_text_nl(const char* text)const override;
   virtual void write_text(const wchar_t* text)const override;
   virtual void write_text_nl(const wchar_t* text)const override;
   virtual void write_text_v(const char* iformat, ...)const override;
   virtual const std::string& prv_dir() const override;
   virtual const std::string& res_dir() const override;
   virtual const std::string& tmp_dir() const override;
   virtual std::string get_timezone_id()const override;
   umf_list get_directory_listing(const std::string& idirectory, umf_list iplist, bool is_recursive);
   virtual bool is_full_screen_mode();
   virtual void set_full_screen_mode(bool i_enabled);

   void init_screen_metrix(uint32 i_screen_width, uint32 i_screen_height, float i_screen_horizontal_dpi, float i_screen_vertical_dpi);
   void on_resize(uint32 i_screen_width, uint32 i_screen_height);

private:
   emst_main();
   void setup_callbacks();

   umf_list plist;
   // screen metrix
   std::pair<uint32, uint32> screen_res;
   float avg_screen_dpi = 0.f;
   std::pair<float, float> screen_dpi;
   std::pair<float, float> screen_dim_inch;
   float avg_screen_dpcm = 0.f;
   std::pair<float, float> screen_dpcm;
   std::pair<float, float> screen_dim_cm;
   static mws_sp<emst_main> instance;
};
