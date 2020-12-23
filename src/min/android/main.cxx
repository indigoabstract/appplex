#include "stdafx.hxx"

#include "pfm.hxx"
#include "mws-impl.hxx"
#include "pfm-gl.h"
#include "min.hxx"
#include "mws-mod.hxx"
#include "mws-mod-ctrl.hxx"
#include "data-sequence.hxx"
#include "gfx.hxx"
#include "java-callbacks.h"
#include "jni-helper.hxx"
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
#include <vector>


class android_main : public mws_app
{
public:
   android_main();
   virtual ~android_main();
   void init();
   void start();
   void run();

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
   virtual void write_text(const char* i_text) const override;
   virtual void write_text_nl(const char* i_text) const override;
   virtual void write_text(const wchar_t* i_text) const override;
   virtual void write_text_nl(const wchar_t* i_text) const override;
   virtual void write_text_v(const char* i_format, ...) const override;
   // filesystem
   virtual mws_sp<mws_file_impl> new_mws_file_impl(const mws_path& i_path, bool i_is_internal = false) const override;
   //virtual mws_file_map list_internal_directory() const override;
   virtual const mws_path& prv_dir() const override;
   virtual const mws_path& res_dir() const override;
   virtual const mws_path& tmp_dir() const override;
   virtual void reconfigure_directories(mws_sp<mws_mod> i_crt_mod) override;
   virtual std::string get_timezone_id() const override;

   void snd_init(int i_sample_rate, int i_buffer_size);
   void snd_close();

   void init_screen_metrix(uint32 i_screen_width, uint32 i_screen_height, float i_screen_horizontal_dpi, float i_screen_vertical_dpi);
   void on_resize(uint32 i_screen_width, uint32 i_screen_height);

   int sample_rate;
   int buffer_size;

private:
   std::vector<mws_sp<mws_file> > apk_file_list;
   mws_file_map plist;
   // screen metrix
   std::pair<uint32, uint32> screen_res;
   float avg_screen_dpi = 0.f;
   std::pair<float, float> screen_dpi;
   std::pair<float, float> screen_dim_inch;
   float avg_screen_dpcm = 0.f;
   std::pair<float, float> screen_dpcm;
   std::pair<float, float> screen_dim_cm;
};


namespace
{
   const std::string assets_pfx = "assets/";
   std::string g_apk_path;
   AAssetManager* asset_manager = NULL;
   std::string global_prv_path;
   std::string global_tmp_path;
   mws_path prv_path;
   mws_path res_path;
   mws_path tmp_path;
   bool prv_path_exists = false;
   bool tmp_path_exists = false;
   mws_sp<android_main> instance;
}
JavaVM* g_p_java_vm = NULL;
jobject g_obj = NULL;


static mws_sp<android_main> app_inst()
{
   if (!instance)
   {
      instance = mws_sp<android_main>(new android_main());
   }

   return instance;
}

mws_sp<mws_app> mws_app_inst() { return app_inst(); }


class android_file_impl : public mws_file_impl
{
public:
   android_file_impl(const mws_path& i_path, bool i_is_internal = false) : mws_file_impl(i_path, i_is_internal) {}

   virtual ~android_file_impl() {}

   FILE* get_file_impl() const override
   {
      return file;
   }

   virtual uint64 length() override
   {
      uint64 size = 0;

      if (asset_file)
      {
         size = AAsset_getLength64(asset_file);
      }
      else if (file)
      {
         long crt_pos = ftell(file);

         fseek(file, 0, SEEK_END);
         size = ftell(file);
         fseek(file, crt_pos, SEEK_SET);
      }
      else
      {
         open_impl("rb");

         if (asset_file)
         {
            size = AAsset_getLength64(asset_file);
            close_impl();
         }
         else if (file)
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
      std::string path = ppath.string();

      return 0;
   }

   virtual uint64 last_write_time()const override
   {
      std::string path = ppath.string();

      return 0;
   }

   virtual bool open_impl(std::string i_open_mode) override
   {
      const std::string& path = ppath.string();

      if (i_open_mode[0] == 'w' && path[0] != '/')
      {
         return false;
      }

      if (path[0] == '/')
      {
         // external path
         file = fopen(path.c_str(), i_open_mode.c_str());
         bool file_opened = (file != nullptr);
         mws_print("open_impl: opening external file %s\n", path.c_str());

         return file_opened;
      }

      asset_file = AAssetManager_open(asset_manager, path.c_str(), 0);
      mws_print("open_impl: opening asset file %s\n", path.c_str());

      return asset_file != nullptr;
   }

   virtual void close_impl() override
   {
      if (file)
      {
         fclose(file);
         file = nullptr;
      }
      else if (asset_file)
      {
         AAsset_close(asset_file);
         asset_file = nullptr;
      }
      else
      {
         mws_print("error[ file [ %s ] is not open! ]", ppath.string().c_str());
      }
   }

   virtual void flush_impl() override
   {
      if (file)
      {
         fflush(file);
      }
      else if (asset_file)
      {
         mws_print("error[ cannot flush an asset file! [ %s ] ]", ppath.string().c_str());
      }
      else
      {
         mws_print("error[ file [ %s ] is not open! ]", ppath.string().c_str());
      }
   }

   virtual void set_io_position_impl(uint64 i_pos, int i_io_pos) override
   {
      if (file)
      {
         fseek(file, i_pos, i_io_pos);
      }
      else if (asset_file)
      {
         AAsset_seek64(asset_file, i_pos, i_io_pos);
      }
      else
      {
         mws_throw mws_exception("error");
      }
   }

   virtual uint64 tell_impl() override
   {
      if (file)
      {
         return ftell(file);
      }

      mws_throw mws_exception("unsupported op");

      return 0;
   }

   virtual int read_impl(uint8* i_buffer, int i_size) override
   {
      if (file)
      {
         return fread(i_buffer, 1, i_size, file);
      }

      return AAsset_read(asset_file, i_buffer, i_size);
   }

   virtual int write_impl(const uint8* i_buffer, int i_size) override
   {
      if (file)
      {
         return fwrite(i_buffer, 1, i_size, file);
      }

      mws_throw mws_exception("unsupported op");

      return 0;
   }

   FILE* file = nullptr;
   AAsset* asset_file = nullptr;
};


android_main::android_main() {}

android_main::~android_main()
{
}

mws_key_types android_main::translate_key(int i_pfm_key_id) const
{
   if (i_pfm_key_id > mws_key_invalid && i_pfm_key_id < mws_key_count)
   {
      return static_cast<mws_key_types>(i_pfm_key_id);
   }

   return mws_key_invalid;
}

mws_key_types android_main::apply_key_modifiers_impl(mws_key_types i_key_id) const { return i_key_id; }

bool android_main::is_full_screen_mode() const
{
   return true;
}

void android_main::set_full_screen_mode(bool ienabled) const
{
}

float android_main::get_screen_scale() const
{
   return 1.f;
}

float android_main::get_screen_brightness() const
{
   JNIEnv* env = JniHelper::getEnv();
   jclass clazz = env->FindClass(CLASS_MAIN_PATH);
   jmethodID mid = env->GetStaticMethodID(clazz, "get_screen_brightness", "()F");
   jfloat brightness = env->CallStaticFloatMethod(clazz, mid);

   return (float)brightness;
}

void android_main::set_screen_brightness(float i_brightness) const
{
   JNIEnv* env = JniHelper::getEnv();
   jclass clazz = env->FindClass(CLASS_MAIN_PATH);
   jmethodID mid = env->GetStaticMethodID(clazz, "set_screen_brightness", "(F)V");
   env->CallStaticVoidMethod(clazz, mid, (jfloat)i_brightness);
}

std::pair<uint32, uint32> android_main::get_screen_res_px() const { return screen_res; }
float android_main::get_avg_screen_dpi() const { return avg_screen_dpi; }
std::pair<float, float> android_main::get_screen_dpi() const { return screen_dpi; }
std::pair<float, float> android_main::get_screen_dim_inch() const { return screen_dim_inch; }
float android_main::get_avg_screen_dpcm() const { return avg_screen_dpcm; }
std::pair<float, float> android_main::get_screen_dpcm() const { return screen_dpcm; }
std::pair<float, float> android_main::get_screen_dim_cm() const { return screen_dim_cm; }

void android_main::write_text(const char* i_text)const
{
   __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, i_text, 0);
}

void android_main::write_text_nl(const char* i_text)const
{
   write_text(i_text);
}

void android_main::write_text(const wchar_t* i_text)const
{
   write_text("wchar write_text not implemented");
}

void android_main::write_text_nl(const wchar_t* i_text)const
{
   write_text("wchar write_text_nl not implemented");
}

void android_main::write_text_v(const char* i_format, ...)const
{
   va_list arg_list;

   va_start(arg_list, i_format);
   __android_log_vprint(ANDROID_LOG_VERBOSE, APPNAME, i_format, arg_list);
   va_end(arg_list);
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

mws_sp<mws_file_impl> android_main::new_mws_file_impl(const mws_path& i_path, bool i_is_internal) const
{
   return std::make_shared<android_file_impl>(i_path, i_is_internal);
}

const mws_path& android_main::prv_dir() const
{
   if (!prv_path_exists)
   {
      prv_path_exists = mws_make_directory(prv_path);
   }

   return prv_path;
}

const mws_path& android_main::res_dir() const
{
   return res_path;
}

const mws_path& android_main::tmp_dir() const
{
   if (!tmp_path_exists)
   {
      tmp_path_exists = mws_make_directory(tmp_path);
   }

   return tmp_path;
}

void android_main::reconfigure_directories(mws_sp<mws_mod> i_crt_mod)
{
   std::string mod_dir = i_crt_mod->get_name() + "/";

   mws_assert(i_crt_mod != nullptr);
   prv_path = mws_path(global_prv_path) / mod_dir;
   res_path = mws_path(mod_dir);
   tmp_path = mws_path(global_tmp_path) / mod_dir;
   prv_path_exists = false;
   tmp_path_exists = false;
}

std::string android_main::get_timezone_id()const
{
   JNIEnv* env = JniHelper::getEnv();
   jclass clazz = env->FindClass(CLASS_MAIN_PATH);
   jmethodID mid = env->GetStaticMethodID(clazz, "get_timezone_id", "()Ljava/lang/String;");
   jstring timezone_id = (jstring)env->CallStaticObjectMethod(clazz, mid);
   const char* t_timezone_id = env->GetStringUTFChars(timezone_id, 0);
   std::string ret;

   if (t_timezone_id)
   {
      ret = t_timezone_id;
      env->ReleaseStringUTFChars(timezone_id, t_timezone_id);
   }

   return ret;
}

void android_main::init()
{
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

   if (is_landscape_0 != is_landscape_1)
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
   JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
   {
      JniHelper::setJavaVM(vm);
      g_p_java_vm = vm;

      return JNI_VERSION_1_4;
   }

   JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved)
   {
      JNIEnv* env = NULL;

      env->DeleteGlobalRef(g_obj);
      g_obj = NULL;
      g_p_java_vm = NULL;
   }


   void initJavaMethodsIDs(JNIEnv* env, jobject obj)
   {
      g_obj = env->NewGlobalRef(obj);
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
      app_inst()->init_screen_metrix(i_screen_width, i_screen_height, i_screen_horizontal_dpi, i_screen_vertical_dpi);

      const char* apk_path = env->GetStringUTFChars(i_apk_path, 0);

      if (apk_path)
      {
         g_apk_path = apk_path;
         env->ReleaseStringUTFChars(i_apk_path, apk_path);
      }

      asset_manager = AAssetManager_fromJava(env, i_asset_manager);

      if (asset_manager)
      {
         app_inst()->init();
         mws_print("asset manager loaded");
      }
      else
      {
         mws_print("error loading asset manager");
      }
   }

   JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1start_1app(JNIEnv* env, jobject thiz)
   {
      app_inst()->start();
   }

   JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1destroy(JNIEnv* env, jobject thiz)
   {
      mws_mod_ctrl::inst()->destroy_app();
   }

   JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1resume(JNIEnv* env, jobject thiz)
   {
      mws_mod_ctrl::inst()->resume();
   }

   JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1pause(JNIEnv* env, jobject thiz)
   {
      mws_mod_ctrl::inst()->pause();
   }

   JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1resize(JNIEnv* env, jobject  thiz, jint i_w, jint i_h)
   {
      mws_assert(i_w > 0 && i_h > 0);
      app_inst()->on_resize(i_w, i_h);
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
      pfm_te->time = mws::time::get_time_millis();

      for (uint32 k = 0; k < pfm_te->touch_count; k++)
      {
         auto& te = pfm_te->points[k];

         te.identifier = dsr.read_uint32();
         te.x = dsr.read_fltp32();
         te.y = dsr.read_fltp32();
         te.is_changed = static_cast<bool>(dsr.read_uint32());
      }

      mws_mod_ctrl::inst()->pointer_action(pfm_te);
   }

   JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1render(JNIEnv* env, jobject thiz)
   {
      if (mws_mod_ctrl::inst()->is_set_app_exit_on_next_run())
      {
         exit_application();
      }
      else
      {
         app_inst()->run();
      }
   }

   JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1log(JNIEnv* env, jobject thiz, jstring i_msg)
   {
      const char* msg = env->GetStringUTFChars(i_msg, 0);

      if (msg)
      {
         mws_log::i()->push(msg);
         env->ReleaseStringUTFChars(i_msg, msg);
      }
   }

   JNIEXPORT jboolean JNICALL Java_com_indigoabstract_appplex_main_native_1back_1evt(JNIEnv* env, jobject thiz)
   {
      bool r = app_inst()->back_evt();
      return jboolean(r ? 1 : 0);
   }

   JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1snd_1init(JNIEnv* env, jobject thiz, jint i_sample_rate, jint i_buffer_size)
   {
      //app_inst()->snd_init(i_sample_rate, i_buffer_size);
   }

   JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1snd_1close(JNIEnv* env, jobject thiz)
   {
      //app_inst()->snd_close();
   }

   JNIEXPORT void JNICALL Java_com_indigoabstract_appplex_main_native_1set_1prv_1tmp_1dirs(JNIEnv* env, jobject thiz, jstring i_prv_dir, jstring i_tmp_dir)
   {
      const char* t_prv_dir = env->GetStringUTFChars(i_prv_dir, 0);
      const char* t_tmp_dir = env->GetStringUTFChars(i_tmp_dir, 0);

      if (t_prv_dir)
      {
         global_prv_path = t_prv_dir;
         global_tmp_path = t_tmp_dir;
         env->ReleaseStringUTFChars(i_prv_dir, t_prv_dir);
         env->ReleaseStringUTFChars(i_tmp_dir, t_tmp_dir);
      }
   }

   GL_APICALL void GL_APIENTRY glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, void* data)
   {
      mws_throw mws_exception("not implemented");
   }
}
