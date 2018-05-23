#import "ViewController.h"
#include "main.hpp"
#include "unit-ctrl.hpp"
#include "com/unit/input-ctrl.hpp"
#include "gfx.hpp"
#include "gfx-tex.hpp"
#include "objc-cxx-bridge.hpp"
#import <MobileCoreServices/MobileCoreServices.h>



std::shared_ptr<gfx_tex> load_texture_by_ui_image(UIImage* image, std::string image_filename, std::shared_ptr<gfx> gfx_inst)
{
    auto tex = gfx_inst->tex.get_texture_by_name(image_filename);
    
    if(tex)
    {
        return tex;
    }
    
    NSError* error;
    GLKTextureInfo* info = [GLKTextureLoader textureWithCGImage:image.CGImage options:nil error:&error];
    
    if (info != nil)
    {
        gfx_uint tex_gl_id = info.name;
        gfx_uint width = info.width;
        gfx_uint height = info.height;
        gfx_tex_params p;
        
        tex = gfx_inst->tex.new_external_tex_2d(image_filename, tex_gl_id, width, height, &p);
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

@implementation ViewController

+(EAGLContext*) eagl_context
{
    return eagl_context_inst;
}

+(ViewController*) inst
{
    return instance;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    instance = self;
    self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    
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
            auto tex = load_texture_by_ui_image(picked_image, img_path_str, gfx::i());
            
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

int render_video_frame_to_fbo(int fb_width, int fb_height, int tex_gl_id)
{
    return 0;
}

void render_video_frame_to_fbo_2()
{
    
}
