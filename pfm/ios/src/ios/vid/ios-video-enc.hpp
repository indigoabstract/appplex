#pragma once

#include "media/vid/video-enc.hpp"
#include <memory>
#include <string>


class ios_video_enc_impl;
class ios_reencoder_impl;


class ios_video_enc : public mws_video_enc
{
public:
	static std::shared_ptr<ios_video_enc> nwi();

	virtual ~ios_video_enc() {}
	mws_vid_enc_st get_state() const override;
	std::string get_video_path() override;
	void set_video_path(std::string i_video_path) override;
	void start_encoding(std::shared_ptr<gfx> i_gi, const mws_video_params& i_prm, mws_vid_enc_method i_enc_method) override;
	void encode_frame_m0_yuv420(const uint8* y_frame, const uint8* u_frame, const uint8* v_frame) override;
	void encode_frame_m1_yuv420(const char* iframe_data, int iframe_data_length) override;
	void encode_frame_m2_rbga(std::shared_ptr<gfx> i_gi, std::shared_ptr<gfx_tex> i_frame_tex) override;
	void stop_encoding() override;

protected:
	ios_video_enc() {}

	std::shared_ptr<ios_video_enc_impl> p;
};


class ios_reencoder : public mws_video_reencoder
{
public:
	static std::shared_ptr<ios_reencoder> nwi();

	virtual ~ios_reencoder() {}
	virtual mws_vdec_state get_dec_state() const override;
	virtual mws_vid_enc_st get_enc_state() const override;
	std::string get_src_video_path() override;
	void set_src_video_path(std::string i_video_path) override;
	std::string get_dst_video_path() override;
	void set_dst_video_path(std::string i_video_path) override;
	void start_encoding(const mws_video_params& i_prm) override;
	void stop_encoding() override;
	void update() override;
	void set_listener(mws_sp<mws_video_reencoder_listener> i_listener) override;

protected:
	ios_reencoder() {}

private:
	std::shared_ptr<ios_reencoder_impl> p;
};
