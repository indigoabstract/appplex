#pragma once

#include "pfm.hxx"


class ios_main : public pfm_main
{
public:
	virtual ~ios_main();
	static mws_sp<ios_main> get_instance();
	static mws_sp<pfm_impl::pfm_file_impl> new_pfm_file_impl(const std::string& i_filename, const std::string& i_root_dir);
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
	virtual void write_text(const char* i_text)const override;
	virtual void write_text_nl(const char* i_text)const override;
	virtual void write_text(const wchar_t* i_text)const override;
	virtual void write_text_nl(const wchar_t* i_text)const override;
	virtual void write_text_v(const char* i_format, ...)const override;
	virtual std::string get_writable_path()const override;
    virtual std::string get_timezone_id()const override;
	umf_list get_directory_listing(const std::string& i_directory, umf_list i_plist, bool i_is_recursive);
	virtual float get_screen_scale() const override;
	virtual bool is_full_screen_mode() const;
	virtual void set_full_screen_mode(bool i_enabled);
	void init() override;
	void start() override;
	void run() override;

	void init_screen_metrix(uint32 i_screen_width, uint32 i_screen_height, float i_screen_horizontal_dpi, float i_screen_vertical_dpi);
    void on_resize(uint32 i_screen_width, uint32 i_screen_height);
	
	void snd_init(int i_sample_rate, int i_buffer_size);
	void snd_close();

	int sample_rate;
	int buffer_size;
	float screen_scale;

private:
	ios_main();
	void load_apk_file_list();

	std::vector<mws_sp<pfm_file> > apk_file_list;
	umf_list plist;
	bool is_started;
    // screen metrix
    std::pair<uint32, uint32> screen_res;
    float avg_screen_dpi = 0.f;
    std::pair<float, float> screen_dpi;
    std::pair<float, float> screen_dim_inch;
    float avg_screen_dpcm = 0.f;
    std::pair<float, float> screen_dpcm;
    std::pair<float, float> screen_dim_cm;
	uint32 screen_width = 0;
	uint32 screen_height = 0;
	static mws_sp<ios_main> instance;
};
