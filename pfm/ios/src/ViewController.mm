#import "ViewController.h"
#import "main.hpp"
#import "unit-ctrl.hpp"
#include "com/unit/input-ctrl.hpp"


@interface ViewController ()
{
}

@property (strong, nonatomic) EAGLContext *context;

- (void)setupGL;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    
    if (!self.context) {
        NSLog(@"Failed to create ES context");
    }
    
    GLKView *view = (GLKView *)self.view;
    view.context = self.context;
    view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    view.drawableMultisample = GLKViewDrawableMultisample4X;

    CGRect screen_rect = [[UIScreen mainScreen] bounds];
    CGFloat screen_width = screen_rect.size.width;
    CGFloat screen_height = screen_rect.size.height;

    unit_ctrl::inst()->resize_app(screen_width, screen_height);
    
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
//    on_surface_created();
//    on_surface_changed();
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

@end
