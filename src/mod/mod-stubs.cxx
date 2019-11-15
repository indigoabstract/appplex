#include "stdafx.hxx"

#include "appplex-conf.hxx"
#include "mod-list.hxx"
#include "pfm.hxx"


extern const bool mod_ffmpeg_on = MOD_FFMPEG;
extern const bool mod_fmod_on = MOD_FMOD;
extern const bool mod_fmt_on = MOD_FMT;
extern const bool mod_freeimage_on = MOD_FREEIMAGE;
extern const bool mod_freetype_on = MOD_FREETYPE;
extern const bool mod_freetype_gl_on = MOD_FREETYPE_GL;
extern const bool mod_gfx_on = MOD_GFX;
extern const bool mod_gfx_ext_on = MOD_GFX_EXT;
extern const bool mod_gl_on = MOD_GL;
extern const bool mod_glm_on = MOD_GLM;
extern const bool mod_input_on = MOD_INPUT;
extern const bool mod_vrn_on = MOD_VRN;
extern const bool mod_kxmd_on = MOD_KXMD;
extern const bool mod_mws_on = MOD_MWS;
extern const bool mod_mws_vkb_on = MOD_MWS_VKB;
extern const bool mod_noise_on = MOD_NOISE;
extern const bool mod_notifications_on = MOD_NOTIFICATIONS;
extern const bool mod_ogg_on = MOD_OGG;
extern const bool mod_ovg_on = MOD_OVG;
extern const bool mod_png_on = MOD_PNG;
extern const bool mod_res_ld_on = MOD_RES_LD;
extern const bool mod_rng_on = MOD_RNG;
extern const bool mod_snd_on = MOD_SND;
extern const bool mod_stk_on = MOD_STK;
extern const bool mod_tiny_obj_loader_on = MOD_TINY_OBJ_LOADER;
extern const bool mod_util_on = MOD_UTIL;
extern const bool mod_vg_on = MOD_VG;
extern const bool mod_vid_on = MOD_VID;
extern const bool mod_vorbis_on = MOD_VORBIS;


static void err_na() { mws_throw mws_exception("call not available"); }


#if !MOD_GFX
#include "gfx/gfx-inc.hxx"
void mws_report_gfx_errs_impl(const char*, uint32) { err_na(); }

mws_sp<gfx> gfx::main_instance;
void gfx::global_init() { err_na(); }
void gfx::on_destroy() { err_na(); }
void gfx::on_resize(int, int) { err_na(); }
void gfx::ic_rt::set_current_render_target(mws_sp<gfx_rt> irdt, bool i_force_binding) { err_na(); }

void gfx_scene::draw() { err_na(); }
void gfx_scene::post_draw() { err_na(); }

mws_cm mws_cm::to_cm() const { return mws_cm(); }
mws_in mws_cm::to_in() const { return mws_in(); }
mws_pt mws_cm::to_pt() const { return mws_pt(); }
mws_px mws_cm::to_px(dpi_types i_dpi_type) const { return mws_px(); }
mws_cm mws_in::to_cm() const { return mws_cm(); }
mws_in mws_in::to_in() const { return mws_in(); }
mws_pt mws_in::to_pt() const { return mws_pt(); }
mws_px mws_in::to_px(dpi_types i_dpi_type) const { return mws_px(); }
mws_cm mws_pt::to_cm() const { return mws_cm(); }
mws_in mws_pt::to_in() const { return mws_in(); }
mws_pt mws_pt::to_pt() const { return mws_pt(); }
mws_px mws_pt::to_px(dpi_types i_dpi_type) const { return mws_px(); }
mws_cm mws_px::to_cm() const { return mws_cm(); }
mws_in mws_px::to_in() const { return mws_in(); }
mws_pt mws_px::to_pt() const { return mws_pt(); }
mws_px mws_px::to_px(dpi_types i_dpi_type) const { return mws_px(); }
#endif

#if !MOD_INPUT
#include "input/input-ctrl.hxx"
#include "input/update-ctrl.hxx"
mws_sp<key_ctrl> key_ctrl::nwi() { err_na(); return nullptr; }
void key_ctrl::key_pressed(key_types) {}
void key_ctrl::key_released(key_types) {}
void key_ctrl::update() {}
const std::string mws_key_evt::KEYEVT_EVT_TYPE;
mws_sp<mws_key_evt> mws_key_evt::as_key_evt(mws_sp<mws_dp>) { err_na(); return nullptr; }
key_types mws_key_evt::get_key(void) const { err_na(); return KEY_INVALID; }
mws_key_evt::key_evt_types mws_key_evt::get_type() const { err_na(); return mws_key_evt::KE_PRESSED; }
const std::string mws_ptr_evt::TOUCHSYM_EVT_TYPE;
mws_sp<mws_ptr_evt> mws_ptr_evt::nwi() { err_na(); return nullptr; }
mws_sp<mws_ptr_evt> mws_ptr_evt::as_pointer_evt(mws_sp<mws_dp>) { err_na(); return nullptr; }
mws_sp<touchctrl> touchctrl::nwi() { err_na(); return nullptr; }
void touchctrl::enqueue_pointer_event(mws_sp<mws_ptr_evt_base>) {}
void touchctrl::update() {}
mws_sp<updatectrl> updatectrl::nwi() { err_na(); return nullptr; }
int updatectrl::getTimeStepDuration() { err_na(); return 0; }
#endif

#if !MOD_KAWASE_BLOOM
#include "kawase-bloom/kawase-bloom.hxx"
mws_sp<mws_kawase_bloom> mws_kawase_bloom::nwi(mws_sp<gfx_tex> i_input_tex) { return nullptr; }
mws_sp<gfx_tex> mws_kawase_bloom::get_bloom_tex() const { return nullptr; }
void mws_kawase_bloom::update() {}
#endif

#if !MOD_MWS
#include "mws/mws.hxx"
bool mws::is_visible() const { return false; }
#endif

#if !MOD_MWS_VKB
#include "mws-vkb/mws-vkb.hxx"
mws_sp<mws_vkb> mws_vkb::gi() { err_na(); return nullptr; }
key_types mws_vkb::apply_key_modifiers(key_types i_key_id) const { return KEY_INVALID; }
void mws_vkb::receive(mws_sp<mws_dp>) {}
void mws_vkb::update_state() {}
void mws_vkb::setup() {}
void mws_vkb::on_resize() {}
void mws_vkb::set_target(mws_sp<mws_text_area>) {}
mws_sp<mws_font> mws_vkb::get_font() { return nullptr; }
void mws_vkb::set_font(mws_sp<mws_font>, mws_sp<mws_font>) {}
mws_sp<mws_vkb_file_store> mws_vkb::get_file_store() const { return nullptr; }
void mws_vkb::set_file_store(mws_sp<mws_vkb_file_store>) {}
std::vector<mws_sp<gfx_tex>> mws_vkb::get_tex_list() { return std::vector<mws_sp<gfx_tex>>(); }
void mws_vkb::done() {}
mws_sp<mws_vkb_impl> mws_vkb::get_impl() { return nullptr; }
#endif

#if !MOD_OMNIRECALL
#include "pfm.hxx"
bool mws_log::is_enabled() { return false; }
void mws_log::set_enabled(bool i_is_enabled) {}
mws_sp<mws_log> mws_log::i() { err_na(); return nullptr; }
const std::vector<std::string> mws_log::get_log() { return std::vector<std::string>(); }
void mws_log::push(const char* i_msg) {}
void mws_log::pushf(const char* i_fmt, ...) {}
void mws_log::clear() {}
mws_log::mws_log() {}
mws_sp<mws_log> mws_log::inst;
#endif

#if !MOD_RES_LD
#include "res-ld/res-ld.hxx"
mws_sp<res_ld> res_ld::inst() { return nullptr; }
mws_sp<gfx_tex> res_ld::load_tex(std::string i_filename) { return nullptr; }
mws_sp<raw_img_data> res_ld::load_image(mws_sp<pfm_file> i_file) { return nullptr; }
mws_sp<raw_img_data> res_ld::load_image(std::string i_filename) { return nullptr; }
#endif

#if !MOD_SND
#include "snd/snd.hxx"
void snd::init() { err_na(); }
void snd::update() { err_na(); }
#endif
