// ViewController.h
// Your main view controller

#import <GLKit/GLKit.h>

@interface ViewController : GLKViewController <UIImagePickerControllerDelegate, UINavigationControllerDelegate>

+(EAGLContext*) eagl_context;
+(ViewController*) inst;

@end

