#include "main.hpp"

#include "min.hpp"
#include "unit-ctrl.hpp"
#include "java-callbacks.h"
#include "jni-helper.hpp"
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
		is_external = false;
	}

	virtual ~android_file_impl() {}

	virtual uint64 length()
	{
		uint64 size = 0;

		if (!file)
		{
			open("rb");

			if (file)
			{
				if(is_external)
				{
					size = ftell((FILE*)file);
				}
				else
				{
					size = AAsset_getLength64((AAsset*)file);
				}
				
				close();
			}
		}
		else
		{
			if(is_external)
			{
				size = ftell((FILE*)file);
			}
			else
			{
				size = AAsset_getLength64((AAsset*)file);
			}
		}

		return size;
	}

	virtual uint64 creation_time()const
	{
		std::string path = ppath.get_full_path();

		return 0;
	}

	virtual uint64 last_write_time()const
	{
		std::string path = ppath.get_full_path();

		return 0;
	}

	virtual void* open_impl(std::string iopen_mode)
	{
		std::string path = ppath.get_full_path();
		is_external = false;

		if (iopen_mode[0] == 'w' && path[0] != '/')
		{
			return nullptr;
		}

		if(path[0] == '/')
		{
			// external path
			FILE* file = fopen(path.c_str(), iopen_mode.c_str());
			is_external = true;
			vprint("open_impl: opening external file %s\n", path.c_str());
//			fseek( file, 0, SEEK_END );
//			uint64 len = ftell((FILE*)file);
//			fseek(file, 0, SEEK_SET );

			return file;
		}

		AAsset* asset = AAssetManager_open(asset_manager, path.c_str(), 0);
		vprint("open_impl: opening asset file %s\n", path.c_str());

		return asset;
	}

	virtual void close_impl()
	{
		if(is_external)
		{
			fclose((FILE*)file);
		}
		else
		{
			AAsset_close((AAsset*)file);
		}
		
		file = nullptr;
	}

	virtual void seek_impl(uint64 ipos, int iseek_pos)
	{
		if(is_external)
		{
			fseek((FILE*)file, ipos, iseek_pos);
		}
		else
		{
			AAsset_seek64((AAsset*)file, ipos, iseek_pos);
		}
	}

	virtual uint64 tell_impl()
	{
		if(is_external)
		{
			return ftell((FILE*)file);
		}
		
		throw ia_exception("unsupported op");

		return 0;
	}

	virtual int read_impl(uint8* ibuffer, int isize)
	{
		if(is_external)
		{
			return fread(ibuffer, 1, isize, (FILE*)file);
		}
		
		return AAsset_read((AAsset*)file, ibuffer, isize);
	}

	virtual int write_impl(const uint8* ibuffer, int isize)
	{
		if(is_external)
		{
			return fwrite(ibuffer, 1, isize, (FILE*)file);
		}
		
		throw ia_exception("unsupported op");

		return 0;
	}

	bool is_external;
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

void get_directory_listing_helper(umf_list iplist, shared_ptr<pfm_file> ifile)
{
	if (iplist->find(ifile->get_file_name()) != iplist->end())
	{
		vprint("android_main::get_directory_listing. duplicate filename: %s", ifile->get_full_path().c_str());
		throw ia_exception("duplicate filename: " + ifile->get_full_path());
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
				if (starts_with(e->get_root_directory(), idirectory))
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
	//vprint("android_main::load_apk_file_list zip %s %d", g_apk_path.c_str(), file_count);

	for (int i = 0; i < file_count; i++)
	{
		const char* name = zip_get_name(apk_archive, i, 0);

		if (name == nullptr)
		{
			vprint("Error reading zip file name at index %i : %s", i, zip_strerror(apk_archive));

			return;
		}
		else
		{
			std::string full_path(name);

			if (starts_with(full_path, assets_pfx))
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
				//vprint("file %i : %s\n", i, p_file->get_full_path().c_str());
			}
		}
	}

	//vprint("android_main::load_apk_file_list 2");
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

	char* GetBuildVersion()
	{
		return (char*)JavaCallStaticMethodByObject_string("GetBuildVersion", "()Ljava/lang/String;");
	}

	void ExitApplication()
	{
		JavaCallStaticMethodByObject_void("ExitApplication", "()V");
	}

	JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_mainRenderer_nativeResume(JNIEnv*  env, jobject thiz)
	{
	}

	JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_mainRenderer_nativeInitRenderer(JNIEnv* env, jobject obj, jobject iasset_manager, jstring iapk_path)
	{
		initJavaMethodsIDs(env, obj);

		const char* apk_path = env->GetStringUTFChars(iapk_path, 0);

		if (apk_path)
		{
			g_apk_path = apk_path;
			env->ReleaseStringUTFChars(iapk_path, apk_path);
		}

		asset_manager = AAssetManager_fromJava(env, iasset_manager);

		if (!asset_manager)
		{
			trx("error loading asset manager");
		}
		else
		{
			trx("loaded asset manager");
		}

		android_main::get_instance()->init();
	}

	JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_mainRenderer_nativeStartApp(JNIEnv*  env, jobject thiz)
	{
		android_main::get_instance()->start();
	}

	JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_mainRenderer_nativeDestroy(JNIEnv*  env, jobject thiz)
	{
	}

	JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_mainRenderer_nativePause(JNIEnv*  env, jobject thiz)
	{
	}

	JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_mainRenderer_nativeResize(JNIEnv*  env, jobject  thiz, jint w, jint h)
	{
		unit_ctrl::inst()->resize_app(w, h);
	}

	JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_mainRenderer_nativeTouchEvent(JNIEnv*  env, jobject thiz, jint finger_id, jint finger_state, jfloat x, jfloat y)
	{
		pointer_actions action_type;

		switch (finger_state)
		{
		case 0:
			action_type = POINTER_PRESS;
			break;

		case 1:
			action_type = POINTER_RELEASE;
			break;

		case 2:
			action_type = POINTER_DRAGG;
			break;

		default:
			return;
		}

		unit_ctrl::inst()->pointer_action(finger_id, action_type, x, y);
	}

	JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_mainRenderer_nativeRender(JNIEnv*  env, jobject thiz)
	{
		android_main::get_instance()->run();
	}

	JNIEXPORT jboolean JNICALL Java_com_indigoabstract_appplex_main_native_1back_1evt(JNIEnv *env, jobject thiz)
	{
		bool r = android_main::get_instance()->back_evt();
		return jboolean(r ? 1 : 0);
	}
	
	JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1snd_1init(JNIEnv *env, jobject thiz, jint isample_rate, jint ibuffer_size)
	{
		android_main::get_instance()->snd_init(isample_rate, ibuffer_size);
	}

	JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1snd_1close(JNIEnv *env, jobject thiz)
	{
		android_main::get_instance()->snd_close();
	}

	JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1set_1writable_1path(JNIEnv*  env, jobject thiz, jstring iwritable_path)
	{
		const char* twritable_path = env->GetStringUTFChars(iwritable_path, 0);

		if (twritable_path)
		{
			writable_path = twritable_path;
			env->ReleaseStringUTFChars(iwritable_path, twritable_path);
		}

	}
}
