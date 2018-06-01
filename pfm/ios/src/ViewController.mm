#import "ViewController.h"
#include "main.hpp"
#include "unit-ctrl.hpp"
#include "com/unit/input-ctrl.hpp"
#include "gfx.hpp"
#include "gfx-tex.hpp"
#include "objc-cxx-bridge.hpp"

#import "ios/vid/dec/video-player.h"
#import "ios/vid/enc/GPUImageFilter.h"
#import "ios/vid/enc/GPUImageMovie.h"
#import "ios/vid/enc/GPUImageOutput.h"
#import "ios/vid/enc/GPUImageMovieWriter.h"
#import "ios/vid/enc/GPUImageView.h"

#import <MobileCoreServices/MobileCoreServices.h>
#include <unistd.h>


std::shared_ptr<gfx_tex> cxx_2_objc_load_tex_by_name(std::string i_filename, std::shared_ptr<gfx> i_gi)
{
    auto tex = i_gi->tex.get_texture_by_name(i_filename);
    
    if(tex)
    {
        return tex;
    }
    
    auto c_filename = i_filename.c_str();
    NSString* nss_filename = [[NSString alloc] initWithUTF8String:c_filename];
    NSDictionary* options = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:NO], GLKTextureLoaderOriginBottomLeft, nil];
    // get the main bundle for the app
    NSBundle* main_bundle = [NSBundle mainBundle];
    NSError* error;
    NSString* path = [main_bundle pathForResource:nss_filename ofType:@""];
    GLKTextureInfo* info = [GLKTextureLoader textureWithContentsOfFile:path options:options error:&error];
    
    if (info != nil)
    {
        tex = i_gi->tex.nwi_external(i_filename, info.name, "RGBA8");
		tex->set_dim(info.width, info.height);
    }
    else
    {
        NSLog(@"load_tex_by_name: error loading file: %@", [error localizedDescription]);
    }
    
    return tex;
}

std::shared_ptr<gfx_tex> load_tex_by_ui_image(UIImage* image, std::string i_filename, std::shared_ptr<gfx> i_gi)
{
    auto tex = i_gi->tex.get_texture_by_name(i_filename);
    
    if(tex)
    {
        return tex;
    }
    
    NSError* error;
    GLKTextureInfo* info = [GLKTextureLoader textureWithCGImage:image.CGImage options:nil error:&error];
    
    if (info != nil)
    {
        tex = i_gi->tex.nwi_external(i_filename, info.name, "RGBA8");
		tex->set_dim(info.width, info.height);
    }
    else
    {
        NSLog(@"Error loading file: %@", [error localizedDescription]);
    }
    
    return tex;
}


@interface ViewController ()
{
    UIImagePickerController* ipc;
    UIPopoverController* popover;
}

@property (strong, nonatomic) EAGLContext *context;

- (void)setupGL;

@end

static EAGLContext* eagl_context_inst = NULL;
static ViewController* instance = NULL;

// GPUImageMovie
static GPUImageView* gpu_image_view;
static GPUImageMovie* movieFile;
//static GPUImageOutput<GPUImageInput>* filter;
static GPUImageMovieWriter* movieWriter;
static NSTimer* timer;

void cxx_2_objc_encode_video(std::string i_src_path, std::string i_dst_path)
{
	// if a file with that name already exists, delete the old movie
	unlink(i_dst_path.c_str());
	
    NSString* src_path_nss = [[NSString alloc] initWithUTF8String:i_src_path.c_str()];
    NSString* dst_path_nss = [[NSString alloc] initWithUTF8String:i_dst_path.c_str()];
    //mws_print("\n\nencode_selected_videoooooooo : %s %s \n\n\n", i_src_path.c_str(), i_dst_path.c_str());
    //[[VideoEncAppGLKitVC inst] encode_video:src_path_nss i_dst_path:dst_path_nss];
}


@implementation ViewController

+(EAGLContext*) eagl_context
{
    return eagl_context_inst;
}

+(ViewController*) inst
{
    return instance;
}

- (void)retrieving_progress
{
    int p = (int)(movieFile.progress * 100);
    //controller::inst()->on_progress_evt(movieFile.progress);
    //vprint("progress: %d\n", p);
}

-(void)encode_video:(NSString*) src_path dst_path:(NSString*) dst_path
{
    NSLog(@"\n\nenc src %@ dst %@\n\n", src_path, dst_path);
    NSString* full_src_path = [video_file_util getQualifiedFilenameOrResource:src_path];
    NSURL* sampleURL = [NSURL fileURLWithPath:full_src_path];
    //NSURL *sampleURL = [[NSBundle mainBundle] URLForResource:@"sample_iPod" withExtension:@"m4v"];
    
    movieFile = [[GPUImageMovie alloc] initWithURL:sampleURL];
    movieFile.runBenchmark = NO;
    movieFile.playAtActualSpeed = NO;
    //filter = [[GPUImageFilter alloc] init];
    //    filter = [[GPUImageUnsharpMaskFilter alloc] init];
    
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
    //NSString *pathToMovie = [NSHomeDirectory() stringByAppendingPathComponent:@"Documents/Movie.m4v"];
    unlink([dst_path UTF8String]); // If a file already exists, AVAssetWriter won't let you record new frames, so delete the old movie
    NSURL *movieURL = [NSURL fileURLWithPath:dst_path];
    
    //movieWriter = [[GPUImageMovieWriter alloc] initWithMovieURL:movieURL size:CGSizeMake(640.0, 480.0)];
    movieWriter = [[GPUImageMovieWriter alloc] initWithMovieURL:movieURL size:CGSizeMake(screen_width, screen_height)];
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
            
//            controller::inst()->on_encoding_finished();
//            auto vp = controller::inst()->new_video();
//            
//            if(vp)
//            {
//                auto dst_path_c = [dst_path UTF8String];
//                
//                vp->set_video_path(dst_path_c);
//                vp->play();
//            }
        });
    }];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    EAGLSharegroup* sg = [[[GPUImageContext sharedImageProcessingContext] context] sharegroup];
    instance = self;
    self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3 sharegroup:sg];
    eagl_context_inst = self.context;
    
    if (!self.context) {
        NSLog(@"Failed to create ES context");
    }
    
    GLKView *view = (GLKView *)self.view;
    view.context = self.context;
    view.drawableColorFormat = GLKViewDrawableColorFormatRGBA8888;
    view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    view.drawableMultisample = GLKViewDrawableMultisample4X;
    view.contentScaleFactor = 2.0;
    
    // Set animation frame rate
    self.preferredFramesPerSecond = 60;

    float scale = [UIScreen mainScreen].scale;
    CGRect screen_rect = [[UIScreen mainScreen] bounds];
    CGFloat screen_width = screen_rect.size.width;
    CGFloat screen_height = screen_rect.size.height;

    unit_ctrl::inst()->resize_app(screen_width, screen_height);
    // this is needed to find out the default framebuffer's id
    [view bindDrawable];
    [self setupGL];
}

- (void)dealloc
{
    if ([EAGLContext currentContext] == self.context) {
        [EAGLContext setCurrentContext:nil];
    }
}

- (void)setupGL
{
    [EAGLContext setCurrentContext:self.context];
    ios_main::get_instance()->init();
    ios_main::get_instance()->start();
}

#pragma mark - GLKView and GLKViewController delegate methods


- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

- (void) handle_touch_event:(pointer_evt::e_touch_type)type with_touches:(NSSet*)touches with_event:(UIEvent*)event
{
    auto evt = std::make_shared<pointer_evt>();
    NSEnumerator* enumerator = [[event allTouches] objectEnumerator];
    UITouch* current_touch;
    
    evt->type = type;
    evt->time = pfm::time::get_time_millis();
    
    while ((current_touch = [enumerator nextObject]))
    {
        if ((evt->touch_count + 1) < pointer_evt::MAX_TOUCH_POINTS)
        {
            CGPoint pos = [current_touch locationInView:current_touch.view];
            pointer_evt::touch_point& current_point = evt->points[evt->touch_count++];
            
            current_point.identifier = (uintptr_t) current_touch;
            current_point.x = pos.x;
            current_point.y = pos.y;
            current_point.is_changed = [touches containsObject:current_touch];
        }
    }
    
    if(evt->touch_count > 0)
    {
        unit_ctrl::inst()->pointer_action(evt);
    }
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    [super touchesBegan:touches withEvent:event];
    
    //    NSLog(@"timeSinceLastUpdate: %f", self.timeSinceLastUpdate);
    //    NSLog(@"timeSinceLastDraw: %f", self.timeSinceLastDraw);
    //    NSLog(@"timeSinceFirstResume: %f", self.timeSinceFirstResume);
    //    NSLog(@"timeSinceLastResume: %f", self.timeSinceLastResume);
    //    NSLog(@"touch press: %f, %f", normalizedPoint.x, normalizedPoint.y);
    //self.paused = !self.paused;
    
    [self handle_touch_event:pointer_evt::touch_began with_touches:touches with_event:event];
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    [super touchesMoved:touches withEvent:event];
    
    [self handle_touch_event:pointer_evt::touch_moved with_touches:touches with_event:event];
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    [super touchesEnded:touches withEvent:event];
    
    [self handle_touch_event:pointer_evt::touch_ended with_touches:touches with_event:event];
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    [super touchesCancelled:touches withEvent:event];
    
    [self handle_touch_event:pointer_evt::touch_cancelled with_touches:touches with_event:event];
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
    ios_main::get_instance()->run();
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

-(void)on_bg_img_btn_click
{
    ipc= [[UIImagePickerController alloc] init];
    ipc.delegate = self;
    //ipc.sourceType = UIImagePickerControllerSourceTypeSavedPhotosAlbum;
    ipc.sourceType = UIImagePickerControllerSourceTypePhotoLibrary;
    
    if(UI_USER_INTERFACE_IDIOM()==UIUserInterfaceIdiomPhone)
        [self presentViewController:ipc animated:YES completion:nil];
//    else
//    {
//        popover=[[UIPopoverController alloc]initWithContentViewController:ipc];
//        [popover presentPopoverFromRect:_bg_image_btn.frame inView:self.view permittedArrowDirections:UIPopoverArrowDirectionAny animated:YES];
//    }
}

-(void)on_vid_btn_click
{
    ipc = [[UIImagePickerController alloc] init];
    ipc.delegate = self;
    ipc.sourceType = UIImagePickerControllerSourceTypePhotoLibrary;
    ipc.mediaTypes = [[NSArray alloc] initWithObjects:(NSString *)kUTTypeMovie, nil];
    [ipc setVideoQuality:UIImagePickerControllerQualityTypeHigh];
    
    if(UI_USER_INTERFACE_IDIOM()==UIUserInterfaceIdiomPhone)
        [self presentViewController:ipc animated:YES completion:nil];
//    else
//    {
//        popover=[[UIPopoverController alloc]initWithContentViewController:ipc];
//        [popover presentPopoverFromRect:_video_file_btn.frame inView:self.view permittedArrowDirections:UIPopoverArrowDirectionAny animated:YES];
//    }
}


-(void)imagePickerController:(UIImagePickerController *)picker didFinishPickingMediaWithInfo:(NSDictionary *)info
{
    NSString* media_type = [info objectForKey: UIImagePickerControllerMediaType];
    
    if (CFStringCompare ((__bridge CFStringRef) media_type, kUTTypeMovie, 0) == kCFCompareEqualTo)
    {
        NSURL* video_url = [info objectForKey:UIImagePickerControllerMediaURL];
        NSString* video_path = [video_url path];
        
        if(video_path)
        {
            const char* video_path_c = [video_path UTF8String];
            std::string video_path_str = video_path_c;
            
            objc_2_cxx_set_video_path_and_play(video_path_str);
            NSLog(@"video path %@", video_path);
        }
        
        [picker dismissViewControllerAnimated:YES completion:nil];
    }
    else
    {
        if(UI_USER_INTERFACE_IDIOM()==UIUserInterfaceIdiomPhone)
        {
            [picker dismissViewControllerAnimated:YES completion:nil];
        }
        else
        {
            [popover dismissPopoverAnimated:YES];
        }
        
        NSURL* img_url = [info objectForKey:UIImagePickerControllerReferenceURL];
        NSString* img_path = img_url.absoluteString;
        
        UIImage* picked_image = [info objectForKey:UIImagePickerControllerOriginalImage];
        
        if(picked_image && img_path)
        {
            const char* img_path_c = [img_path UTF8String];
            std::string img_path_str = img_path_c;
            auto tex = load_tex_by_ui_image(picked_image, img_path_str, gfx::i());
            
            if(tex)
            {
                objc_2_cxx_set_bg_tex(tex);
            }
        }
    }
}

-(void)imagePickerControllerDidCancel:(UIImagePickerController *)picker
{
    [picker dismissViewControllerAnimated:YES completion:nil];
}

@end


void cxx_2_objc_open_image_picker()
{
    [[ViewController inst] on_bg_img_btn_click];
}

void cxx_2_objc_open_video_picker()
{
    [[ViewController inst] on_vid_btn_click];
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
        
    }

#ifdef __cplusplus
}
#endif

GL_API void GL_APIENTRY glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, void *data)
{
    mws_assert(false);
}
