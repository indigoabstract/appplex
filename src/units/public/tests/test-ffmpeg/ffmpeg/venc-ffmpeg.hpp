#pragma once

#include "appplex-conf.hpp"

#ifdef MOD_FFMPEG

extern "C"
{
	#define __STDC_CONSTANT_MACROS
	#ifdef _STDINT_H
	#undef _STDINT_H
	#endif
	#include <stdint.h>

	#include "libavcodec/avcodec.h"
	#include "libavdevice/avdevice.h"
	#include "libavutil/opt.h"
	#include "libavutil/imgutils.h"
}

#include <string>


class venc_ffmpeg
{
public:
	venc_ffmpeg();
	void startEncoding(const char* ivideo_path);
	void encodeFrame(const char* iframe_data, int iframe_data_length);
	void stopEncoding();

private:
	AVCodec* codec;
	AVCodecContext* c;
	int got_output;
	FILE* f;
	AVFrame* frame;
	AVPacket pkt;
	std::string video_path;
	int pts_idx;
};

#endif
