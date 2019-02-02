#include "main.hxx"

#include "pfm-gl.h"
#include "min.hxx"
#include "unit-ctrl.hxx"
#include "com/unit/input-ctrl.hxx"
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

	uint64 length() override
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

	uint64 creation_time() const override
	{
		std::string path = ppath.get_full_path();

		return 0;
	}

	uint64 last_write_time()const override
	{
		std::string path = ppath.get_full_path();

		return 0;
	}

	bool open_impl(std::string iopen_mode) override
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

	void close_impl() override
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

    void flush_impl() override
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

	void seek_impl(uint64 ipos, int iseek_pos) override
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
            mws_throw ia_exception("error");
        }
	}

	uint64 tell_impl() override
	{
		if(file)
		{
			return ftell(file);
		}

        mws_throw ia_exception("unsupported op");

		return 0;
	}

	int read_impl(uint8* ibuffer, int isize) override
	{
		if(file)
		{
			return fread(ibuffer, 1, isize, file);
		}
		
		return AAsset_read(asset_file, ibuffer, isize);
	}

	int write_impl(const uint8* ibuffer, int isize) override
	{
		if(file)
		{
			return fwrite(ibuffer, 1, isize, file);
		}
		
		mws_throw ia_exception("unsupported op");

		return 0;
	}

    FILE* file = nullptr;
    AAsset* asset_file = nullptr;
};


shared_ptr<android_main> android_main::instance;

android_main::android_main()
{
	plist = std::make_shared<umf_r>();
	is_started = false;
}

android_main::~android_main()
{
}

shared_ptr<android_main> android_main::get_instance()
{
	if (!instance)
	{
		instance = shared_ptr<android_main>(new android_main());
	}

	return instance;
}

shared_ptr<pfm_impl::pfm_file_impl> android_main::new_pfm_file_impl(const std::string& ifilename, const std::string& iroot_dir)
{
	return std::make_shared<android_file_impl>(ifilename, iroot_dir);
}

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

int android_main::get_screen_dpi()const
{
    JNIEnv* env = JniHelper::getEnv();
    jclass clazz = env->FindClass(CLASS_MAIN_PATH);
    jmethodID mid = env->GetStaticMethodID(clazz, "get_screen_dpi", "()I");
    jint dpi = env->CallStaticIntMethod(clazz, mid);

    return (int)dpi;
}

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

void get_directory_listing_helper(umf_list iplist, shared_ptr<pfm_file> ifile)
{
	if (iplist->find(ifile->get_file_name()) != iplist->end())
	{
		mws_print("android_main::get_directory_listing. duplicate filename: %s", ifile->get_full_path().c_str());
        mws_throw ia_exception("duplicate filename: " + ifile->get_full_path());
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

float android_main::get_screen_scale()
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

	unit_ctrl::inst()->pre_init_app();
	unit_ctrl::inst()->set_gfx_available(true);
	unit_ctrl::inst()->init_app();

	is_started = true;
}

void android_main::start()
{
	unit_ctrl::inst()->start_app();
}

void android_main::run()
{
	unit_ctrl::inst()->update();
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

	JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1init_1renderer(JNIEnv* env, jobject obj, jobject i_asset_manager, jstring i_apk_path)
	{
		initJavaMethodsIDs(env, obj);

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
        unit_ctrl::inst()->destroy_app();
	}

    JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1resume(JNIEnv*  env, jobject thiz)
    {
        unit_ctrl::inst()->resume();
    }

    JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1pause(JNIEnv*  env, jobject thiz)
	{
        unit_ctrl::inst()->pause();
	}

	JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1resize(JNIEnv*  env, jobject  thiz, jint i_w, jint i_h)
	{
		unit_ctrl::inst()->resize_app(i_w, i_h);
	}

	JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1touch_1event
			(JNIEnv*  env, jobject thiz, jint i_touch_type, jint i_touch_count, jintArray i_touch_points_identifier,
			 jbooleanArray i_touch_points_is_changed, jfloatArray i_touch_points_x, jfloatArray i_touch_points_y)
	{
		auto pfm_te = std::make_shared<pointer_evt>();
		jint* touch_points_identifier = env->GetIntArrayElements(i_touch_points_identifier, NULL);
		jboolean* touch_points_is_changed = env->GetBooleanArrayElements(i_touch_points_is_changed, NULL);
		jfloat* touch_points_x = env->GetFloatArrayElements(i_touch_points_x, NULL);
		jfloat* touch_points_y = env->GetFloatArrayElements(i_touch_points_y, NULL);

		for (int k = 0; k < i_touch_count; k++)
		{
            auto& te = pfm_te->points[k];

			te.identifier = touch_points_identifier[k];
			te.is_changed = touch_points_is_changed[k];
			te.x = touch_points_x[k];
			te.y = touch_points_y[k];
		}

		env->ReleaseIntArrayElements(i_touch_points_identifier, touch_points_identifier, 0);
		env->ReleaseBooleanArrayElements(i_touch_points_is_changed, touch_points_is_changed, 0);
		env->ReleaseFloatArrayElements(i_touch_points_x, touch_points_x, 0);
		env->ReleaseFloatArrayElements(i_touch_points_y, touch_points_y, 0);

		pfm_te->time = pfm::time::get_time_millis();
		pfm_te->touch_count = i_touch_count;
		pfm_te->type = (pointer_evt::e_touch_type)i_touch_type;

		unit_ctrl::inst()->pointer_action(pfm_te);
	}

	JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1render(JNIEnv*  env, jobject thiz)
	{
	    if(unit_ctrl::inst()->is_set_app_exit_on_next_run())
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
		android_main::get_instance()->snd_init(i_sample_rate, i_buffer_size);
	}

	JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1snd_1close(JNIEnv *env, jobject thiz)
	{
		android_main::get_instance()->snd_close();
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
        mws_throw ia_exception("not implemented");
    }
}
