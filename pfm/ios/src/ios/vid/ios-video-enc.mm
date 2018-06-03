#include "stdafx.h"

#include "appplex-conf.hpp"
#include "ios-video-enc.hpp"
#include "ios-video-dec.hpp"
#include "pfm.hpp"
#include "min.hpp"


#if defined PLATFORM_IOS

#import "ios/vid/dec/video-player.h"
#import "ios/vid/enc/GPUImageFilter.h"
#import "ios/vid/enc/GPUImageMovie.h"
#import "ios/vid/enc/GPUImageOutput.h"
#import "ios/vid/enc/GPUImageMovieWriter.h"
#import "ios/vid/enc/GPUImageView.h"
#include "gfx-inc.hpp"
#include <unistd.h>


class ios_video_enc_impl;
class ios_video_reencoder_impl;


static mws_wp<ios_video_reencoder_impl> global_vreencoder_impl;
// GPUImageMovie
static GPUImageView* gpu_image_view;
static GPUImageMovie* movieFile;
//static GPUImageOutput<GPUImageInput>* filter;
static GPUImageMovieWriter* movieWriter;
static NSTimer* timer;


void video_enc_progress_handler(mws_sp<ios_video_enc_impl> i_venc_impl, int i_progress_0_2_100);
void video_enc_finished_handler(mws_sp<ios_video_enc_impl> i_venc_impl, std::string i_new_video_path);


@interface video_encoder_helper : NSObject
{
}
@end

@interface video_encoder_helper ()
{
	@public mws_wp<ios_video_enc_impl> venc_impl;
    NSURL* src_video_url;
    NSURL* dst_video_url;
}

@end

@implementation video_encoder_helper

- (void)retrieving_progress
{
    int progress_0_2_100 = (int)(movieFile.progress * 100);
	video_enc_progress_handler(venc_impl.lock(), progress_0_2_100);
    //mws_print("progress: %d\n", progress_0_2_100);
}

-(void)encode_video:(NSString*) src_path dst_path:(NSString*) dst_path
{
    NSLog(@"\n\nenc src %@ dst %@\n\n", src_path, dst_path);
    NSString* full_src_path = [video_file_util getQualifiedFilenameOrResource:src_path];
    src_video_url = [NSURL fileURLWithPath:full_src_path];
    
    movieFile = [[GPUImageMovie alloc] initWithURL:src_video_url];
    movieFile.runBenchmark = NO;
    movieFile.playAtActualSpeed = NO;
    //filter = [[GPUImageFilter alloc] init];
    //filter = [[GPUImageUnsharpMaskFilter alloc] init];
    
    //[movieFile addTarget:filter];
    
    // Only rotate the video for display, leave orientation the same for recording
    //GPUImageView *filterView = (GPUImageView *)self.view;
    
    CGRect screen_rect = [[UIScreen mainScreen] bounds];
    CGFloat screen_width = screen_rect.size.width;
    CGFloat screen_height = screen_rect.size.height;
    
    if(!gpu_image_view)
    {
        gpu_image_view = [[GPUImageView alloc] initWithFrame:CGRectMake(0.0, 0.0, screen_width, screen_height)];
    }
    
    [movieFile addTarget:gpu_image_view];
    
    // In addition to displaying to the screen, write out a processed version of the movie to disk
    unlink([dst_path UTF8String]); // If a file already exists, AVAssetWriter won't let you record new frames, so delete the old movie
    dst_video_url = [NSURL fileURLWithPath:dst_path];
    
    movieWriter = [[GPUImageMovieWriter alloc] initWithMovieURL:dst_video_url size:CGSizeMake(screen_width, screen_height)];
    [movieFile addTarget:movieWriter];
    
    // Configure this for video from the movie file, where we want to preserve all video frames and audio samples
    movieWriter.shouldPassthroughAudio = YES;
    movieFile.audioEncodingTarget = movieWriter;
    [movieFile enableSynchronizedEncodingUsingMovieWriter:movieWriter];
    
    [movieWriter startRecording];
    [movieFile startProcessing];
    
    timer = [NSTimer scheduledTimerWithTimeInterval:0.3f
                                             target:self
                                           selector:@selector(retrieving_progress)
                                           userInfo:nil
                                            repeats:YES];
    
    [movieWriter setCompletionBlock:^{
        [movieFile removeTarget:movieWriter];
        [movieWriter finishRecording];
        
        dispatch_async(dispatch_get_main_queue(), ^{
            [timer invalidate];
            mws_print("\n\nencoding finished\n\n");
            
            if (UIVideoAtPathIsCompatibleWithSavedPhotosAlbum(dst_path))
            {
                UISaveVideoAtPathToSavedPhotosAlbum(dst_path, nil, nil, nil);
            }
            
			auto dst_path_c = [dst_path UTF8String];
			std::string new_video_path = (dst_path_c) ? dst_path_c : "";
			
			video_enc_finished_handler(venc_impl.lock(), new_video_path);
        });
    }];
}

@end


class ios_video_enc_impl
{
public:
	ios_video_enc_impl()
	{
		venc_helper_inst = [video_encoder_helper alloc];
	}
	
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

	void start_encoding(const mws_video_params& i_prm, mws_vid_enc_method i_enc_method)
	{
		mws_assert(!src_video_path.empty());
		mws_assert(!video_path.empty());
		state = mws_vid_enc_st::e_st_encoding;
		enc_method = i_enc_method;
		
		// if a file with that name already exists, delete the old movie
		unlink(video_path.c_str());
		
		NSString* src_path_nss = [[NSString alloc] initWithUTF8String:src_video_path.c_str()];
		NSString* dst_path_nss = [[NSString alloc] initWithUTF8String:video_path.c_str()];
		mws_print("\n\nencode_selected_videoooooooo : [%s] [%s] \n\n\n", src_video_path.c_str(), video_path.c_str());
		[venc_helper_inst encode_video:src_path_nss dst_path:dst_path_nss];
	}

	void encode_frame_m0_yuv420(const uint8* y_frame, const uint8* u_frame, const uint8* v_frame)
	{
		mws_throw ia_exception("n/a");
	}
	
	void encode_frame_m1_yuv420(const char* iframe_data, int iframe_data_length)
	{
		mws_throw ia_exception("n/a");
	}
	
	void encode_frame_m2_rbga(std::shared_ptr<gfx_tex> i_frame_tex)
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
    video_encoder_helper* venc_helper_inst;
    std::string src_video_path;
	mws_wp<ios_video_reencoder_impl> reencoder_impl;
};


std::shared_ptr<ios_video_enc> ios_video_enc::nwi()
{
	auto i = std::shared_ptr<ios_video_enc>(new ios_video_enc());
	i->p = std::shared_ptr<ios_video_enc_impl>(new ios_video_enc_impl());
	i->p->venc_helper_inst->venc_impl = i->p;
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

void ios_video_enc::start_encoding(const mws_video_params& i_prm, mws_vid_enc_method i_enc_method)
{
	p->start_encoding(i_prm, i_enc_method);
}

void ios_video_enc::encode_frame_m0_yuv420(const uint8* y_frame, const uint8* u_frame, const uint8* v_frame)
{
	p->encode_frame_m0_yuv420(y_frame, u_frame, v_frame);
}

void ios_video_enc::encode_frame_m1_yuv420(const char* iframe_data, int iframe_data_length)
{
	p->encode_frame_m1_yuv420(iframe_data, iframe_data_length);
}

void ios_video_enc::encode_frame_m2_rbga(std::shared_ptr<gfx_tex> i_frame_tex)
{
	p->encode_frame_m2_rbga(i_frame_tex);
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
   
    void start_encoding(const mws_video_params& i_prm)
    {
        recv_params = std::make_shared<mws_video_params>();
        *recv_params = i_prm;

        vdec_state = mws_vdec_state::st_playing;
        venc->start_encoding(*recv_params, mws_vid_enc_method::e_enc_m2);
        
        if(vdec_listener)
        {
            vdec_listener->on_start(recv_params);
        }
    }
    
    void encode_video_frame(int tex_gl_id)
    {
        bool use_rt_video_frame = false;
        
        create_rt();
        rt_video_frame->set_texture_gl_id(tex_gl_id);

        if(video_reencoder_listener)
        {
            use_rt_video_frame = video_reencoder_listener->on_reencode_frame(rt, rt_video_frame);
        }
        
        // render the source video into the final frame
        if (!use_rt_video_frame)
        {
            gfx::i()->rt.set_current_render_target(rt);
            rt_cam->clear_buffers();
            gfx::i()->rt.set_current_render_target();
        }
    }
    
    // this is the frame which will be fed to the encoder
    void draw_video_frame_into_fbo()
    {
        rt_cam->clear_buffers();
        rt_video_quad->draw_out_of_sync(rt_cam);
    }
    
    void create_rt()
    {
        int width = recv_params->width;
        int height = recv_params->height;
        
        if (!rt_tex || (rt_tex->get_width() != width) || (rt_tex->get_height() != height))
        {
            if(!gi)
            {
                gi = gfx::nwi();
            }
            
            gfx_tex_params prm;
            
            prm.set_rt_params();
            rt_tex = gi->tex.nwi("vid-reenc-" + gfx_tex::gen_id(), width, height, &prm);
            rt = gi->rt.new_rt();
            rt->set_color_attachment(rt_tex);
            
            rt_cam = gfx_camera::nwi(gi);
            rt_cam->projection_type = gfx_camera::e_orthographic_proj;
            rt_cam->clear_color = gfx_color::colors::dark_orange;
            rt_cam->clear_color = true;
            
            {
                rt_video_quad = gfx_quad_2d::nwi(gi);
                auto& msh = *rt_video_quad;
                
                rt_video_frame = gi->tex.nwi_external("vid-frame" + gfx_tex::gen_id(), 0, "RGBA8");
                msh.set_dimensions(1.f, 1.f);
                msh.set_scale((float)width, (float)height);
                msh.set_v_flip(true);
                msh[MP_SHADER_NAME] = "basic-tex-shader";
                msh["u_s2d_tex"][MP_TEXTURE_INST] = rt_video_frame;
                msh[MP_CULL_BACK] = false;
            }
        }
    }
    
    mws_vdec_state vdec_state;
   mws_sp<ios_video_enc> venc;
   mws_sp<mws_vdec_listener> vdec_listener;
   mws_sp<mws_vreencoder_listener> video_reencoder_listener;
   mws_sp<mws_video_params> recv_params;
   std::string src_video_path;
    // rt
    mws_sp<gfx> gi;
    mws_sp<gfx_tex> rt_video_frame;
    mws_sp<gfx_rt> rt;
    mws_sp<gfx_tex> rt_tex;
    mws_sp<gfx_camera> rt_cam;
    mws_sp<gfx_quad_2d> rt_video_quad;
};


mws_sp<ios_video_reencoder> ios_video_reencoder::nwi()
{
   auto p = mws_sp<ios_video_reencoder_impl>(new ios_video_reencoder_impl());
   auto r = mws_sp<ios_video_reencoder>(new ios_video_reencoder());
   p->venc->p->reencoder_impl = p;
   r->p = p;
   // it's going to take a lot of time to get rid of this hack
   global_vreencoder_impl = p;

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
   p->venc->p->src_video_path = i_video_path;
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
    p->start_encoding(i_prm);
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

void ios_video_reencoder::set_listener(mws_sp<mws_vdec_listener> i_listener)
{
   p->vdec_listener = i_listener;
}

void ios_video_reencoder::set_reencode_listener(std::shared_ptr<mws_vreencoder_listener> i_listener)
{
    p->video_reencoder_listener = i_listener;
}


void video_enc_progress_handler(mws_sp<ios_video_enc_impl> i_venc_impl, int i_progress_0_2_100)
{
	//mws_print("video_enc_progress_handler [%d]\n", i_progress_0_2_100);
    auto reencoder_impl = i_venc_impl->reencoder_impl.lock();
    
    if(reencoder_impl->vdec_listener)
    {
        reencoder_impl->vdec_listener->on_progress_evt(i_progress_0_2_100 / 100.f);
    }
}

void video_enc_finished_handler(mws_sp<ios_video_enc_impl> i_venc_impl, std::string i_new_video_path)
{
	auto reencoder_impl = i_venc_impl->reencoder_impl.lock();
	
	if(reencoder_impl->vdec_listener)
	{
		reencoder_impl->vdec_listener->on_finish();
	}
	
    reencoder_impl->vdec_state = mws_vdec_state::st_stopped;
    reencoder_impl->venc->stop_encoding();
	mws_print("video_enc_finished_handler [%s]\n", i_new_video_path.c_str());
}


#ifdef __cplusplus
extern "C"
{
#endif
    
    void render_video_frame_to_fbo(int fb_width, int fb_height, int tex_gl_id)
    {
        auto vreencoder_impl = global_vreencoder_impl.lock();
        mws_assert(vreencoder_impl != nullptr);
        vreencoder_impl->encode_video_frame(tex_gl_id);
    }
    
    void render_video_frame_to_fbo_2()
    {
        auto vreencoder_impl = global_vreencoder_impl.lock();
        mws_assert(vreencoder_impl != nullptr);
        vreencoder_impl->draw_video_frame_into_fbo();
    }

#ifdef __cplusplus
}
#endif

#endif
