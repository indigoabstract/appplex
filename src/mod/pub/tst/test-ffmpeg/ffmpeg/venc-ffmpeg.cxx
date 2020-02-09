#include "stdafx.hxx"

#include "venc-ffmpeg.hxx"
#include "vdec-ffmpeg.hxx"
#include "pfm.hxx"
#include "min.hxx"
#include "gfx.hxx"
#include "gfx-quad-2d.hxx"
#include "gfx-tex.hxx"
#include "gfx-pbo.hxx"
#include "gfx-camera.hxx"
#include "gfx-rt.hxx"

#pragma comment (lib, "ffmpeg/avformat.lib")
#pragma comment(lib, "ffmpeg/avcodec.lib")
#pragma comment(lib, "ffmpeg/avutil.lib")


#define AV_CODEC_FLAG_GLOBAL_HEADER   (1 << 22)
#define STREAM_DURATION   10.0
#define STREAM_FRAME_RATE 25 /* 25 images/s */
#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */

void av_packet_rescale_ts(AVPacket *pkt, AVRational src_tb, AVRational dst_tb)
{
   if (pkt->pts != AV_NOPTS_VALUE)
      pkt->pts = av_rescale_q(pkt->pts, src_tb, dst_tb);
   if (pkt->dts != AV_NOPTS_VALUE)
      pkt->dts = av_rescale_q(pkt->dts, src_tb, dst_tb);
   if (pkt->duration > 0)
      pkt->duration = (int)av_rescale_q(pkt->duration, src_tb, dst_tb);
   if (pkt->convergence_duration > 0)
      pkt->convergence_duration = av_rescale_q(pkt->convergence_duration, src_tb, dst_tb);
}

std::string get_av_error_string(int av_error_code)
{
   char tbuf[AV_ERROR_MAX_STRING_SIZE] = { 0 };
   auto str = av_make_error_string(tbuf, AV_ERROR_MAX_STRING_SIZE, av_error_code);

   return std::string(str);
}


static void my_log_callback(void *ptr, int level, const char *fmt, va_list vargs)
{
   mws_print("ffmpeg encoding error log %s", fmt);
}


venc_ffmpeg::venc_ffmpeg()
{
   ortho_cam = gfx_camera::nwi();
   ortho_cam->projection_type = gfx_camera::e_orthographic_proj;
   state = mws_vid_enc_st::e_st_finished;
   fmt = nullptr;
   opt = nullptr;
   f = nullptr;
   frame = nullptr;
   oc = nullptr;
   video_path = "";
   pts_idx = 0;

   have_video = have_audio = false;
   encode_video = encode_audio = false;
   audio_codec = nullptr;
   video_codec = nullptr;

   trx("ffmpeg: avcodec_register_all - register all formats and codecs");
   av_register_all();
}


static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt)
{
   //AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

   //printf("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
   //   av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
   //   av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
   //   av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
   //   pkt->stream_index);
}

static int write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt)
{
   // rescale output packet timestamp values from codec to stream timebase
   av_packet_rescale_ts(pkt, *time_base, st->time_base);
   pkt->stream_index = st->index;

   // Write the compressed frame to the media file.
   log_packet(fmt_ctx, pkt);
   return av_interleaved_write_frame(fmt_ctx, pkt);
}

// Prepare a 16 bit dummy audio frame of 'frame_size' samples and 'nb_channels' channels.
static AVFrame *get_audio_frame(output_stream_ffmpeg *ost)
{
   AVFrame *frame = ost->tmp_frame;
   int j, i, v;
   int16_t *q = (int16_t*)frame->data[0];
   AVRational t = { 1, 1 };

   /* check if we want to generate more frames */
   if (av_compare_ts(ost->next_pts, ost->st->codec->time_base, (int64_t)STREAM_DURATION, t) >= 0)
   {
      return NULL;
   }

   for (j = 0; j < frame->nb_samples; j++)
   {
      v = (int)(sin(ost->t) * 10000);

      for (i = 0; i < ost->st->codec->channels; i++)
      {
         *q++ = v;
      }

      ost->t += ost->tincr;
      ost->tincr += ost->tincr2;
   }

   frame->pts = ost->next_pts;
   ost->next_pts += frame->nb_samples;

   return frame;
}

/*
* encode one audio frame and send it to the muxer
* return 1 when encoding is finished, 0 otherwise
*/
static int write_audio_frame(AVFormatContext *oc, output_stream_ffmpeg *ost)
{
   AVCodecContext *c;
   AVPacket pkt = { 0 }; // data and size must be 0;
   AVFrame *frame;
   int ret;
   int got_packet;
   int dst_nb_samples;

   av_init_packet(&pkt);
   c = ost->st->codec;

   frame = get_audio_frame(ost);

   if (frame)
   {
      /* convert samples from native format to destination codec format, using the resampler */
      /* compute destination number of samples */
      dst_nb_samples = (int)av_rescale_rnd(swr_get_delay(ost->swr_ctx, c->sample_rate) + frame->nb_samples,
         c->sample_rate, c->sample_rate, AV_ROUND_UP);
      mws_assert(dst_nb_samples == frame->nb_samples);

      /* when we pass a frame to the encoder, it may keep a reference to it
      * internally;
      * make sure we do not overwrite it here
      */
      ret = av_frame_make_writable(ost->frame);

      if (ret < 0)
      {
         exit(1);
      }

      /* convert to destination format */
      ret = swr_convert(ost->swr_ctx, ost->frame->data, dst_nb_samples, (const uint8_t **)frame->data, frame->nb_samples);

      if (ret < 0)
      {
         mws_print("Error while converting\n");
         exit(1);
      }

      frame = ost->frame;
      AVRational t = { 1, c->sample_rate };

      frame->pts = av_rescale_q(ost->samples_count, t, c->time_base);
      ost->samples_count += dst_nb_samples;
   }

   ret = avcodec_encode_audio2(c, &pkt, frame, &got_packet);

   if (ret < 0)
   {
      mws_print("Error encoding audio frame: %s\n", get_av_error_string(ret).c_str());
      exit(1);
   }

   if (got_packet)
   {
      ret = write_frame(oc, &c->time_base, ost->st, &pkt);

      if (ret < 0)
      {
         mws_print("Error while writing audio frame: %s\n", get_av_error_string(ret).c_str());
         exit(1);
      }
   }

   return (frame || got_packet) ? 0 : 1;
}

/*
* encode one video frame and send it to the muxer
* return 1 when encoding is finished, 0 otherwise
*/
static int write_video_frame(AVFormatContext *oc, output_stream_ffmpeg *ost, AVFrame* frame)
{
   int ret;
   AVCodecContext *c;
   int got_packet = 0;

   c = ost->st->codec;

   if (oc->oformat->flags & AVFMT_RAWPICTURE)
   {
      /* a hack to avoid data copy with some raw video muxers */
      AVPacket pkt;
      av_init_packet(&pkt);

      if (!frame)
      {
         return 1;
      }

      pkt.flags |= AV_PKT_FLAG_KEY;
      pkt.stream_index = ost->st->index;
      pkt.data = (uint8_t *)frame;
      pkt.size = sizeof(AVPicture);

      pkt.pts = pkt.dts = frame->pts;
      av_packet_rescale_ts(&pkt, c->time_base, ost->st->time_base);

      ret = av_interleaved_write_frame(oc, &pkt);
   }
   else
   {
      AVPacket pkt = { 0 };
      av_init_packet(&pkt);

      /* encode the image */
      ret = avcodec_encode_video2(c, &pkt, frame, &got_packet);

      if (ret < 0)
      {
         mws_print("Error encoding video frame: %s\n", get_av_error_string(ret).c_str());
         exit(1);
      }

      if (got_packet)
      {
         ret = write_frame(oc, &c->time_base, ost->st, &pkt);
      }
      else
      {
         ret = 0;
      }
   }

   if (ret < 0)
   {
      mws_print("Error while writing video frame: %s\n", get_av_error_string(ret).c_str());
      exit(1);
   }

   return (frame || got_packet) ? 0 : 1;
}

static void close_stream(AVFormatContext *oc, output_stream_ffmpeg *ost)
{
   avcodec_close(ost->st->codec);
   av_frame_free(&ost->frame);
   av_frame_free(&ost->tmp_frame);
   //sws_freeContext(ost->sws_ctx);
   //swr_free(&ost->swr_ctx);
}

mws_vid_enc_st venc_ffmpeg::get_state() const
{
   return state;
}

mws_vid_enc_method venc_ffmpeg::get_enc_method() const
{
   return enc_method;
}

mws_path venc_ffmpeg::get_video_path()
{
   return video_path;
}

void venc_ffmpeg::set_video_path(mws_path i_video_path)
{
   video_path = i_video_path;
}

void venc_ffmpeg::start_encoding(const mws_video_params& i_prm, mws_vid_enc_method i_enc_method)
{
   if (video_path.is_empty())
   {
      mws_throw mws_exception("venc-ffmpeg [video_path is empty]");
   }

   int ret = 0;

   enc_method = i_enc_method;
   params = std::make_shared<mws_video_params>();
   *params = i_prm;
   mws_print("Encode video file %s\n", video_path.string().c_str());
   pts_idx = 0;

   /* allocate the output media context */
   avformat_alloc_output_context2(&oc, NULL, NULL, video_path.string().c_str());

   if (!oc)
   {
      printf("Could not deduce output format from file extension: using mp4.\n");
      avformat_alloc_output_context2(&oc, NULL, "mp4", video_path.string().c_str());
   }

   if (!oc)
   {
      exit(1);
   }

   fmt = oc->oformat;

   /* Add the audio and video streams using the default format codecs
   * and initialize the codecs. */
   if (fmt->video_codec != AV_CODEC_ID_NONE)
   {
      add_stream(&video_st, oc, &video_codec, fmt->video_codec);
      have_video = true;
      encode_video = true;
   }

   if (fmt->audio_codec != AV_CODEC_ID_NONE)
   {
      //add_stream(&audio_st, oc, &audio_codec, fmt->audio_codec);
      //have_audio = true;
      //encode_audio = true;
   }

   /* Now that all the parameters are set, we can open the audio and
   * video codecs and allocate the necessary encode buffers. */
   if (have_video)
   {
      open_video(oc, video_codec, &video_st, opt);
   }

   if (have_audio)
   {
      open_audio(oc, audio_codec, &audio_st, opt);
   }

   av_dump_format(oc, 0, video_path.string().c_str(), 1);

   /* open the output file, if needed */
   if (!(fmt->flags & AVFMT_NOFILE))
   {
      ret = avio_open(&oc->pb, video_path.string().c_str(), AVIO_FLAG_WRITE);

      if (ret < 0)
      {
         mws_print("Could not open '%s': %s\n", video_path.string().c_str(), get_av_error_string(ret).c_str());
         exit(1);
      }
   }

   /* Write the stream header, if any. */
   ret = avformat_write_header(oc, &opt);

   if (ret < 0)
   {
      mws_print("Error occurred when opening output file: %s\n", get_av_error_string(ret).c_str());
      exit(1);
   }

   //while (encode_video || encode_audio) {
   //   /* select the stream to encode */
   //   if (encode_video &&
   //      (!encode_audio || av_compare_ts(video_st.next_pts, video_st.st->codec->time_base,
   //         audio_st.next_pts, audio_st.st->codec->time_base) <= 0)) {
   //      encode_video = !write_video_frame(oc, &video_st);
   //   }
   //   else {
   //      encode_audio = !write_audio_frame(oc, &audio_st);
   //   }
   //}

   if (enc_method == mws_vid_enc_method::e_enc_m2)
   {
      if (pbo_b_y)
      {
         pbo_b_y->readback->rewind();
         pbo_b_u->readback->rewind();
         pbo_b_v->readback->rewind();
      }
   }

   state = mws_vid_enc_st::e_st_encoding;
}

// encode 1 frame of video
void venc_ffmpeg::encode_frame_impl(AVFrame* i_frame)
{
   mws_assert(enc_method == mws_vid_enc_method::e_enc_m0);
   i_frame->pts = pts_idx;
   write_video_frame(oc, &video_st, i_frame);
   pts_idx++;
}

void venc_ffmpeg::encode_frame_m0_yuv420(const uint8* y_frame, const uint8* u_frame, const uint8* v_frame)
{
   mws_assert(enc_method == mws_vid_enc_method::e_enc_m0);
   encode_frame_m0_yuv420_impl(y_frame, u_frame, v_frame);
}

void venc_ffmpeg::encode_frame_m1_yuv420(const char* iframe_data, int iframe_data_length)
{
   mws_assert(enc_method == mws_vid_enc_method::e_enc_m1);
   encode_frame_m1_yuv420_impl(iframe_data, iframe_data_length);
}

void venc_ffmpeg::encode_frame_m2_rbga(mws_sp<gfx_tex> i_frame_tex)
{
   mws_assert(enc_method == mws_vid_enc_method::e_enc_m2);
   encode_frame_m2_rbga_impl(i_frame_tex);
}

void venc_ffmpeg::update()
{

}

static AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
{
   AVFrame *picture;
   int ret;

   picture = av_frame_alloc();

   if (!picture)
   {
      return NULL;
   }

   picture->format = pix_fmt;
   picture->width = width;
   picture->height = height;

   /* allocate the buffers for the frame data */
   ret = av_frame_get_buffer(picture, 32);

   if (ret < 0)
   {
      mws_print("Could not allocate frame data.\n");
      exit(1);
   }

   return picture;
}

static AVFrame* alloc_audio_frame(enum AVSampleFormat sample_fmt, uint64_t channel_layout, int sample_rate, int nb_samples)
{
   AVFrame *frame = av_frame_alloc();
   int ret;

   if (!frame)
   {
      mws_print("Error allocating an audio frame\n");
      exit(1);
   }

   frame->format = sample_fmt;
   frame->channel_layout = channel_layout;
   frame->sample_rate = sample_rate;
   frame->nb_samples = nb_samples;

   if (nb_samples)
   {
      ret = av_frame_get_buffer(frame, 0);

      if (ret < 0)
      {
         mws_print("Error allocating an audio buffer\n");
         exit(1);
      }
   }

   return frame;
}

void venc_ffmpeg::encode_frame_m0_yuv420_impl(const uint8* y_frame, const uint8* u_frame, const uint8* v_frame)
{
   uint8_t* p0 = &frame->data[0][0];
   uint8_t* p1 = &frame->data[0][1];
   uint8_t* p2 = &frame->data[0][2];

   // swap frame pointers to incoming data
   frame->data[0] = (uint8_t*)y_frame;
   frame->data[1] = (uint8_t*)u_frame;
   frame->data[2] = (uint8_t*)v_frame;

   frame->pts = pts_idx;
   write_video_frame(oc, &video_st, frame);
   pts_idx++;

   // swap frame pointers back to original data
   frame->data[0] = p0;
   frame->data[1] = p1;
   frame->data[2] = p2;
}

void venc_ffmpeg::encode_frame_m1_yuv420_impl(const char* iframe_data, int iframe_data_length)
{
   int video_height = video_st.st->codec->height;
   int video_width = video_st.st->codec->width;
   int idx = 0;

   // y channel
   for (int y = 0; y < video_height; y++)
   {
      for (int x = 0; x < video_width; x++)
      {
         frame->data[0][y * frame->linesize[0] + x] = iframe_data[idx];
         idx++;
      }
   }

   // uv channels
   for (int y = 0; y < video_height / 2; y++)
   {
      for (int x = 0; x < video_width / 2; x++)
      {
         frame->data[2][y * frame->linesize[2] + x] = iframe_data[idx];
         idx++;
         frame->data[1][y * frame->linesize[1] + x] = iframe_data[idx];
         idx++;
      }
   }

   frame->pts = pts_idx;
   write_video_frame(oc, &video_st, frame);
   pts_idx++;
}

void venc_ffmpeg::encode_frame_m2_rbga_impl(mws_sp<gfx_tex> i_tex)
{
   int width = i_tex->get_width();
   int height = i_tex->get_height();

   if (!pbo_b_y || (pbo_b_y->rt_tex->get_width() != width) || (pbo_b_y->rt_tex->get_height() != height))
   {
      auto gi = i_tex->gi();
      {
         pbo_b_y = mws_sp<mws_pbo_bundle>(new mws_pbo_bundle(gi, width, height, "R8"));
         pbo_b_y->readback->set_read_method(mws_read_method::e_map_buff_pixels_buff);
         auto handler = [this](const gfx_readback* i_rb, gfx_ubyte* i_data, int i_size)
         {
            //memcpy(i_data, y_pbo_pixels.data(), i_size);
            encode_frame_m0_yuv420_impl(i_data, pbo_b_u->readback->get_pbo_pixels().data(), pbo_b_v->readback->get_pbo_pixels().data());
            //mws_print("recv y data\n");
         };
         pbo_b_y->set_on_data_recv_handler(handler);
         //pbo_b_y->rt_quad->set_v_flip(true);
         //pbo_b_y->rt_quad->set_h_flip(true);
         (*pbo_b_y->rt_quad)[MP_SHADER_NAME][MP_VSH_NAME] = "conv-rgb-2-yuv-420.vsh";
         (*pbo_b_y->rt_quad)[MP_SHADER_NAME][MP_FSH_NAME] = "conv-rgb-2-y-420.fsh";
      }
      {
         pbo_b_u = mws_sp<mws_pbo_bundle>(new mws_pbo_bundle(gi, width / 2, height / 2, "R8"));
         pbo_b_u->readback->set_read_method(mws_read_method::e_map_buff_pixels_buff);
         auto handler = [this](const gfx_readback* i_rb, gfx_ubyte* i_data, int i_size)
         {
            memcpy((void*)i_rb->get_pbo_pixels().data(), i_data, i_size);
            //mws_print("recv u data\n");
         };
         pbo_b_u->set_on_data_recv_handler(handler);
         //pbo_b_u->rt_quad->set_v_flip(true);
         //pbo_b_u->rt_quad->set_h_flip(true);
         (*pbo_b_u->rt_quad)[MP_SHADER_NAME][MP_VSH_NAME] = "conv-rgb-2-yuv-420.vsh";
         (*pbo_b_u->rt_quad)[MP_SHADER_NAME][MP_FSH_NAME] = "conv-rgb-2-u-420.fsh";
      }
      {
         pbo_b_v = mws_sp<mws_pbo_bundle>(new mws_pbo_bundle(gi, width / 2, height / 2, "R8"));
         pbo_b_v->readback->set_read_method(mws_read_method::e_map_buff_pixels_buff);
         auto handler = [this](const gfx_readback* i_rb, gfx_ubyte* i_data, int i_size)
         {
            memcpy((void*)i_rb->get_pbo_pixels().data(), i_data, i_size);
            //mws_print("recv v data\n");
         };
         pbo_b_v->set_on_data_recv_handler(handler);
         //pbo_b_v->rt_quad->set_v_flip(true);
         //pbo_b_v->rt_quad->set_h_flip(true);
         (*pbo_b_v->rt_quad)[MP_SHADER_NAME][MP_VSH_NAME] = "conv-rgb-2-yuv-420.vsh";
         (*pbo_b_v->rt_quad)[MP_SHADER_NAME][MP_FSH_NAME] = "conv-rgb-2-v-420.fsh";
      }
   }

   pbo_b_u->set_tex(i_tex);
   pbo_b_u->update(ortho_cam);

   pbo_b_v->set_tex(i_tex);
   pbo_b_v->update(ortho_cam);

   // must be last
   pbo_b_y->set_tex(i_tex);
   pbo_b_y->update(ortho_cam);

   //gfx_util::draw_tex(ortho_cam, i_tex, 10, 300);
}

void venc_ffmpeg::stop_encoding()
{
   /* Write the trailer, if any. The trailer must be written before you
   * close the CodecContexts open when you wrote the header; otherwise
   * av_write_trailer() may try_ to use memory that was freed on
   * av_codec_close(). */
   av_write_trailer(oc);

   /* Close each codec. */
   if (have_video)
   {
      close_stream(oc, &video_st);
   }

   if (have_audio)
   {
      close_stream(oc, &audio_st);
   }

   if (!(fmt->flags & AVFMT_NOFILE))
   {
      /* Close the output file. */
      avio_close(oc->pb);
   }

   /* free the stream */
   avformat_free_context(oc);
   state = mws_vid_enc_st::e_st_finished;
}

void venc_ffmpeg::open_audio(AVFormatContext *oc, AVCodec *codec, output_stream_ffmpeg *ost, AVDictionary *opt_arg)
{
   AVCodecContext *c;
   int nb_samples;
   int ret;
   AVDictionary *opt = NULL;

   c = ost->st->codec;

   /* open it */
   av_dict_copy(&opt, opt_arg, 0);
   ret = avcodec_open2(c, codec, &opt);
   av_dict_free(&opt);

   if (ret < 0)
   {
      mws_print("Could not open audio codec: %s\n", get_av_error_string(ret).c_str());
      exit(1);
   }

   /* init signal generator */
   ost->t = 0;
   ost->tincr = 2 * (float)M_PI * 110.f / c->sample_rate;
   /* increment frequency by 110 Hz per second */
   ost->tincr2 = 2 * (float)M_PI * 110.f / c->sample_rate / c->sample_rate;

   if (c->codec->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE)
   {
      nb_samples = 10000;
   }
   else
   {
      nb_samples = c->frame_size;
   }

   ost->frame = alloc_audio_frame(c->sample_fmt, c->channel_layout, c->sample_rate, nb_samples);
   ost->tmp_frame = alloc_audio_frame(AV_SAMPLE_FMT_S16, c->channel_layout, c->sample_rate, nb_samples);

   /* create resampler context */
   //ost->swr_ctx = swr_alloc();
   //if (!ost->swr_ctx) {
   //   mws_print("Could not allocate resampler context\n");
   //   exit(1);
   //}

   /* set options */
   av_opt_set_int(ost->swr_ctx, "in_channel_count", c->channels, 0);
   av_opt_set_int(ost->swr_ctx, "in_sample_rate", c->sample_rate, 0);
   av_opt_set_sample_fmt(ost->swr_ctx, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
   av_opt_set_int(ost->swr_ctx, "out_channel_count", c->channels, 0);
   av_opt_set_int(ost->swr_ctx, "out_sample_rate", c->sample_rate, 0);
   av_opt_set_sample_fmt(ost->swr_ctx, "out_sample_fmt", c->sample_fmt, 0);

   /* initialize the resampling context */
   //if ((ret = swr_init(ost->swr_ctx)) < 0) {
   //   mws_print("Failed to initialize the resampling context\n");
   //   exit(1);
   //}
}

void venc_ffmpeg::open_video(AVFormatContext *oc, AVCodec *codec, output_stream_ffmpeg *ost, AVDictionary *opt_arg)
{
   int ret;
   AVCodecContext *c = ost->st->codec;
   AVDictionary *opt = NULL;

   av_dict_copy(&opt, opt_arg, 0);

   if (codec->id == AV_CODEC_ID_H264)
   {
      /*
      Constant Rate Factor (CRF): Use this mode if you want to keep the best quality and don't care about the file size.
      This method allows the encoder to attempt to achieve a certain output quality for the whole file when output file size is of less importance.
      This provides maximum compression efficiency with a single pass. By adjusting the so-called quantizer for each frame, it gets the bitrate it needs to keep the requested quality level.
      The downside is that you can't tell it to get a specific filesize or not go over a specific size or bitrate, which means that this method is not recommended for encoding videos for streaming.
      Choose a CRF value: The range of the CRF scale is 0-51, where 0 is lossless, 23 is the default, and 51 is worst quality possible.
      A lower value generally leads to higher quality, and a subjectively sane range is 17-28.
      Consider 17 or 18 to be visually lossless or nearly so; it should look the same or nearly the same as the input but it isn't technically lossless.
      The range is exponential, so increasing the CRF value +6 results in roughly half the bitrate / file size, while -6 leads to roughly twice the bitrate.
      Choose the highest CRF value that still provides an acceptable quality. If the output looks good, then try_ a higher value. If it looks bad, choose a lower value.

      A preset is a collection of options that will provide a certain encoding speed to compression ratio.
      A slower preset will provide better compression(compression is quality per filesize).
      This means that, for example, if you target a certain file size or constant bit rate, you will achieve better quality with a slower preset.
      Similarly, for constant quality encoding, you will simply save bitrate by choosing a slower preset.
      Use the slowest preset that you have patience for.The available presets in descending order of speed are :
      ultrafast, superfast, veryfast, faster, fast, medium – default preset, slow, slower, veryslow

      You can optionally use -tune to change settings based upon the specifics of your input. Current tunings include:
      film – use for high quality movie content; lowers deblocking
      animation – good for cartoons; uses higher deblocking and more reference frames
      grain – preserves the grain structure in old, grainy film material
      stillimage – good for slideshow-like content
      fastdecode – allows faster decoding by disabling certain filters
      zerolatency – good for fast encoding and low-latency streaming
      psnr – ignore this as it is only used for codec development
      ssim – ignore this as it is only used for codec development
      For example, if your input is animation then use the animation tuning, or if you want to preserve grain in a film then use the grain tuning.
      If you are unsure of what to use or your input does not match any of tunings then omit the -tune option.
      You can see a list of current tunings with -tune help, and what settings they apply with x264 --fullhelp.
      */

      auto ctx = ost->st->codec->priv_data;

      av_opt_set(ctx, "preset", params->preset.c_str(), 0);
      av_opt_set(ctx, "tune", params->tune.c_str(), 0);
      av_opt_set_int(ctx, "crf", params->crf, 0);
   }

   /* open the codec */
   ret = avcodec_open2(c, codec, &opt);
   av_dict_free(&opt);

   if (ret < 0)
   {
      char 	errbuf[1000];
      int x = av_strerror(ret, errbuf, 1000);
      mws_print("Could not open video codec: %s\n%s\n", errbuf, get_av_error_string(ret).c_str());
      exit(1);
   }

   /* allocate and init a re-usable frame */
   ost->frame = alloc_picture(c->pix_fmt, c->width, c->height);

   if (!ost->frame)
   {
      mws_print("Could not allocate video frame\n");
      exit(1);
   }

   /* If the output format is not YUV420P, then a temporary YUV420P
   * picture is needed too. It is then converted to the required
   * output format. */
   ost->tmp_frame = NULL;

   if (c->pix_fmt != AV_PIX_FMT_YUV420P)
   {
      ost->tmp_frame = alloc_picture(AV_PIX_FMT_YUV420P, c->width, c->height);

      if (!ost->tmp_frame)
      {
         mws_print("Could not allocate temporary picture\n");
         exit(1);
      }
   }

   // allocate video frame
   frame = av_frame_alloc();

   if (!frame)
   {
      mws_print("Could not allocate video frame\n");
      exit(1);
   }

   frame->format = c->pix_fmt;
   frame->width = c->width;
   frame->height = c->height;

   // the image can be allocated by any means and av_image_alloc() is
   // just the most convenient way if av_malloc() is to be used
   ret = av_image_alloc(frame->data, frame->linesize, c->width, c->height, c->pix_fmt, 32);
   mws_assert(frame->linesize[0] == c->width);

   if (ret < 0)
   {
      mws_print("Could not allocate raw picture buffer\n");
      exit(1);
   }
}

/* Add an output stream. */
void venc_ffmpeg::add_stream(output_stream_ffmpeg *ost, AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id)
{
   AVCodecContext *c;
   int i;

   /* find the encoder */
   *codec = avcodec_find_encoder(codec_id);
   if (!(*codec))
   {
      mws_print("Could not find encoder for '%s'\n", avcodec_get_name(codec_id));
      exit(1);
   }

   ost->st = avformat_new_stream(oc, NULL);

   if (!ost->st)
   {
      mws_print("Could not allocate stream\n");
      exit(1);
   }

   ost->st->id = oc->nb_streams - 1;
   c = avcodec_alloc_context3(*codec);

   if (!c)
   {
      mws_print("Could not alloc an encoding context\n");
      exit(1);
   }

   ost->st->codec = c;

   switch ((*codec)->type)
   {
   case AVMEDIA_TYPE_AUDIO:
   {
      c->sample_fmt = (*codec)->sample_fmts ? (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
      c->bit_rate = 64000;
      c->sample_rate = 44100;

      if ((*codec)->supported_samplerates)
      {
         c->sample_rate = (*codec)->supported_samplerates[0];

         for (i = 0; (*codec)->supported_samplerates[i]; i++)
         {
            if ((*codec)->supported_samplerates[i] == 44100)
            {
               c->sample_rate = 44100;
            }
         }
      }
      c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
      c->channel_layout = AV_CH_LAYOUT_STEREO;

      if ((*codec)->channel_layouts)
      {
         c->channel_layout = (*codec)->channel_layouts[0];

         for (i = 0; (*codec)->channel_layouts[i]; i++)
         {
            if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
            {
               c->channel_layout = AV_CH_LAYOUT_STEREO;
            }
         }
      }

      c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
      AVRational t = { 1, c->sample_rate };
      ost->st->time_base = t;

      break;
   }

   case AVMEDIA_TYPE_VIDEO:
   {
      c->codec_id = (AVCodecID)params->codec_id;

      c->bit_rate = params->bit_rate;
      //c->bit_rate = 800 * 1000;
      /* Resolution must be a multiple of two. */
      c->width = params->width;
      c->height = params->height;
      /* timebase: This is the fundamental unit of time (in seconds) in terms
      * of which frame timestamps are represented. For fixed-fps content,
      * timebase should be 1/framerate and timestamp increments should be
      * identical to 1. */
      //AVRational t = { 1, STREAM_FRAME_RATE };
      ost->st->time_base.num = params->time_base_numerator;
      ost->st->time_base.den = params->time_base_denominator;
      c->time_base.num = params->time_base_numerator * 2;
      c->time_base.den = params->time_base_denominator;
      c->ticks_per_frame = params->ticks_per_frame;

      c->gop_size = params->gop_size; /* emit one intra frame every twelve frames at most */
      c->pix_fmt = (AVPixelFormat)params->pix_fmt;
      c->max_b_frames = params->max_b_frames;

      if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO)
      {
         /* just for testing, we also add B-frames */
         c->max_b_frames = 2;
      }
      else if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO)
      {
         /* Needed to avoid using macroblocks in which some coeffs overflow.
         * This does not happen with normal video, it just happens here as
         * the motion of the chroma plane does not match the luma plane. */
         c->mb_decision = 2;
      }
      break;
   }

   default:
      break;
   }


   // Some formats want stream headers to be separate
   if (oc->oformat->flags & AVFMT_GLOBALHEADER)
   {
      c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
   }
}


class mws_ffmpeg_vdec_listener;


class mws_ffmpeg_reencoder_impl
{
public:
   mws_ffmpeg_reencoder_impl();
   void start_encoding(const mws_video_params& i_prm);
   void on_frame_decoded(mws_sp<gfx_tex> i_video_frame);
   void update();

   mws_sp<vdec_ffmpeg> vdec;
   mws_sp<venc_ffmpeg> venc;
   // use this class to listen to the FFMPEG video decoder events
   mws_sp<mws_ffmpeg_vdec_listener> vdec_listener;
   // use this class to listen to the FFMPEG video reencoder events
   mws_sp<mws_vdec_listener> vreencoder_evt_listener;
   // use this class to listen to reencode frame events
   mws_sp<mws_vreencoder_listener> rencode_frame_listener;
   mws_video_params recv_params;
   // video frame rt
   mws_sp<gfx_rt> rt;
   mws_sp<gfx_tex> rt_tex;
   mws_sp<gfx_camera> rt_cam;
   mws_sp<gfx_quad_2d> rt_video_quad;
   mws_sp<gfx_tex> rt_video_frame;
};


// use this class to listen to the FFMPEG video decoder events
class mws_ffmpeg_vdec_listener : public mws_vdec_listener
{
public:
   void on_start(mws_sp<mws_video_params> i_params) override
   {
      mws_assert(i_params->width > 0 && i_params->height > 0);
      auto reenc = reenc_impl.lock();
      mws_assert(reenc != nullptr);
      mws_print("on_decoding_started\n");

      // if rencode_frame_listener is not null, use the async gpu readback method, else use the direct FFMPMEG method
      {
         mws_video_params prm = reenc->recv_params;
         mws_vid_enc_method enc_method = (reenc->rencode_frame_listener) ? mws_vid_enc_method::e_enc_m2 : mws_vid_enc_method::e_enc_m0;

         prm.time_base_numerator = i_params->time_base_numerator;
         prm.time_base_denominator = i_params->time_base_denominator;
         prm.ticks_per_frame = i_params->ticks_per_frame;
         reenc->venc->start_encoding(prm, enc_method);
      }

      if (reenc->vreencoder_evt_listener)
      {
         reenc->vreencoder_evt_listener->on_start(i_params);
      }
   }

   void on_progress_evt(float i_progress_percent) override
   {
      auto reenc = reenc_impl.lock();
      mws_assert(reenc != nullptr);
      //mws_print("on_progress_evt [%f]\n", i_progress_percent);

      if (reenc->vreencoder_evt_listener)
      {
         reenc->vreencoder_evt_listener->on_progress_evt(i_progress_percent);
      }
   }

   void on_frame_decoded(void* i_frame) override
   {
      auto reenc = reenc_impl.lock();
      mws_assert(reenc != nullptr);

      // if video_reencoder_listener is null, then no gfx processing is done to the frame. use the direct route to encode frames
      if (!reenc->rencode_frame_listener)
      {
         reenc->venc->encode_frame_impl((AVFrame*)i_frame);
         mws_print("encode frame AVFrame\n!");
      }
   }

   void on_frame_decoded(mws_sp<gfx_tex> i_video_frame) override
   {
      auto reenc = reenc_impl.lock();
      mws_assert(reenc != nullptr);

      if (reenc->vreencoder_evt_listener)
      {
         reenc->vreencoder_evt_listener->on_frame_decoded(i_video_frame);

         // if video_reencoder_listener is not null, do some gfx processing to the frame and then read pixels back and feed them to the FFMPEG encoder
         if (reenc->rencode_frame_listener)
         {
            reenc->on_frame_decoded(i_video_frame);
            //mws_print("encode frame i_frame_tex\n");
         }
      }
   }

   void on_stop() override
   {
      auto reenc = reenc_impl.lock();
      mws_assert(reenc != nullptr);
      mws_print("on_decoding_stopped\n");
      reenc->venc->stop_encoding();

      if (reenc->vreencoder_evt_listener)
      {
         reenc->vreencoder_evt_listener->on_stop();
      }
   }

   void on_finish() override
   {
      auto reenc = reenc_impl.lock();
      mws_assert(reenc != nullptr);
      mws_print("on_decoding_finished\n");
      reenc->venc->stop_encoding();

      if (reenc->vreencoder_evt_listener)
      {
         reenc->vreencoder_evt_listener->on_finish();
      }
   }

   mws_wp<mws_ffmpeg_reencoder_impl> reenc_impl;
};


mws_ffmpeg_reencoder_impl::mws_ffmpeg_reencoder_impl()
{
   vdec = mws_sp<vdec_ffmpeg>(new vdec_ffmpeg());
   venc = mws_sp<venc_ffmpeg>(new venc_ffmpeg());
   vdec_listener = mws_sp<mws_ffmpeg_vdec_listener>(new mws_ffmpeg_vdec_listener());
   vdec->set_listener(vdec_listener);
}

void mws_ffmpeg_reencoder_impl::start_encoding(const mws_video_params& i_prm)
{
   recv_params = i_prm;

   if (rencode_frame_listener)
   {
      int width = i_prm.width;
      int height = i_prm.height;

      if (!rt_tex || (rt_tex->get_width() != width) || (rt_tex->get_height() != height))
      {
         auto gi = gfx::i();
         gfx_tex_params prm;

         prm.set_rt_params();
         rt_tex = gi->tex.nwi("vid-reenc-" + gfx_tex::gen_id(), width, height, &prm);
         rt = gi->rt.new_rt();
         rt->set_color_attachment(rt_tex);

         rt_cam = gfx_camera::nwi(gi);
         rt_cam->projection_type = gfx_camera::e_orthographic_proj;
         rt_cam->clear_color_value = gfx_color::colors::dark_orange;
         rt_cam->clear_color = true;

         {
            rt_video_quad = gfx_quad_2d::nwi();
            auto& msh = *rt_video_quad;

            rt_video_frame = gi->tex.nwi_external("vid-frame" + gfx_tex::gen_id(), 0, "RGBA8");
            msh.set_dimensions(1.f, 1.f);
            msh.set_scale((float)width, (float)height);
            msh[MP_SHADER_NAME] = gfx::basic_tex_sh_id;
            msh["u_s2d_tex"][MP_TEXTURE_INST] = rt_video_frame;
            msh[MP_CULL_BACK] = false;
         }
      }
   }

   vdec->start_decoding();
   //venc->start_encoding(i_prm, enc_method);
}

void mws_ffmpeg_reencoder_impl::on_frame_decoded(mws_sp<gfx_tex> i_video_frame)
{
   bool use_rt_video_frame = rencode_frame_listener->on_reencode_frame(rt, i_video_frame);

   // render the source video into the final frame
   if (!use_rt_video_frame)
   {
      rt_video_frame->set_texture_gl_id(i_video_frame->get_texture_gl_id());
      gfx::i()->rt.set_current_render_target(rt);
      rt_cam->clear_buffers();
      gfx::i()->rt.set_current_render_target();
   }

   venc->encode_frame_m2_rbga(rt_tex);
}

void mws_ffmpeg_reencoder_impl::update()
{
   vdec->update();
   venc->update();
}


mws_sp<mws_ffmpeg_reencoder> mws_ffmpeg_reencoder::nwi()
{
   auto p = mws_sp<mws_ffmpeg_reencoder_impl>(new mws_ffmpeg_reencoder_impl());
   auto r = mws_sp<mws_ffmpeg_reencoder>(new mws_ffmpeg_reencoder());
   r->p = p;
   p->vdec_listener->reenc_impl = p;

   return r;
}

mws_vdec_state mws_ffmpeg_reencoder::get_dec_state() const
{
   return p->vdec->get_state();
}

mws_vid_enc_st mws_ffmpeg_reencoder::get_enc_state() const
{
   return p->venc->get_state();
}

mws_path mws_ffmpeg_reencoder::get_src_video_path()
{
   return p->vdec->get_video_path();
}

void mws_ffmpeg_reencoder::set_src_video_path(mws_path i_video_path)
{
   p->vdec->set_video_path(i_video_path);
}

mws_path mws_ffmpeg_reencoder::get_dst_video_path()
{
   return p->venc->get_video_path();
}

void mws_ffmpeg_reencoder::set_dst_video_path(mws_path i_video_path)
{
   p->venc->set_video_path(i_video_path);
}

void mws_ffmpeg_reencoder::start_encoding(const mws_video_params& i_prm)
{
   p->start_encoding(i_prm);
}

void mws_ffmpeg_reencoder::stop_encoding()
{
   p->vdec->stop();
}

void mws_ffmpeg_reencoder::update()
{
   p->update();
}

void mws_ffmpeg_reencoder::set_listener(mws_sp<mws_vdec_listener> i_listener)
{
   p->vreencoder_evt_listener = i_listener;
}

void mws_ffmpeg_reencoder::set_reencode_listener(mws_sp<mws_vreencoder_listener> i_listener)
{
   p->rencode_frame_listener = i_listener;
}
