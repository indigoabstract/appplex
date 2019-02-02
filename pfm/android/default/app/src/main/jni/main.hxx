#pragma once

#include "pfm.hxx"


#define PACKAGE_NAME "com/indigoabstract/appplex"
#define CLASS_MAIN_PATH "com/indigoabstract/appplex/main"


class android_main : public pfm_main
{
public:
	virtual ~android_main();
	static shared_ptr<android_main> get_instance();
	static shared_ptr<pfm_impl::pfm_file_impl> new_pfm_file_impl(const std::string& ifilename, const std::string& iroot_dir);
    virtual float get_screen_brightness() const override;
    virtual void set_screen_brightness(float i_brightness) override;
    virtual int get_screen_dpi()const override;
	virtual void write_text(const char* text)const;
	virtual void write_text_nl(const char* text)const;
	virtual void write_text(const wchar_t* text)const;
	virtual void write_text_nl(const wchar_t* text)const;
	virtual void write_text_v(const char* iformat, ...)const;
	virtual std::string get_writable_path()const;
    virtual std::string get_timezone_id()const;
	umf_list get_directory_listing(const std::string& idirectory, umf_list iplist, bool is_recursive);
    float get_screen_scale();
	virtual bool is_full_screen_mode();
	virtual void set_full_screen_mode(bool ienabled);
	void init();
	void start();
	void run();

	void snd_init(int isample_rate, int ibuffer_size);
	void snd_close();

	int sample_rate;
	int buffer_size;

private:
	android_main();
	void load_apk_file_list();

	std::vector<shared_ptr<pfm_file> > apk_file_list;
	umf_list plist;
	bool is_started;
	static shared_ptr<android_main> instance;
};
