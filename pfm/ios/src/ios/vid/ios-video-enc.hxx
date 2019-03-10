#pragma once

#include "vid/video-enc.hxx"
#include <memory>
#include <string>


class ios_video_enc_impl;
class ios_video_reencoder;
class ios_video_reencoder_impl;


class ios_video_enc : public mws_video_enc
{
public:
	static std::shared_ptr<ios_video_enc> nwi();

	virtual ~ios_video_enc() {}
	mws_vid_enc_st get_state() const override;
    mws_vid_enc_method get_enc_method() const override;
	std::string get_video_path() override;
	void set_video_path(std::string i_video_path) override;
	void start_encoding(const mws_video_params& i_prm, mws_vid_enc_method i_enc_method) override;
	void encode_frame_m0_yuv420(const uint8* y_frame, const uint8* u_frame, const uint8* v_frame) override;
	void encode_frame_m1_yuv420(const char* iframe_data, int iframe_data_length) override;
	void encode_frame_m2_rbga(std::shared_ptr<gfx_tex> i_frame_tex) override;
	void stop_encoding() override;

protected:
	friend class ios_video_reencoder;
	ios_video_enc() {}

	std::shared_ptr<ios_video_enc_impl> p;
};


class ios_video_reencoder : public mws_video_reencoder
{
public:
	static std::shared_ptr<ios_video_reencoder> nwi();

	virtual ~ios_video_reencoder() {}
	virtual mws_vdec_state get_dec_state() const override;
	virtual mws_vid_enc_st get_enc_state() const override;
	std::string get_src_video_path() override;
	void set_src_video_path(std::string i_video_path) override;
	std::string get_dst_video_path() override;
	void set_dst_video_path(std::string i_video_path) override;
	void start_encoding(const mws_video_params& i_prm) override;
	void stop_encoding() override;
	void update() override;
	void set_listener(std::shared_ptr<mws_vdec_listener> i_listener) override;
    void set_reencode_listener(std::shared_ptr<mws_vreencoder_listener> i_listener) override;

protected:
	ios_video_reencoder() {}

private:
	std::shared_ptr<ios_video_reencoder_impl> p;
};
