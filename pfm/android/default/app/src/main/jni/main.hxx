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
    virtual float get_screen_brightness() const override;
    virtual void set_screen_brightness(float i_brightness) override;
    virtual float get_screen_dpi()const override;
	virtual void write_text(const char* i_text)const;
	virtual void write_text_nl(const char* i_text)const;
	virtual void write_text(const wchar_t* i_text)const;
	virtual void write_text_nl(const wchar_t* i_text)const;
	virtual void write_text_v(const char* i_format, ...)const;
	virtual std::string get_writable_path()const;
    virtual std::string get_timezone_id()const;
	umf_list get_directory_listing(const std::string& i_directory, umf_list i_plist, bool i_is_recursive);
    virtual float get_screen_scale() const;
	virtual bool is_full_screen_mode();
	virtual void set_full_screen_mode(bool i_enabled);
	void init();
	void start();
	void run();

	void snd_init(int i_sample_rate, int i_buffer_size);
	void snd_close();

	int sample_rate;
	int buffer_size;

private:
	android_main();
	void load_apk_file_list();

	std::vector<mws_sp<pfm_file> > apk_file_list;
	umf_list plist;
	bool is_started;
	static mws_sp<android_main> instance;
};
