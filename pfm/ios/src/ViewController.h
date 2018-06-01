// ViewController.h
// Your main view controller

#import <GLKit/GLKit.h>

@interface ViewController : GLKViewController <UIImagePickerControllerDelegate, UINavigationControllerDelegate>

+(EAGLContext*) eagl_context;
+(ViewController*) inst;
-(void)retrieving_progress;
-(void)encode_video:(NSString*) src_path dst_path:(NSString*) dst_path;

@end

