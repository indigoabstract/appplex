#include "pfm.hxx"
#include "mws-impl.hxx"
#include "min.hxx"
#include "mws-mod.hxx"
#include "mws-mod-ctrl.hxx"
#include "input/input-ctrl.hxx"
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <vector>


class ios_main : public mws_app
{
public:
    virtual ~ios_main();
    void init() override;
    void start() override;
    void run() override;

    virtual mws_key_types translate_key(int i_pfm_key_id) const override;
    virtual mws_key_types apply_key_modifiers_impl(mws_key_types i_key_id) const override;
    // screen
    virtual bool is_full_screen_mode() const override;
    virtual void set_full_screen_mode(bool i_enabled) const override;
    virtual float get_screen_scale() const override;
    virtual float get_screen_brightness() const override;
    virtual void set_screen_brightness(float i_brightness) const override;
    // screen metrix
    virtual std::pair<uint32, uint32> get_screen_res_px() const override;
    virtual float get_avg_screen_dpi() const override;
    virtual std::pair<float, float> get_screen_dpi() const override;
    virtual std::pair<float, float> get_screen_dim_inch() const override;
    virtual float get_avg_screen_dpcm() const override;
    virtual std::pair<float, float> get_screen_dpcm() const override;
    virtual std::pair<float, float> get_screen_dim_cm() const override;
    // log
    virtual void write_text(const char* i_text)const override;
    virtual void write_text_nl(const char* i_text)const override;
    virtual void write_text(const wchar_t* i_text)const override;
    virtual void write_text_nl(const wchar_t* i_text)const override;
    virtual void write_text_v(const char* i_format, ...)const override;
    virtual std::string get_writable_path()const;

    // filesystem
    virtual mws_sp<mws_file_impl> new_mws_file_impl(const mws_path& i_path, bool i_is_internal = false) const override;
    virtual const mws_path& prv_dir() const override;
    virtual const mws_path& res_dir() const override;
    virtual const mws_path& tmp_dir() const override;
    virtual void reconfigure_directories(mws_sp<mws_mod> i_crt_mod) override;
    virtual std::string get_timezone_id() const override;

    void init_screen_metrix(uint32 i_screen_width, uint32 i_screen_height, float i_screen_horizontal_dpi, float i_screen_vertical_dpi);
    void on_resize(uint32 i_screen_width, uint32 i_screen_height);
	
    void snd_init(int i_sample_rate, int i_buffer_size);
    void snd_close();

    int sample_rate;
    int buffer_size;
    float screen_scale;

private:
    mws_file_map plist;
    bool is_started = false;
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
};


namespace
{
   std::string global_prv_path;
   std::string global_tmp_path;
   mws_path prv_path;
   mws_path res_path;
   mws_path tmp_path;
   bool prv_path_exists = false;
   bool tmp_path_exists = false;
    mws_sp<ios_main> instance;
}


static mws_sp<ios_main> app_inst()
{
   if (!instance)
   {
      instance = mws_sp<ios_main>(new ios_main());
   }

   return instance;
}

mws_sp<mws_app> mws_app_inst() { return app_inst(); }


mws_sp<std::string> load_res_as_string(std::string i_filename)
{
    auto c_filename = i_filename.c_str();
    NSString* nss_filename = [[NSString alloc] initWithUTF8String:c_filename];
    // get the main bundle for the app
    NSBundle* main_bundle = [NSBundle mainBundle];
    NSString* path = [main_bundle pathForResource:nss_filename ofType:@""];
    mws_sp<std::string> text;
    
    if(path)
    {
        const char *c_path = [path UTF8String];
        FILE* f = fopen(c_path, "rt");
        
        if (f)
        {
            // read file size
            fseek(f, 0, SEEK_END);
            size_t size = ftell(f);
            fseek(f, 0, SEEK_SET);
            
            auto res = std::make_shared<std::vector<uint8> >(size);
            const char* res_bytes = (const char*)begin_ptr(res);
            size_t text_size = fread(begin_ptr(res), 1, size, f);
            
            fclose(f);
            f = nullptr;
            text = std::make_shared<std::string>(res_bytes, text_size);
        }
    }
    
    return text;
}


class ios_file_impl : public mws_file_impl
{
public:
	ios_file_impl(const mws_path& i_path, bool i_is_internal = false) : mws_file_impl(i_path, i_is_internal) {}
	{
	}

	virtual ~ios_file_impl() {}
    
    FILE* get_file_impl() const override
    {
        return file;
    }
    
	virtual uint64 length() override
	{
		uint64 size = 0;

		if (!file)
		{
			open("rb");

			if (file)
			{
                fseek(file, 0, SEEK_END );
                size = ftell(file);
				close();
			}
		}
		else
		{
            long crt_pos = ftell(file);

            fseek(file, 0, SEEK_END );
            size = ftell(file);
            fseek(file, crt_pos, SEEK_SET );
		}

		return size;
	}

	virtual uint64 creation_time() const override
	{
		std::string path = ppath.string();

		return 0;
	}

	virtual uint64 last_write_time() const override
	{
		std::string path = ppath.string();

		return 0;
	}

	virtual bool open_impl(std::string i_open_mode) override
	{
		std::string path = ppath.string();
        std::string path_in_bundle = get_path_in_bundle(path.c_str());
        std::string* path_ptr = nullptr;
        
        // file not found in bundle. try the filesystem
        if(path_in_bundle.empty())
        {
            path_ptr = &path;
        }
        // file found in bundle
        else
        {
            // files in the bundle are read only
            if(i_open_mode[0] != 'w')
            {
                path_ptr = &path_in_bundle;
            }
        }
        
        if(path_ptr)
        {
            file = fopen(path_ptr->c_str(), i_open_mode.c_str());
        }
        
        bool file_opened = (file != nullptr);
        
        if(file_opened)
        {
            mws_println("open_impl [ opening file %s ]", path_ptr->c_str());
        }
        else
        {
            mws_println("open_impl [ cannot open file %s ]", path.c_str());
        }

        if (file_opened && (i_open_mode.find('w') != std::string::npos))
        {
            file_is_writable = true;
        }
        
        return file_opened;
	}

	virtual void close_impl() override
	{
        fclose(file);
		file = nullptr;
	}

    virtual void flush_impl() override
    {
        if(file)
        {
            fflush(file);
        }
        else
        {
            mws_print("error[ file [ %s ] is not open! ]", ppath.string().c_str());
        }
    }

    virtual void seek_impl(uint64 ipos, int iseek_pos) override
	{
        fseek(file, ipos, iseek_pos);
	}

	virtual uint64 tell_impl() override
	{
        return ftell(file);
	}

	virtual int read_impl(uint8* i_buffer, int i_size) override
	{
        return (int)fread(i_buffer, 1, i_size, file);
	}

	virtual int write_impl(const uint8* i_buffer, int i_size) override
	{
        return (int)fwrite(i_buffer, 1, i_size, file);
	}
    
    static std::string get_path_in_bundle(std::string i_filename)
    {
        auto c_filename = i_filename.c_str();
        NSString* nss_filename = [[NSString alloc] initWithUTF8String:c_filename];
        // get the main bundle for the app
        NSBundle* main_bundle = [NSBundle mainBundle];
        NSString* path = [main_bundle pathForResource:nss_filename ofType:@""];
        
        if(path)
        {
            const char *c_path = [path UTF8String];
            std::string str_path(c_path);
            
            return str_path;
        }
        
        return "";
    }
    
    FILE* file = nullptr;
};


ios_main::~ios_main()
{
}

mws_key_types ios_main::translate_key(int i_pfm_key_id) const { return i_pfm_key_id; }
mws_key_types ios_main::apply_key_modifiers_impl(mws_key_types i_key_id) const { return i_key_id; }


bool ios_main::is_full_screen_mode() const
{
	return true;
}

void ios_main::set_full_screen_mode(bool ienabled) const
{
}

float ios_main::get_screen_scale() const
{
	return screen_scale;
}

float ios_main::get_screen_brightness() const
{
    return 1.f;
}

void ios_main::set_screen_brightness(float i_brightness) const 
{}

std::pair<uint32, uint32> ios_main::get_screen_res_px() const { return screen_res; }
float ios_main::get_avg_screen_dpi() const { return avg_screen_dpi; }
std::pair<float, float> ios_main::get_screen_dpi() const { return screen_dpi; }
std::pair<float, float> ios_main::get_screen_dim_inch() const { return screen_dim_inch; }
float ios_main::get_avg_screen_dpcm() const { return avg_screen_dpcm; }
std::pair<float, float> ios_main::get_screen_dpcm() const { return screen_dpcm; }
std::pair<float, float> ios_main::get_screen_dim_cm() const { return screen_dim_cm; }


void ios_main::write_text(const char* text) const
{
	printf("%s", text);
}

void ios_main::write_text_nl(const char* text) const
{
	printf("%s\n", text);
}

void ios_main::write_text(const wchar_t* text) const
{
}

void ios_main::write_text_nl(const wchar_t* text) const
{
}

void ios_main::write_text_v(const char* iformat, ...) const
{
   char dest[1024 * 16];
   va_list arg_ptr;

   va_start(arg_ptr, iformat);
    vsnprintf(dest, 1024 * 16 - 1, iformat, arg_ptr);
   va_end(arg_ptr);
   
   printf("%s", dest);
}

std::string ios_main::get_writable_path() const
{
    NSString* output_path_nss = nil;
    std::string output_path;
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString* documents_directory = [paths objectAtIndex:0];
    const char* output_path_c = [documents_directory UTF8String];
    output_path = output_path_c;

    return output_path;
}

static bool mws_make_directory(const mws_path& i_path)
{
   bool path_exists = false;

   if (!i_path.exists())
   {
      path_exists = i_path.make_dir();
   }
   else
   {
      path_exists = true;
   }

   if (!path_exists)
   {
      mws_println("WARNING[ failed to create path [ %s ]]", i_path.string().c_str());
   }

   return path_exists;
}

mws_sp<mws_file_impl> ios_main::new_mws_file_impl(const mws_path& i_path, bool i_is_internal)
{
	return std::make_shared<ios_file_impl>(i_path, i_is_internal);
}

const mws_path& ios_main::prv_dir() const
{
   if (!prv_path_exists)
   {
      prv_path_exists = make_directory(prv_path);
   }

   return prv_path;
}

const mws_path& ios_main::res_dir() const
{
   return res_path;
}

const mws_path& ios_main::tmp_dir() const
{
   if (!tmp_path_exists)
   {
      tmp_path_exists = make_directory(tmp_path);
   }

   return tmp_path;
}

void ios_main::reconfigure_directories(mws_sp<mws_mod> i_crt_mod)
{
   std::string mod_dir = i_crt_mod->get_name() + "/";

   mws_assert(i_crt_mod != nullptr);
   prv_path = mws_path(get_writable_path());
   res_path = mws_path(get_path_in_bundle(""));
   tmp_path = mws_path(get_writable_path());
   prv_path_exists = false;
   tmp_path_exists = false;
}

std::string ios_main::get_timezone_id()const
{
    NSTimeZone* time_zone = [NSTimeZone localTimeZone];
    NSString* name_nss = [time_zone name];
    const char* name_c = [name_nss UTF8String];
    std::string name(name_c);
    
    return name;
}

void ios_main::init()
{
   // screen metrix
   {
      float horizontal_screen_dpi = 480.f;
      float vertical_screen_dpi = 480.f;

      init_screen_metrix(screen_width, screen_height, horizontal_screen_dpi, vertical_screen_dpi);
   }
   
	mws_mod_ctrl::inst()->pre_init_app();
	mws_mod_ctrl::inst()->set_gfx_available(true);
    auto start_mod = mws_mod_ctrl::inst()->get_app_start_mod();

    if (start_mod)
    {
        auto mod_pref = start_mod->get_preferences();
        mws_log::set_enabled(mod_pref->log_enabled());
    }

	mws_mod_ctrl::inst()->init_app();

	is_started = true;
}

void ios_main::start()
{
	mws_mod_ctrl::inst()->start_app();
}

void ios_main::run()
{
	mws_mod_ctrl::inst()->update();
}

void ios_main::init_screen_metrix(uint32 i_screen_width, uint32 i_screen_height, float i_screen_horizontal_dpi, float i_screen_vertical_dpi)
{
    float horizontal_dim_inch = i_screen_width / i_screen_horizontal_dpi;
    float vertical_dim_inch = i_screen_height / i_screen_vertical_dpi;
    float horizontal_dim_cm = mws_in(horizontal_dim_inch).to_cm().val();
    float vertical_dim_cm = mws_in(vertical_dim_inch).to_cm().val();
    float horizontal_screen_dpcm = mws_cm(i_screen_horizontal_dpi).to_in().val();
    float vertical_screen_dpcm = mws_cm(i_screen_vertical_dpi).to_in().val();

    screen_res = std::make_pair((uint32)i_screen_width, (uint32)i_screen_height);
    screen_dim_inch = std::make_pair(horizontal_dim_inch, vertical_dim_inch);
    screen_dpi = std::make_pair(i_screen_horizontal_dpi, i_screen_vertical_dpi);
    avg_screen_dpi = (screen_dpi.first + screen_dpi.second) * 0.5f;
    screen_dim_cm = std::make_pair(horizontal_dim_cm, vertical_dim_cm);
    screen_dpcm = std::make_pair(horizontal_screen_dpcm, vertical_screen_dpcm);
    avg_screen_dpcm = (screen_dpcm.first + screen_dpcm.second) * 0.5f;
}

void ios_main::on_resize(uint32 i_screen_width, uint32 i_screen_height)
{
    bool is_landscape_0 = (i_screen_width > i_screen_height);
    bool is_landscape_1 = (screen_res.first > screen_res.second);

    if(is_landscape_0 != is_landscape_1)
    {
        std::swap(screen_res.first, screen_res.second);
        std::swap(screen_dpi.first, screen_dpi.second);
        std::swap(screen_dim_inch.first, screen_dim_inch.second);
        std::swap(screen_dpcm.first, screen_dpcm.second);
        std::swap(screen_dim_cm.first, screen_dim_cm.second);
    }
	
	screen_width = i_screen_width;
	screen_height = i_screen_height;

    mws_mod_ctrl::inst()->resize_app(i_screen_width, i_screen_height);
}
