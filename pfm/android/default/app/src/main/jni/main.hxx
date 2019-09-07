#pragma once

#include "pfm.hxx"


#define PACKAGE_NAME "com/indigoabstract/appplex"
#define CLASS_MAIN_PATH "com/indigoabstract/appplex/main"


class android_main : public pfm_main
{
public:
	virtual ~android_main();
	static mws_sp<android_main> get_instance();
	static mws_sp<pfm_impl::pfm_file_impl> new_pfm_file_impl(const std::string& i_filename, const std::string& i_root_dir);
    virtual key_types translate_key(int i_pfm_key_id) const override;
    virtual key_types apply_key_modifiers_impl(key_types i_key_id) const override;
    virtual float get_screen_brightness() const override;
    virtual void set_screen_brightness(float i_brightness) override;
    // screen metrix
    virtual std::pair<uint32, uint32> get_screen_res_px() const override;
    virtual float get_avg_screen_dpi() const override;
    virtual std::pair<float, float> get_screen_dpi() const override;
    virtual std::pair<float, float> get_screen_dim_inch() const override;
    virtual float get_avg_screen_dpcm() const override;
    virtual std::pair<float, float> get_screen_dpcm() const override;
    virtual std::pair<float, float> get_screen_dim_cm() const override;
	virtual void write_text(const char* i_text)const;
	virtual void write_text_nl(const char* i_text)const;
	virtual void write_text(const wchar_t* i_text)const;
	virtual void write_text_nl(const wchar_t* i_text)const;
	virtual void write_text_v(const char* i_format, ...)const;
	virtual std::string get_writable_path() const;
    virtual std::string get_timezone_id() const;
	umf_list get_directory_listing(const std::string& i_directory, umf_list i_plist, bool i_is_recursive);
    virtual float get_screen_scale() const;
	virtual bool is_full_screen_mode();
	virtual void set_full_screen_mode(bool i_enabled);
	void init();
	void start();
	void run();

	void snd_init(int i_sample_rate, int i_buffer_size);
	void snd_close();

	void init_screen_metrix(uint32 i_screen_width, uint32 i_screen_height, float i_screen_horizontal_dpi, float i_screen_vertical_dpi);
    void on_resize(uint32 i_screen_width, uint32 i_screen_height);

	int sample_rate;
	int buffer_size;

private:
	android_main();
	void load_apk_file_list();

	std::vector<mws_sp<pfm_file> > apk_file_list;
	umf_list plist;
    // screen metrix
    std::pair<uint32, uint32> screen_res;
    float avg_screen_dpi = 0.f;
    std::pair<float, float> screen_dpi;
    std::pair<float, float> screen_dim_inch;
    float avg_screen_dpcm = 0.f;
    std::pair<float, float> screen_dpcm;
    std::pair<float, float> screen_dim_cm;
    static mws_sp<android_main> instance;
};
