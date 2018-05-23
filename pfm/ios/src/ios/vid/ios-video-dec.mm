#include "stdafx.h"

#include "appplex-conf.hpp"
#include "ios-video-dec.hpp"

#if defined PLATFORM_IOS

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
    
    start_time = get_time_millis();
    return;
}

// Invoked after animation playback has stopped

- (void)animatorStoppedNotification:(NSNotification*)notification
{
    NSLog(@"animatorStoppedNotification");
    
    uint32 crt_time = get_time_millis();
    uint32 delta = crt_time - start_time;
    float duration = delta / 1000.f;
    vprint("\n\n\nvideo duration: %f\n\n\n", duration);
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


class ios_video_dec_impl;
{
public:
    ios_video_dec_impl;()
    {
        is_playing = false;
        
        anim_helper_inst = [AnimHelper alloc];
        anim_helper_inst.video_player = [video_dec_player alloc];
        anim_helper_inst.video_player.context = [VideoEncAppGLKitVC eagl_context];
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
    }
    
    void play()
    {
        auto video_path_c = video_path.c_str();
        NSString* video_path_nss = [[NSString alloc] initWithUTF8String:video_path_c];
        
        [anim_helper_inst set_video_path:video_path_nss];
        [anim_helper_inst start_playing];
        is_playing = true;
    }
    
    void stop()
    {
        is_playing = false;
        [anim_helper_inst stop_playing];
    }
    
    void pause()
    {
        is_playing = false;
        [anim_helper_inst stop_playing];
    }
    
    void destroy()
    {
        if(is_playing)
        {
            stop();
        }
        
        anim_helper_inst = nil;
    }
    
    void render_frame()
    {
        [anim_helper_inst.video_player drawRect];
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
    bool is_playing;
    std::string video_path;
};


int ios_media_info::get_width()
{
	
}

int ios_media_info::get_height()
{
	
}

int ios_media_info::get_current_frame_index()
{
	
}

int64 ios_media_info::get_frame_count()
{
	
}

double gios_media_info::et_frame_rate()
{
	
}

unsigned long long ios_media_info::get_duration_us()
{
	
}

std::shared_ptr<gfx_tex> ios_media_info::get_current_frame()
{
	
}

int ios_media_info::get_total_width()
{
	
}

mws_vdec_state ios_media_info::get_dec_state()
{
	
}


std::shared_ptr<ios_video_dec> ios_video_dec::nwi()
{
	return std::shared_ptr<ios_video_dec>(new ios_video_dec());
}

std::string ios_video_dec::get_video_path()
{
	
}

void ios_video_dec::set_video_path(std::string i_video_path)
{
	
}

std::shared_ptr<mws_media_info> ios_video_dec::get_media_info()
{
	
}

int ios_video_dec::start_decoding()
{
	
}

void ios_video_dec::stop()
{
	
}

mws_vdec_state ios_video_dec::get_state() const
{
	
}

void ios_video_dec::update(std::shared_ptr<gfx_camera> i_mws_cam = nullptr)
{
	
}

void ios_video_dec::play()
{
	
}

void ios_video_dec::replay()
{
	
}

void ios_video_dec::pause()
{
	
}

void ios_video_dec::play_pause()
{
	
}

void ios_video_dec::goto_frame(int iframe_idx)
{
	
}

void ios_video_dec::next_frame()
{
	
}

void ios_video_dec::prev_frame()
{
	
}

void ios_video_dec::set_frame_limit(float iframe_limit)
{
	
}

void ios_video_dec::set_listener(std::shared_ptr<mws_vdec_listener> listener)
{
	
}

ios_video_dec::ios_video_dec()
{
	p = std::make_unique<ios_video_dec_impl>();
}

#endif
