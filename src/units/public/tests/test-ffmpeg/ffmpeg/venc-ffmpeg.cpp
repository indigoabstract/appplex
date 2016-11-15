#include "stdafx.h"

#include "appplex-conf.hpp"

#ifdef MOD_FFMPEG

#include "venc-ffmpeg.hpp"
#include "pfm.hpp"


static void my_log_callback(void *ptr, int level, const char *fmt, va_list vargs)
{
	vprint("ffmpeg encoding error log %s", fmt);
}


venc_ffmpeg::venc_ffmpeg()
{
	codec = 0;
	c = 0;
	got_output = 0;
	f = 0;
	frame = 0;
	video_path = "";
	pts_idx = 0;
}

void venc_ffmpeg::startEncoding(const char* ivideo_path)
{
	AVCodecID codec_id = AV_CODEC_ID_H264;
	pts_idx = 0;
	video_path = std::string(ivideo_path);
	vprint("Encode video file %s\n", ivideo_path);
	//av_log_set_callback(my_log_callback);
	//av_log_set_level (AV_LOG_VERBOSE);

	// find the h264 video encoder
	codec = avcodec_find_encoder(codec_id);

	if (!codec)
	{
		vprint("Codec %d not found\n", codec_id);
		exit(1);
	}

	c = avcodec_alloc_context3(codec);

	if (!c)
	{
		vprint("Could not allocate video codec context\n");
		exit(1);
	}

	// params:

	//c->bit_rate = 400000;
	c->bit_rate = 0;//3000000;
	// resolution must be a multiple of two
	c->width = 480;
	c->height = 480;
	// frames per second
	AVRational fps = {1, 24};
	c->time_base = fps;
	// emit one intra frame every ten frames
	c->gop_size = 10;
	c->max_b_frames = 1;
	c->pix_fmt = AV_PIX_FMT_YUV420P;

	if (codec_id == AV_CODEC_ID_H264)
	{
		av_opt_set(c->priv_data, "preset", "ultrafast", 0);
		//av_opt_set(c->priv_data, "preset", "slow", 0);
	}

	// open codec
	int err = avcodec_open2(c, codec, NULL);

	if (err < 0)
	{
		vprint("Could not open codec %d\n", err);
		exit(1);
	}

	// open file for writing
	f = fopen(video_path.c_str(), "wb");

	if (!f)
	{
		vprint("Could not open %s\n", video_path.c_str());
		exit(1);
	}

	// allocate video frame
	frame = avcodec_alloc_frame();

	if (!frame)
	{
		vprint("Could not allocate video frame\n");
		exit(1);
	}

	frame->format = c->pix_fmt;
	frame->width = c->width;
	frame->height = c->height;

	// the image can be allocated by any means and av_image_alloc() is
	// just the most convenient way if av_malloc() is to be used
	int ret = av_image_alloc(frame->data, frame->linesize, c->width, c->height, c->pix_fmt, 32);

	if (ret < 0)
	{
		vprint("Could not allocate raw picture buffer\n");
		exit(1);
	}
}

void venc_ffmpeg::encodeFrame(const char* iframe_data, int iframe_data_length)
{
	// encode 1 frame of video
	int x, y, idx = 0;

	av_init_packet(&pkt);
	// packet data will be allocated by the encoder
	pkt.data = NULL;
	pkt.size = 0;

	fflush (stdout);
	// y channel
	for (y = 0; y < c->height; y++)
	{
		for (x = 0; x < c->width; x++)
		{
			frame->data[0][y * frame->linesize[0] + x] = iframe_data[idx];
			idx++;
		}
	}

	// uv channels
	for (y = 0; y < c->height / 2; y++)
	{
		for (x = 0; x < c->width / 2; x++)
		{
			frame->data[2][y * frame->linesize[2] + x] = iframe_data[idx];
			idx++;
			frame->data[1][y * frame->linesize[1] + x] = iframe_data[idx];
			idx++;
		}
	}

	frame->pts = pts_idx;

	// encode the image
	int ret = avcodec_encode_video2(c, &pkt, frame, &got_output);

	if (ret < 0)
	{
		vprint("Error encoding frame\n");
		exit(1);
	}

	if (got_output)
	{
		vprint("encodeFrame write frame %3d (size=%5d)\n", pts_idx, pkt.size);
		fwrite(pkt.data, 1, pkt.size, f);
		av_free_packet(&pkt);
	}

	pts_idx++;
}

void venc_ffmpeg::stopEncoding()
{
	static uint8_t endcode[] = {0, 0, 1, 0xb7};

	// get the delayed frames
	got_output = 1;

	for (int k = 0; got_output; k++)
	{
		fflush (stdout);

		int ret = avcodec_encode_video2(c, &pkt, NULL, &got_output);

		if (ret < 0)
		{
			vprint("Error encoding frame\n");
			exit(1);
		}

		if (got_output)
		{
			vprint("stopEncoding write frame %3d (size=%5d)\n", k, pkt.size);
			fwrite(pkt.data, 1, pkt.size, f);
			av_free_packet(&pkt);
		}
	}

	// add sequence end code to have a real mpeg file
	fwrite(endcode, 1, sizeof(endcode), f);
	fclose(f);

	avcodec_close(c);
	av_free(c);
	av_freep(&frame->data[0]);
	avcodec_free_frame(&frame);
}

#endif
