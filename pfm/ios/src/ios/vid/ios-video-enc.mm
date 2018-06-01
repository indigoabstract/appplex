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
#include <unistd.h>


class ios_video_enc_impl;
class ios_video_reencoder_impl;


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
	mws_wp<ios_video_enc_impl> venc_impl;
    NSURL* src_video_url;
    NSURL* dst_video_url;
}

@end

@implementation video_encoder_helper

- (void)retrieving_progress
{
    int progress_0_2_100 = (int)(movieFile.progress * 100);
	video_enc_progress_handler(venc_impl, progress_0_2_100);
    //mws_print("progress: %d\n", p);
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
			
			video_enc_finished_handler(venc_impl, new_video_path);
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

	void start_encoding(std::shared_ptr<gfx> i_gi, const mws_video_params& i_prm, mws_vid_enc_method i_enc_method)
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
    video_encoder_helper* venc_helper_inst;
    std::string src_video_path;
	mws_wp<ios_video_reencoder_impl> reencoder_impl;
};


std::shared_ptr<ios_video_enc> ios_video_enc::nwi()
{
	auto i = std::shared_ptr<ios_video_enc>(new ios_video_enc());
	i->p = std::shared_ptr<ios_video_enc_impl>(new ios_video_enc_impl());
	i->p->venc_helper_inst = i->p;
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
   mws_sp<mws_video_reencoder_listener> video_reencoder_listener;
   mws_video_params params;
   std::string src_video_path;
};


mws_sp<ios_video_reencoder> ios_video_reencoder::nwi()
{
   auto p = mws_sp<ios_video_reencoder_impl>(new ios_video_reencoder_impl());
   auto r = mws_sp<ios_video_reencoder>(new ios_video_reencoder());
   p->venc->p->reencoder_impl = p;
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
    p->vdec_state = mws_vdec_state::st_playing;
    p->venc->start_encoding(nullptr, i_prm, mws_vid_enc_method::e_enc_m2);
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
   p->video_reencoder_listener = i_listener;
}


void video_enc_progress_handler(mws_sp<ios_video_enc_impl> i_venc_impl, int i_progress_0_2_100)
{
	mws_print("video_enc_progress_handler [%d]\n", i_progress_0_2_100);
}

void video_enc_finished_handler(mws_sp<ios_video_enc_impl> i_venc_impl, std::string i_new_video_path)
{
	auto i_venc_impl->reencoder_impl.lock();
	
	if(i_venc_impl->video_reencoder_listener)
	{
		i_venc_impl->video_reencoder_listener->on_decoding_finished();
	}
	
	mws_print("video_enc_finished_handler [%s]\n", i_new_video_path.c_str());
}


#ifdef __cplusplus
extern "C"
{
#endif
    
    int render_video_frame_to_fbo(int fb_width, int fb_height, int tex_gl_id)
    {
        return 0;
    }
    
    void render_video_frame_to_fbo_2()
    {
        int x = 3;
    }

#ifdef __cplusplus
}
#endif

#endif
