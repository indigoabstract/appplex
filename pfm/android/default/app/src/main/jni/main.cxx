#include "main.hxx"

#include "pfm-gl.h"
#include "min.hxx"
#include "data-sequence.hxx"
#include "mws-mod-ctrl.hxx"
#include "gfx.hxx"
#include "java-callbacks.h"
#include "jni-helper.hxx"
#include <zip/zip.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <jni.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <vector>


#define APPNAME "appplex"

std::string g_apk_path;
AAssetManager* asset_manager = NULL;
JavaVM* g_pJavaVM = NULL;
jobject g_obj = NULL;
std::string writable_path;


class android_file_impl : public pfm_impl::pfm_file_impl
{
public:
	android_file_impl(const std::string& ifilename, const std::string& iroot_dir) : pfm_impl::pfm_file_impl(ifilename, iroot_dir)
	{
	}

	virtual ~android_file_impl() {}

    FILE* get_file_impl() const override
    {
        return file;
    }

	virtual uint64 length() override
	{
		uint64 size = 0;

        if(asset_file)
        {
            size = AAsset_getLength64(asset_file);
        }
        else if(file)
        {
            long crt_pos = ftell(file);

            fseek(file, 0, SEEK_END);
            size = ftell(file);
            fseek(file, crt_pos, SEEK_SET);
        }
        else
        {
            open_impl("rb");

            if(asset_file)
            {
                size = AAsset_getLength64(asset_file);
                close_impl();
            }
            else if(file)
            {
                fseek(file, 0, SEEK_END);
                size = ftell(file);
                close_impl();
            }
        }

		return size;
	}

	virtual uint64 creation_time() const override
	{
		std::string path = ppath.get_full_path();

		return 0;
	}

	virtual uint64 last_write_time()const override
	{
		std::string path = ppath.get_full_path();

		return 0;
	}

	virtual bool open_impl(std::string iopen_mode) override
	{
		std::string path = ppath.get_full_path();

		if (iopen_mode[0] == 'w' && path[0] != '/')
		{
			return false;
		}

		if(path[0] == '/')
		{
			// external path
			file = fopen(path.c_str(), iopen_mode.c_str());
			mws_print("open_impl: opening external file %s\n", path.c_str());

			return file != nullptr;
		}

		asset_file = AAssetManager_open(asset_manager, path.c_str(), 0);
		mws_print("open_impl: opening asset file %s\n", path.c_str());

		return asset_file != nullptr;
	}

	virtual void close_impl() override
	{
		if(file)
		{
			fclose(file);
            file = nullptr;
		}
		else if(asset_file)
		{
			AAsset_close(asset_file);
            asset_file = nullptr;
		}
        else
        {
            mws_print("error[ file [ %s ] is not open! ]", ppath.get_full_path().c_str());
        }
	}

    virtual void flush_impl() override
    {
        if(file)
        {
            fflush(file);
        }
        else if(asset_file)
        {
            mws_print("error[ cannot flush an asset file! [ %s ] ]", ppath.get_full_path().c_str());
        }
        else
        {
            mws_print("error[ file [ %s ] is not open! ]", ppath.get_full_path().c_str());
        }
    }

	virtual void seek_impl(uint64 ipos, int iseek_pos) override
	{
		if(file)
		{
			fseek(file, ipos, iseek_pos);
		}
		else if(asset_file)
		{
			AAsset_seek64(asset_file, ipos, iseek_pos);
		}
        else
        {
            mws_throw mws_exception("error");
        }
	}

	virtual uint64 tell_impl() override
	{
		if(file)
		{
			return ftell(file);
		}

        mws_throw mws_exception("unsupported op");

		return 0;
	}

	virtual int read_impl(uint8* ibuffer, int isize) override
	{
		if(file)
		{
			return fread(ibuffer, 1, isize, file);
		}
		
		return AAsset_read(asset_file, ibuffer, isize);
	}

	virtual int write_impl(const uint8* ibuffer, int isize) override
	{
		if(file)
		{
			return fwrite(ibuffer, 1, isize, file);
		}
		
		mws_throw mws_exception("unsupported op");

		return 0;
	}

    FILE* file = nullptr;
    AAsset* asset_file = nullptr;
};


mws_sp<android_main> android_main::instance;

android_main::android_main()
{
	plist = std::make_shared<umf_r>();
}

android_main::~android_main()
{
}

mws_sp<android_main> android_main::get_instance()
{
	if (!instance)
	{
		instance = mws_sp<android_main>(new android_main());
	}

	return instance;
}

mws_sp<pfm_impl::pfm_file_impl> android_main::new_pfm_file_impl(const std::string& ifilename, const std::string& iroot_dir)
{
	return std::make_shared<android_file_impl>(ifilename, iroot_dir);
}

key_types android_main::translate_key(int i_pfm_key_id) const
{
    if(i_pfm_key_id > KEY_INVALID && i_pfm_key_id < KEY_COUNT)
    {
        return static_cast<key_types>(i_pfm_key_id);
    }

    return KEY_INVALID;
}

key_types android_main::apply_key_modifiers_impl(key_types i_key_id) const { return i_key_id; }

float android_main::get_screen_brightness() const
{
    JNIEnv* env = JniHelper::getEnv();
    jclass clazz = env->FindClass(CLASS_MAIN_PATH);
    jmethodID mid = env->GetStaticMethodID(clazz, "get_screen_brightness", "()F");
    jfloat brightness = env->CallStaticFloatMethod(clazz, mid);

    return (float)brightness;
}

void android_main::set_screen_brightness(float i_brightness)
{
    JNIEnv* env = JniHelper::getEnv();
    jclass clazz = env->FindClass(CLASS_MAIN_PATH);
    jmethodID mid = env->GetStaticMethodID(clazz, "set_screen_brightness", "(F)V");
    env->CallStaticVoidMethod(clazz, mid, (jfloat)i_brightness);
}

// screen metrix
std::pair<uint32, uint32> android_main::get_screen_res_px() const { return screen_res; }
float android_main::get_avg_screen_dpi() const { return avg_screen_dpi; }
std::pair<float, float> android_main::get_screen_dpi() const { return screen_dpi; }
std::pair<float, float> android_main::get_screen_dim_inch() const { return screen_dim_inch; }
float android_main::get_avg_screen_dpcm() const { return avg_screen_dpcm; }
std::pair<float, float> android_main::get_screen_dpcm() const { return screen_dpcm; }
std::pair<float, float> android_main::get_screen_dim_cm() const { return screen_dim_cm; }

void android_main::write_text(const char* text)const
{
	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, text, 0);
}

void android_main::write_text_nl(const char* text)const
{
	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, text, 0);
}

void android_main::write_text(const wchar_t* text)const
{
	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "wwrite_text not implemented");
}

void android_main::write_text_nl(const wchar_t* text)const
{
	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "wwrite_text_nl not implemented");
}

void android_main::write_text_v(const char* iformat, ...)const
{
	va_list arg_list;

	va_start(arg_list, iformat);
    __android_log_vprint(ANDROID_LOG_VERBOSE, APPNAME, iformat, arg_list);
	va_end(arg_list);
}

std::string android_main::get_writable_path()const
{
	return writable_path;
}

std::string android_main::get_timezone_id()const
{
    JNIEnv* env = JniHelper::getEnv();
    jclass clazz = env->FindClass(CLASS_MAIN_PATH);
    jmethodID mid = env->GetStaticMethodID(clazz, "get_timezone_id", "()Ljava/lang/String;");
    jstring timezone_id = (jstring)env->CallStaticObjectMethod(clazz, mid);
    const char* ttimezone_id = env->GetStringUTFChars(timezone_id, 0);
    std::string ret;

    if (ttimezone_id)
    {
        ret = ttimezone_id;
        env->ReleaseStringUTFChars(timezone_id, ttimezone_id);
    }

    return ret;
}

void get_directory_listing_helper(umf_list iplist, mws_sp<pfm_file> ifile)
{
	if (iplist->find(ifile->get_file_name()) != iplist->end())
	{
		mws_print("android_main::get_directory_listing. duplicate filename: %s", ifile->get_full_path().c_str());
        mws_throw mws_exception("duplicate filename: " + ifile->get_full_path());
	}

	(*iplist)[ifile->get_file_name()] = ifile;
}

umf_list android_main::get_directory_listing(const std::string& idirectory, umf_list iplist, bool is_recursive)
{
	if (!idirectory.empty())
	{
		if (is_recursive)
		{
			for (auto& e : apk_file_list)
			{
				if (mws_str::starts_with(e->get_root_directory(), idirectory))
				{
					get_directory_listing_helper(iplist, e);
				}
			}
		}
		else
		{
			for (auto& e : apk_file_list)
			{
				if (idirectory == e->get_root_directory())
				{
					get_directory_listing_helper(iplist, e);
				}
			}
		}
	}

	return iplist;
}

float android_main::get_screen_scale() const
{
    return 1.f;
}

bool android_main::is_full_screen_mode()
{
	return true;
}

void android_main::set_full_screen_mode(bool ienabled)
{
}

void android_main::load_apk_file_list()
{
	umf_r& list = *plist;
	std::string assets_pfx = "assets/";
	zip* apk_archive = zip_open(g_apk_path.c_str(), 0, nullptr);
	int file_count = zip_get_num_files(apk_archive);
	//mws_print("android_main::load_apk_file_list zip %s %d", g_apk_path.c_str(), file_count);

	for (int i = 0; i < file_count; i++)
	{
		const char* name = zip_get_name(apk_archive, i, 0);

		if (name == nullptr)
		{
			mws_print("Error reading zip file name at index %i : %s", i, zip_strerror(apk_archive));

			return;
		}
		else
		{
			std::string full_path(name);

			if (mws_str::starts_with(full_path, assets_pfx))
			{
				std::string path = full_path.substr(assets_pfx.length(), std::string::npos);
				std::size_t dir_delim_idx = path.find_last_of('/');
				std::string dir;
				std::string file;

				if (dir_delim_idx != std::string::npos)
				{
					dir = path.substr(0, dir_delim_idx + 1);
					file = path.substr(dir_delim_idx + 1, std::string::npos);
				}
				else
				{
					file = path;
				}

				auto file_impl = std::make_shared<android_file_impl>(file, dir);
				auto p_file = pfm_file::get_inst(file_impl);

				apk_file_list.push_back(p_file);
				//mws_print("file %i : %s\n", i, p_file->get_full_path().c_str());
			}
		}
	}

	//mws_print("android_main::load_apk_file_list 2");
	zip_close(apk_archive);
}

void android_main::init()
{
	load_apk_file_list();

	mws_mod_ctrl::inst()->pre_init_app();
	mws_mod_ctrl::inst()->set_gfx_available(true);
	mws_mod_ctrl::inst()->init_app();
}

void android_main::start()
{
	mws_mod_ctrl::inst()->start_app();
}

void android_main::run()
{
	mws_mod_ctrl::inst()->update();
}

void android_main::init_screen_metrix(uint32 i_screen_width, uint32 i_screen_height, float i_screen_horizontal_dpi, float i_screen_vertical_dpi)
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

void android_main::on_resize(uint32 i_screen_width, uint32 i_screen_height)
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

    mws_mod_ctrl::inst()->resize_app(i_screen_width, i_screen_height);
}


extern "C"
{
	JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
	{
		JniHelper::setJavaVM(vm);
		g_pJavaVM = vm;

		return JNI_VERSION_1_4;
	}

	JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved)
	{
		JNIEnv* env = NULL;

		env->DeleteGlobalRef(g_obj);
		g_obj = NULL;
		g_pJavaVM = NULL;
	}


	void initJavaMethodsIDs(JNIEnv*  env, jobject obj)
	{
		g_obj = env->NewGlobalRef(obj);
	}

	void JavaCallback_DestroyApp(int nAppCleanClose)
	{
		JavaCallMethod_void("appDestroy", "(Z)V", CppBool_To_JniBool(nAppCleanClose));
	}

	bool JavaCallback_CreateFolder(char* path, char* name)
	{
		return JavaCallStaticMethodByClass_bool("com/namco/namcoworks/NUSDKManager",
			"JCreateFolder",
			"(Ljava/lang/String;Ljava/lang/String;)Z",
			CppString_to_JniString(path),
			CppString_to_JniString(name));
	}

	void exit_application()
	{
        JNIEnv* env = JniHelper::getEnv();
        jclass clazz = env->FindClass(CLASS_MAIN_PATH);
        jmethodID mid = env->GetStaticMethodID(clazz, "exit_application", "()V");
        env->CallStaticVoidMethod(clazz, mid);
	}

	JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1init_1renderer
	(JNIEnv* env, jobject obj, jobject i_asset_manager, jstring i_apk_path,
	        jint i_screen_width, jint i_screen_height, jfloat i_screen_horizontal_dpi, jfloat i_screen_vertical_dpi)
	{
		initJavaMethodsIDs(env, obj);
        // screen metrix
        mws_assert(i_screen_width > 0 && i_screen_height > 0);
        android_main::get_instance()->init_screen_metrix(i_screen_width, i_screen_height, i_screen_horizontal_dpi, i_screen_vertical_dpi);

		const char* apk_path = env->GetStringUTFChars(i_apk_path, 0);

		if (apk_path)
		{
			g_apk_path = apk_path;
			env->ReleaseStringUTFChars(i_apk_path, apk_path);
		}

		asset_manager = AAssetManager_fromJava(env, i_asset_manager);

        if(asset_manager)
        {
            android_main::get_instance()->init();
            mws_print("asset manager loaded");
        }
        else
        {
            mws_print("error loading asset manager");
        }
	}

	JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1start_1app(JNIEnv*  env, jobject thiz)
	{
		android_main::get_instance()->start();
	}

	JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1destroy(JNIEnv*  env, jobject thiz)
	{
        mws_mod_ctrl::inst()->destroy_app();
	}

    JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1resume(JNIEnv*  env, jobject thiz)
    {
        mws_mod_ctrl::inst()->resume();
    }

    JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1pause(JNIEnv*  env, jobject thiz)
	{
        mws_mod_ctrl::inst()->pause();
	}

	JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1resize(JNIEnv*  env, jobject  thiz, jint i_w, jint i_h)
	{
	    mws_assert(i_w > 0 && i_h > 0);
        android_main::get_instance()->on_resize(i_w, i_h);
	}

	JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1touch_1event(JNIEnv* i_env, jobject i_this, jobject i_byte_buff)
	{
        uint8* byte_buff_addr = (uint8*)i_env->GetDirectBufferAddress(i_byte_buff);
        // type + count + 8 * (id, x, y, is_changed)
        const int touch_data_size = 4 + 4 + mws_ptr_evt_base::max_touch_points * (4 + 4 + 4 + 4);
        mws_ro_mem_seq ro_mem(byte_buff_addr, touch_data_size);
        data_seq_rdr_ptr dsr(&ro_mem);
        auto pfm_te = mws_ptr_evt_base::nwi();

        pfm_te->type = static_cast<mws_ptr_evt_base::e_touch_type>(dsr.read_int32());
        pfm_te->touch_count = dsr.read_uint32();
        pfm_te->time = pfm::time::get_time_millis();

        for (uint32 k = 0; k < pfm_te->touch_count; k++)
        {
            auto& te = pfm_te->points[k];

            te.identifier = dsr.read_uint32();
            te.x = dsr.read_real32();
            te.y = dsr.read_real32();
            te.is_changed = static_cast<bool>(dsr.read_uint32());
        }

        mws_mod_ctrl::inst()->pointer_action(pfm_te);
	}

	JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1render(JNIEnv*  env, jobject thiz)
	{
	    if(mws_mod_ctrl::inst()->is_set_app_exit_on_next_run())
        {
            exit_application();
        }
        else
        {
            android_main::get_instance()->run();
        }
	}

    JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1log(JNIEnv*  env, jobject thiz, jstring i_msg)
    {
        const char* msg = env->GetStringUTFChars(i_msg, 0);

        if (msg)
        {
            mws_log::i()->push(msg);
            env->ReleaseStringUTFChars(i_msg, msg);
        }
    }

	JNIEXPORT jboolean JNICALL Java_com_indigoabstract_appplex_main_native_1back_1evt(JNIEnv *env, jobject thiz)
	{
		bool r = android_main::get_instance()->back_evt();
		return jboolean(r ? 1 : 0);
	}
	
	JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1snd_1init(JNIEnv *env, jobject thiz, jint i_sample_rate, jint i_buffer_size)
	{
		//android_main::get_instance()->snd_init(i_sample_rate, i_buffer_size);
	}

	JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1snd_1close(JNIEnv *env, jobject thiz)
	{
		//android_main::get_instance()->snd_close();
	}

	JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1set_1writable_1path(JNIEnv*  env, jobject thiz, jstring i_writable_path)
	{
		const char* twritable_path = env->GetStringUTFChars(i_writable_path, 0);

		if (twritable_path)
		{
			writable_path = twritable_path;
			env->ReleaseStringUTFChars(i_writable_path, twritable_path);
		}
	}

    GL_APICALL void GL_APIENTRY glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, void *data)
    {
        mws_throw mws_exception("not implemented");
    }
}
