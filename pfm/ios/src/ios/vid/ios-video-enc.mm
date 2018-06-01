#include "stdafx.h"

#include "appplex-conf.hpp"
#include "ios-video-enc.hpp"
#include "ios-video-dec.hpp"
#include "pfm.hpp"
#include "min.hpp"
#include "ViewController.h"


#if defined PLATFORM_IOS

class ios_video_enc_impl
{
public:
	mws_vid_enc_st get_state() const
	{
		return state;
	}

	std::string get_video_path()
	{
		return video_path;
	}

	void set_video_path(std::string i_video_path)
	{
		video_path = i_video_path;
	}

	void start_encoding(std::shared_ptr<gfx> i_gi, const mws_video_params& i_prm, mws_vid_enc_method i_enc_method)
	{
		mws_assert(!video_path.empty());
		state = mws_vid_enc_st::e_st_encoding;
		enc_method = i_enc_method;
		
		// if a file with that name already exists, delete the old movie
		//_unlink(i_dst_path.c_str());

		//NSString* src_path_nss = [[NSString alloc] initWithUTF8String:i_src_path.c_str()];
		//NSString* dst_path_nss = [[NSString alloc] initWithUTF8String:i_dst_path.c_str()];
		//mws_print("\n\nencode_selected_videoooooooo : %s %s \n\n\n", i_src_path.c_str(), i_dst_path.c_str());
		//[[ViewController inst] encode_video:src_path_nss i_dst_path:dst_path_nss];
	}

	void encode_frame_m0_yuv420(const uint8* y_frame, const uint8* u_frame, const uint8* v_frame)
	{
		mws_throw ia_exception("n/a");
	}
	
	void encode_frame_m1_yuv420(const char* iframe_data, int iframe_data_length)
	{
		mws_throw ia_exception("n/a");
	}
	
	void encode_frame_m2_rbga(std::shared_ptr<gfx> i_gi, std::shared_ptr<gfx_tex> i_frame_tex)
	{
		mws_throw ia_exception("n/a");
	}

	void stop_encoding()
	{
		state = mws_vid_enc_st::e_st_finished;
	}
	
	std::string video_path;
	mws_vid_enc_st state;
	mws_vid_enc_method enc_method;
};


std::shared_ptr<ios_video_enc> ios_video_enc::nwi()
{
	auto i = std::shared_ptr<ios_video_enc>(new ios_video_enc());
	i->p = std::shared_ptr<ios_video_enc_impl>(new ios_video_enc_impl());
	return i;
}

mws_vid_enc_st ios_video_enc::get_state() const
{
	return p->get_state();
}

mws_vid_enc_method ios_video_enc::get_enc_method() const
{
    return p->enc_method;
}

std::string ios_video_enc::get_video_path()
{
	return p->get_video_path();
}

void ios_video_enc::set_video_path(std::string i_video_path)
{
	p->set_video_path(i_video_path);
}

void ios_video_enc::start_encoding(std::shared_ptr<gfx> i_gi, const mws_video_params& i_prm, mws_vid_enc_method i_enc_method)
{
	p->start_encoding(i_gi, i_prm, i_enc_method);
}

void ios_video_enc::encode_frame_m0_yuv420(const uint8* y_frame, const uint8* u_frame, const uint8* v_frame)
{
	p->encode_frame_m0_yuv420(y_frame, u_frame, v_frame);
}

void ios_video_enc::encode_frame_m1_yuv420(const char* iframe_data, int iframe_data_length)
{
	p->encode_frame_m1_yuv420(iframe_data, iframe_data_length);
}

void ios_video_enc::encode_frame_m2_rbga(std::shared_ptr<gfx> i_gi, std::shared_ptr<gfx_tex> i_frame_tex)
{
	p->encode_frame_m2_rbga(i_gi, i_frame_tex);
}

void ios_video_enc::stop_encoding()
{
	p->stop_encoding();
}


class ios_video_reencoder_impl
{
public:
   ios_video_reencoder_impl()
   {
	   venc = ios_video_enc::nwi();
   }
   
   //mws_sp<ios_video_dec> vdec;
    mws_vdec_state vdec_state;
   mws_sp<ios_video_enc> venc;
   // mws_sp<mws_ffmpeg_vdec_listener> vdec_listener;
   // mws_sp<mws_video_reencoder_listener> video_reencoder_listener;
   mws_video_params params;
   std::string src_video_path;
};


mws_sp<ios_video_reencoder> ios_video_reencoder::nwi()
{
   auto p = mws_sp<ios_video_reencoder_impl>(new ios_video_reencoder_impl());
   auto r = mws_sp<ios_video_reencoder>(new ios_video_reencoder());
   r->p = p;

   return r;
}

mws_vdec_state ios_video_reencoder::get_dec_state() const
{
   return p->vdec_state;
}

mws_vid_enc_st ios_video_reencoder::get_enc_state() const
{
   return p->venc->get_state();
}

std::string ios_video_reencoder::get_src_video_path()
{
   return p->src_video_path;
}

void ios_video_reencoder::set_src_video_path(std::string i_video_path)
{
   p->src_video_path = i_video_path;
}

std::string ios_video_reencoder::get_dst_video_path()
{
   return p->venc->get_video_path();
}

void ios_video_reencoder::set_dst_video_path(std::string i_video_path)
{
   p->venc->set_video_path(i_video_path);
}

void ios_video_reencoder::start_encoding(const mws_video_params& i_prm)
{
    p->vdec_state = mws_vdec_state::st_playing;
   //p->vdec->start_decoding();
}

void ios_video_reencoder::stop_encoding()
{
    mws_throw ia_exception("n/a");
   //p->vdec->stop();
}

void ios_video_reencoder::update()
{
   //p->update();
}

void ios_video_reencoder::set_listener(mws_sp<mws_video_reencoder_listener> i_listener)
{
   //p->video_reencoder_listener = i_listener;
}

#endif
