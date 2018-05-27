#include "stdafx.h"

#include "appplex-conf.hpp"
#include "ios-video-dec.hpp"
#include "pfm.hpp"
#include "min.hpp"

#if defined PLATFORM_IOS

#include "ViewController.h"
#include "dec/video-player.h"
#include "gfx.hpp"
#include "gfx-camera.hpp"
#include "gfx-tex.hpp"
#include "gfx-rt.hpp"
#include "gfx-quad-2d.hpp"
#include "glm/gtc/type_ptr.hpp"
#import <AVFoundation/AVAudioPlayer.h>
#import <AVFoundation/AVMediaFormat.h>
#import <AVFoundation/AVAssetExportSession.h>
#import <AVFoundation/AVAsset.h>


std::shared_ptr<mws_video_dec> mws_video_dec::nwi()
{
   return ios_video_dec::nwi();
}

@interface AnimHelper : NSObject <AVAudioPlayerDelegate>
{
}
@end

@interface AnimHelper ()
{
    AVAudioPlayer* audio_player;
    bool can_play_audio;
    NSURL* audio_url;
}

@property (nonatomic, retain) video_dec_player* video_player;
@property (nonatomic, assign) BOOL keepAnimating;

@end

@implementation AnimHelper

@synthesize video_player = _video_player;
@synthesize keepAnimating = _keepAnimating;

uint32 start_time;

-(void)set_video_path:(NSString*)video_path
{
    _video_player.assetFilename = video_path;
    audio_player = nil;
    can_play_audio = false;
}

-(void)start_playing
{
    if(audio_player && can_play_audio)
    {
        [_video_player prepareToAnimate];
    }
    else
    {
        NSString* video_path_nss = _video_player.assetFilename;
        [self extract_audio_from_video:video_path_nss];
    }
}

-(void)stop_playing
{
    [_video_player stopAnimator];
    [self stop_audio_player];
}

-(void)start_audio_player
{
    if(audio_player)
    {
        if([audio_player isPlaying])
        {
            [audio_player stop];
        }
        
        audio_player.currentTime = 0;
        [audio_player play];
    }
    else
    {
        NSError *error;
        
        audio_player = [[AVAudioPlayer alloc] initWithContentsOfURL:audio_url fileTypeHint:AVFileTypeMPEG4 error:&error];
        audio_player.delegate = self;
        
        if (!error)
        {
            [audio_player play];
        }
        else
        {
            NSLog(@"Error in audioPlayer: %@", [error localizedDescription]);
        }
    }
}

-(void)stop_audio_player
{
    if(audio_player)
    {
        if([audio_player isPlaying])
        {
            [audio_player stop];
        }
    }
}

-(void)extract_audio_from_video:(NSString*)video_url_nss
{
    NSString* filename_ext = [video_url_nss lastPathComponent];
    NSString* filename = [filename_ext stringByDeletingPathExtension];
    NSString* filename_audio = [NSString stringWithFormat:@"%@.m4a", filename];
    NSLog(@"\n\nextract_audio_from_video 1: %@ \n\n", filename);
    //float startTime = 0;
    //float endTime = 10;
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    NSString *audioPath = [documentsDirectory stringByAppendingPathComponent:filename_audio];
    
    NSString* asset_full_path = [video_file_util getQualifiedFilenameOrResource:video_url_nss];
    NSURL* video_url = [[NSURL alloc] initFileURLWithPath:asset_full_path];
    //AVAsset *myasset = [AVAsset assetWithURL:[[NSBundle mainBundle] URLForResource:@"VideoName" withExtension:@"mp4"]];
    AVAsset* myasset = [AVAsset assetWithURL:video_url];
    NSLog(@"\n\nextract_audio_from_video 2: %@ \n\n", asset_full_path);
    
    AVAssetExportSession *exportSession=[AVAssetExportSession exportSessionWithAsset:myasset presetName:AVAssetExportPresetAppleM4A];
    
    exportSession.outputURL=[NSURL fileURLWithPath:audioPath];
    exportSession.outputFileType=AVFileTypeAppleM4A;
    
    CMTime vocalStartMarker = kCMTimeZero;//CMTimeMake((int)(floor(startTime * 100)), 100);
    CMTime vocalEndMarker = kCMTimePositiveInfinity;//CMTimeMake((int)(ceil(endTime * 100)), 100);
    
    CMTimeRange exportTimeRange = CMTimeRangeFromTimeToTime(vocalStartMarker, vocalEndMarker);
    exportSession.timeRange= exportTimeRange;
    
    if ([[NSFileManager defaultManager] fileExistsAtPath:audioPath])
    {
        [[NSFileManager defaultManager] removeItemAtPath:audioPath error:nil];
    }
    
    [exportSession exportAsynchronouslyWithCompletionHandler:^
     {
         if (exportSession.status==AVAssetExportSessionStatusFailed)
         {
             NSLog(@"failed");
             audio_url = nil;
             can_play_audio = false;
         }
         else
         {
             NSLog(@"AudioLocation : %@",audioPath);
             audio_url = exportSession.outputURL;
             can_play_audio = true;
         }
         
         if([NSThread currentThread] == [NSThread mainThread])
         {
             [_video_player prepareToAnimate];
         }
         else
         {
             // run on main thread
             dispatch_async(dispatch_get_main_queue(), ^
                            {
                                [_video_player prepareToAnimate];
                            });
         }
     }];
}

- (void)audioPlayerDidFinishPlaying:(AVAudioPlayer *)aPlayer successfully:(BOOL)flag
{
    NSLog(@"audioPlayerDidFinishPlaying");
    //audio_player.currentTime = 0;
    //[audio_player play];
}

// The prepared callback is invoked once the asset has been opened and the first frame
// has been loaded into memory. Note that this prepared callback can be invoked multiple
// times in the case looping the media playback is implemented.

- (void)animatorPreparedNotification:(NSNotification*)notification
{
    video_dec_player* player = notification.object;
    
    frame_decoder* frameDecoder = (frame_decoder*) player.frameDecoder;
    NSString *file = player.assetFilename;
    
    // Size of movie is available now
    
    CGSize videoSize = CGSizeMake(frameDecoder.width, frameDecoder.height);
    
    NSLog(@"animatorPreparedNotification %@ : videoSize %d x %d", file, (int)videoSize.width, (int)videoSize.height);
    
    //  NSLog(@"self.carView origin %d,%d", (int)self.carView.frame.origin.x, (int)self.carView.frame.origin.y);
    
    //NSLog(@"self.carView : %d x %d", (int)self.video_player.frame.size.width, (int)self.video_player.frame.size.height);
    
    [self.video_player startAnimator];
    
    if(can_play_audio)
    {
        [self start_audio_player];
    }
    
    start_time = pfm::time::get_time_millis();
    return;
}

// Invoked after animation playback has stopped

- (void)animatorStoppedNotification:(NSNotification*)notification
{
    NSLog(@"animatorStoppedNotification");
    
    uint32 crt_time = pfm::time::get_time_millis();
    uint32 delta = crt_time - start_time;
    float duration = delta / 1000.f;
    mws_print("\n\n\nvideo duration: %f\n\n\n", duration);
    // Kick off another animation cycle by doing a prepare operation which will open
    // the asset resource and load the first frame of data. An asset based player
    // needs to be reinitialized each time it is played.
    
    if (self.keepAnimating)
    {
        [self.video_player prepareToAnimate];
    }
    
}

// Invoked when app goes into background

- (void)willResignActiveNotification:(NSNotification*)notification
{
    self.keepAnimating = FALSE;
    [self.video_player stopAnimator];
    [self stop_audio_player];
}

// Invoked when app goes from background to the foreground

- (void)willResumeActiveNotification:(NSNotification*)notification
{
    self.keepAnimating = TRUE;
    
    // Playback must be started over when app comes back into the foregroeund
    [self.video_player prepareToAnimate];
    
    //[self colorCycleAnimation];
}
@end


class ios_video_dec_impl
{
public:
    ios_video_dec_impl()
    {
        state = st_stopped;
        
        anim_helper_inst = [AnimHelper alloc];
        anim_helper_inst.video_player = [video_dec_player alloc];
        anim_helper_inst.video_player.context = [ViewController eagl_context];
        anim_helper_inst.keepAnimating = TRUE;
        
        [[NSNotificationCenter defaultCenter] addObserver:anim_helper_inst
                                                 selector:@selector(animatorPreparedNotification:)
                                                     name:vplayer_prepared_to_start_notif
                                                   object:anim_helper_inst.video_player];
        
        [[NSNotificationCenter defaultCenter] addObserver:anim_helper_inst
                                                 selector:@selector(animatorStoppedNotification:)
                                                     name:vplayer_did_stop_notif
                                                   object:anim_helper_inst.video_player];
        
        // App foreground/background state
        
        [[NSNotificationCenter defaultCenter] addObserver:anim_helper_inst
                                                 selector:@selector(willResignActiveNotification:)
                                                     name:UIApplicationWillResignActiveNotification
                                                   object:nil];
        
        [[NSNotificationCenter defaultCenter] addObserver:anim_helper_inst
                                                 selector:@selector(willResumeActiveNotification:)
                                                     name:UIApplicationWillEnterForegroundNotification
                                                   object:nil];
        
        {
            ortho_cam = gfx_camera::nwi();
            ortho_cam->projection_type = gfx_camera::e_orthographic_proj;
            ortho_cam->clear_color = true;
            ortho_cam->clear_depth = true;
        }
        {
            quad_mesh = std::make_shared<gfx_quad_2d>();
            auto& qm = *quad_mesh;
            
            qm[MP_SHADER_NAME] = "yuv";
        }
        {
            tex_y = gfx::i()->tex.new_external_tex_2d("tex-y", 0, 1, 1);
            tex_uv = gfx::i()->tex.new_external_tex_2d("tex-uv", 0, 1, 1);
        }
    }
    
    void play()
    {
        auto video_path_c = video_path.c_str();
        NSString* video_path_nss = [[NSString alloc] initWithUTF8String:video_path_c];
        
        [anim_helper_inst set_video_path:video_path_nss];
        [anim_helper_inst start_playing];
        state = st_playing;
    }
    
    void stop()
    {
        state = st_stopped;
        [anim_helper_inst stop_playing];
    }
    
    void pause()
    {
        state = st_paused;
        [anim_helper_inst stop_playing];
    }
    
    void destroy()
    {
        if(state == st_playing)
        {
            stop();
        }
        
        anim_helper_inst = nil;
    }
    
    void render_frame()
    {
        [anim_helper_inst.video_player drawRect];
        
        int width = video_width();
        
        // convert yuv to rgba
        if(width > 0)
        {
            auto& qm = *quad_mesh;
            gfx_uint y_gl_id = tex_y_gl_id();
            gfx_uint uv_gl_id = tex_uv_gl_id();
            
            if(!rt_tex || (rt_tex->get_width() != width))
            {
                static int tex_id = 0;
                auto pc = preferred_conversion();
                auto ccm = glm::make_mat3(pc);
                int height = video_height();
                gfx_tex_params p;
                
                p.mag_filter = p.e_tf_nearest;
                p.min_filter = p.e_tf_nearest;
                p.wrap_s = p.e_twm_clamp_to_edge;
                p.wrap_t = p.e_twm_clamp_to_edge;
                p.gen_mipmaps = false;
                
                rt = nullptr;
                rt_tex = nullptr;
                
                std::string tex_nr = std::to_string(tex_id);
                tex_id++;
                rt_tex = gfx::i()->tex.new_tex_2d("video-rgba-tex-" + tex_nr, width, height, &p);
                rt = gfx::i()->rt.new_rt();
                rt->set_color_attachment(rt_tex);
                
                qm.set_dimensions(2, 2);
                qm["SamplerY"] = tex_y->get_name();
                qm["SamplerUV"] = tex_uv->get_name();
                qm["colorConversionMatrix"] = ccm;
                qm.set_scale(width, height);
                qm.set_v_flip(true);
            }
            
            tex_y->set_texture_gl_id(y_gl_id);
            tex_uv->set_texture_gl_id(uv_gl_id);
            
            gfx::i()->rt.set_current_render_target(rt);
            ortho_cam->clear_buffers();
            qm.draw_out_of_sync(ortho_cam);
            gfx::i()->rt.set_current_render_target();
        }
    }
    
    void end_frame()
    {
        [anim_helper_inst.video_player end_frame];
    }
    
    unsigned int video_width()
    {
        return [anim_helper_inst.video_player video_width];
    }
    
    unsigned int video_height()
    {
        return [anim_helper_inst.video_player video_height];
    }
    
    std::shared_ptr<gfx_tex> get_current_frame()
    {
        return rt_tex;
    }
    
    unsigned int tex_y_gl_id()
    {
        return [anim_helper_inst.video_player tex_y_gl_id];
    }
    
    unsigned int tex_uv_gl_id()
    {
        return [anim_helper_inst.video_player tex_uv_gl_id];
    }
    
    const float* preferred_conversion()
    {
        return [anim_helper_inst.video_player preferred_conversion];
    }
    
    AnimHelper* anim_helper_inst;
    std::string video_path;
    std::shared_ptr<ios_media_info> mi;
    mws_vdec_state state;
    shared_ptr<gfx_camera> ortho_cam;
    shared_ptr<gfx_rt> rt;
    shared_ptr<gfx_tex> rt_tex;
    shared_ptr<gfx_tex> tex_y;
    shared_ptr<gfx_tex> tex_uv;
    shared_ptr<gfx_quad_2d> quad_mesh;
};


ios_media_info::ios_media_info(std::shared_ptr<ios_video_dec_impl> i_p)
{
    p = i_p;
}

int ios_media_info::get_width()
{
    return impl()->video_width();
}

int ios_media_info::get_height()
{
    return impl()->video_height();
}

int ios_media_info::get_current_frame_index()
{
    return (int)impl()->anim_helper_inst.video_player.frameDecoder.frameIndex;
}

int64 ios_media_info::get_frame_count()
{
    return (int64)impl()->anim_helper_inst.video_player.frameDecoder.numFrames;
}

double ios_media_info::get_frame_rate()
{
    return (double)impl()->anim_helper_inst.video_player.frameDecoder.nominalFrameRate;
}

unsigned long long ios_media_info::get_duration_us()
{
    mws_throw ia_exception("n/a");
    return 0;
}

std::shared_ptr<gfx_tex> ios_media_info::get_current_frame()
{
    return impl()->get_current_frame();
}

int ios_media_info::get_total_width()
{
    return impl()->video_width();
}

mws_vdec_state ios_media_info::get_dec_state()
{
    return impl()->state;
}

std::shared_ptr<ios_video_dec_impl> ios_media_info::impl() const
{
    return p.lock();
}


std::shared_ptr<ios_video_dec> ios_video_dec::nwi()
{
	return std::shared_ptr<ios_video_dec>(new ios_video_dec());
}

std::string ios_video_dec::get_video_path()
{
    return p->video_path;
}

void ios_video_dec::set_video_path(std::string i_video_path)
{
    p->video_path = i_video_path;
}

std::shared_ptr<mws_media_info> ios_video_dec::get_media_info()
{
    return p->mi;
}

int ios_video_dec::start_decoding()
{
    p->play();
    
    return 0;
}

void ios_video_dec::stop()
{
    p->stop();
}

mws_vdec_state ios_video_dec::get_state() const
{
    return p->state;
}

void ios_video_dec::update(std::shared_ptr<gfx_camera> i_mws_cam)
{
    p->render_frame();
    p->end_frame();
}

void ios_video_dec::play()
{
    p->play();
}

void ios_video_dec::replay()
{
    p->stop();
    p->play();
}

void ios_video_dec::pause()
{
    p->pause();
}

void ios_video_dec::play_pause()
{
    mws_throw ia_exception("n/a");
}

void ios_video_dec::goto_frame(int iframe_idx)
{
    mws_throw ia_exception("n/a");
}

void ios_video_dec::next_frame()
{
    mws_throw ia_exception("n/a");
}

void ios_video_dec::prev_frame()
{
    mws_throw ia_exception("n/a");
}

void ios_video_dec::set_frame_limit(float iframe_limit)
{
    mws_throw ia_exception("n/a");
}

void ios_video_dec::set_listener(std::shared_ptr<mws_vdec_listener> listener)
{
    mws_throw ia_exception("n/a");
}

ios_video_dec::ios_video_dec()
{
	p = std::make_unique<ios_video_dec_impl>();
    p->mi = std::make_shared<ios_media_info>(p);
}

#endif