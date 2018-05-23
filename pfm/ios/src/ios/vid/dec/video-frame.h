#import <Foundation/Foundation.h>

#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#else
#import <Cocoa/Cocoa.h>
#endif

@class video_fb;

@interface video_frame : NSObject
{
#if TARGET_OS_IPHONE
  UIImage *m_image;
#else
  NSImage *m_image;
#endif // TARGET_OS_IPHONE
  
  video_fb *m_cgFrameBuffer;
  CVImageBufferRef m_cvBufferRef;
  BOOL m_isDuplicate;
}

#if TARGET_OS_IPHONE
@property (nonatomic, retain) UIImage *image;
#else
@property (nonatomic, retain) NSImage *image;
#endif // TARGET_OS_IPHONE

// If the frame data is already formatted as a pixel buffer, then
// this field is non-nil. A pixel buffer can be wrapped into
// platform specific image data.

@property (nonatomic, retain) video_fb *cgFrameBuffer;

// A frame decoder might provide a buffer directly as a CoreVideo image
// buffer as opposed to an image. Typically, this ref actually points
// at a CVPixelBufferRef, but CVImageBufferRef is a more generic superclass
// ref that could also apply to an OpenGL buffer. The frame_decoder
// class can decode this type of CoreVideo buffer directly.

@property (nonatomic, assign) CVImageBufferRef cvBufferRef;

@property (nonatomic, assign) BOOL     isDuplicate;

// Constructor

+ (video_frame*) vid_frame;

// If the image property is nil but the cgFrameBuffer is not nil, then
// create the image object from the contents of the cgFrameBuffer. This
// method attempts to verify that the image object is created and initialized
// as much as possible, though some image operations may still be deferred
// until the render cycle.

- (void) makeImageFromFramebuffer;

@end
