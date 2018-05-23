#import "video-player.h"

#import <QuartzCore/QuartzCore.h>

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import "video-fb.h"
#import "video-frame.h"
#import <mach/mach.h>
#import <CoreMedia/CMSampleBuffer.h>
#import <QuartzCore/CAEAGLLayer.h>

typedef enum vplayer_state {
    ALLOCATED = 0,
    LOADED,
    FAILED,
    PREPPING,
    READY,
    ANIMATING,
    STOPPED,
    PAUSED
} vplayer_state;

// Color Conversion Constants (YUV to RGB) including adjustment from 16-235/16-240 (video range)

// BT.601, which is the standard for SDTV.
static const GLfloat kColorConversion601[] = {
    1.164,  1.164, 1.164,
    0.0, -0.392, 2.017,
    1.596, -0.813,   0.0,
};

// BT.709, which is the standard for HDTV.
static const GLfloat kColorConversion709[] = {
    1.164,  1.164, 1.164,
    0.0, -0.213, 2.112,
    1.793, -0.533,   0.0,
};

// FIXME: The input sRGB values need to be moved back to a linear
// colorspace so that the non-linear undelta result can be operated
// on with a simple floating point multiplication as opposed to
// a table based approach in the shaders.

// https://developer.apple.com/library/ios/documentation/Metal/Reference/MetalShadingLanguageGuide/numerical-comp/numerical-comp.html
// https://www.khronos.org/webgl/public-mailing-list/archives/1009/msg00028.php
// http://entropymine.com/imageworsener/srgbformula/

const static BOOL renderBGRA = FALSE;
// Debug render from CoreVideo

#if defined(DEBUG)

@interface frame_decoder ()

- (BOOL) renderCVImageBufferRefIntoFramebuffer:(CVImageBufferRef)imageBuffer frameBuffer:(video_fb**)frameBufferPtr;

@end

#endif // DEBUG

@interface video_dec_player () {
@private
    CGSize m_renderSize;
    
    int renderBufferWidth;
    int renderBufferHeight;
    
    // A texture cache ref is an opaque type that contains a specific
    // textured cache. Note that in the case where there are 2 textures
    // just one cache is needed.
    
    CVOpenGLESTextureCacheRef textureCacheRef;
    CVOpenGLESTextureRef textureRef;
    CVOpenGLESTextureRef textureUVRef;
    CVOpenGLESTextureRef textureAlphaRef;
    
    // Colorspace conversion
    
    const GLfloat *_preferredConversion;
    
    BOOL didSetupOpenGLMembers;
    
    vplayer_state m_state;
    
    // Stores the first time when a display link callback was
    // delivered. This corresponds to the time for frame 1.
    
    CFTimeInterval firstTimeInterval;
    
    // This value is set in the main thread when the display
    // link timer is fired. If the display link is running
    // behind the wall clock then this value is advanced
    // so that the decoder can tell things are falling behind.
    
    int nextDecodeFrame;
    
    // repeating GCD timer
    
    dispatch_source_t _dispatchTimer;
    dispatch_queue_t _highPrioQueue;
    
    // Timer set when a repeating dispatch timer is started
    // but with the knowledge that the value only be accessed
    // from the secondary thread.
    
    CFTimeInterval dispatchFirstTimeInterval;
    CFTimeInterval dispatchPrevTimeInterval;
    
    // The number of frames that can be accessed by the
    // decoder. This value should be thread safe to access
    // since it is only set once at load time.
    
    int m_dispatchMaxFrame;
    
    CFTimeInterval frames_per_second;
    CFTimeInterval frame_duration;
}

@property (nonatomic, assign) CGSize renderSize;

// Must be atomic since this property can be accessed
// from both the main and decode threads.

@property (atomic, assign) int currentFrame;

// This atomic property stores the largest frame
// number in the RGB+Alpha frames.

@property (atomic, assign) int dispatchMaxFrame;

// This property is non-zero when waiting for the display timer
// and the decoder timer to sync.

@property (atomic, assign) uint32_t waitingForDecodeToStart;

@property (nonatomic, retain) video_frame *rgbFrame;

@property (nonatomic, retain) NSTimer *animatorPrepTimer;

@property (nonatomic, assign) BOOL renderYUVFrames;

@property (nonatomic, assign) vplayer_state state;

@property (nonatomic, retain) CADisplayLink *displayLink;

// This NSDate object stores a projected optimal frame
// decode time for the next frame decode operation.

+ (uint32_t) timeIntervalToFrameOffset:(CFTimeInterval)elapsed
                                   fps:(CFTimeInterval)fps;

@end

@implementation video_dec_player

// public properties

@synthesize renderSize = m_renderSize;
@synthesize rgbFrame = m_rgbFrame;
@synthesize assetFilename = m_assetFilename;
@synthesize frameDecoder = m_frameDecoder;
@synthesize animatorPrepTimer = m_animatorPrepTimer;
@synthesize currentFrame = m_currentFrame;
@synthesize state = m_state;
@synthesize dispatchMaxFrame = m_dispatchMaxFrame;
@synthesize context = m_context;
@synthesize video_width = m_video_width;
@synthesize video_height = m_video_height;
@synthesize tex_y_gl_id = m_tex_y_gl_id;
@synthesize tex_uv_gl_id = m_tex_uv_gl_id;

#if defined(DEBUG)
@synthesize captureDir = m_captureDir;
#endif // DEBUG

- (void) dealloc {
    // Explicitly release image inside the imageView, the
    // goal here is to get the imageView to drop the
    // ref to the CoreGraphics image and avoid a memory
    // leak. This should not be needed, but it is.
    
    self.rgbFrame = nil;
    self.frameDecoder = nil;
    
    if (self.animatorPrepTimer != nil) {
        [self.animatorPrepTimer invalidate];
    }
    
    // Dealloc OpenGL stuff
    
    if (textureCacheRef) {
        CFRelease(textureCacheRef);
        textureCacheRef = 0;
    }
    
    [self cancelDispatchTimer];
}

// This init method is invoked after the self reference is valid.

- (void) genericInitEAGLContext2:(EAGLContext*)context
{
    // Use 2x scale factor on Retina displays.
    //self.contentScaleFactor = [[UIScreen mainScreen] scale];
    
    // Set to NO to indicate that application will force redraws
    //self.enableSetNeedsDisplay = YES;
    //  self.enableSetNeedsDisplay = NO;
    
    self->textureCacheRef = NULL;
    
    self->didSetupOpenGLMembers = FALSE;
    
    // Set the default conversion to BT.709, which is the standard for HDTV.
    self->_preferredConversion = kColorConversion709;
    
    return;
}

- (id) initWithFrame:(CGRect)frame
{
    return self;
}

// Setup OpenGL objects and ids that need to be created only once, the first time
// the view is being rendered. Any OpenGL state that only needs to be set once
// for this context can be set here as long as it will not change from one render
// to the next.

- (BOOL) setupOpenGLMembers
{
    //	BOOL success = YES;
    //
    //	glDisable(GL_DEPTH_TEST);
    
    BOOL worked;
    
    //  Create a new CVOpenGLESTexture cache
    CVReturn err = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, self.context, NULL, &self->textureCacheRef);
    if (err) {
        NSLog(@"Error at CVOpenGLESTextureCacheCreate %d", err);
        worked = FALSE;
    } else {
        worked = TRUE;
    }
    
    return worked;
}

// Render the RGB and Alpha textures as the view via OpenGL draw

- (void) displayFrame
{
    //NSLog(@"displayFrame %@", frame);
    
    video_frame *rgbFrame = self.rgbFrame;
    
    // Weird case where putting app in background invokes this method and the frames are otherwise
    // valid but the buffer refs are NULL.
    
    CVImageBufferRef cvImageBufferRef = NULL;
    
    BOOL notReady = FALSE;
    BOOL wasReadyButCoreVideoBuffersInvalidated = FALSE;
    
    if (rgbFrame == nil)
    {
        notReady = TRUE;
    }
    
    if (notReady == FALSE) {
        cvImageBufferRef = rgbFrame.cvBufferRef;
        
        if (cvImageBufferRef == NULL)// || (cvAlphaImageBufferRef == NULL))
        {
            notReady = TRUE;
            wasReadyButCoreVideoBuffersInvalidated = TRUE;
        }
    }
    
    if (wasReadyButCoreVideoBuffersInvalidated) {
        // Nop, leave the existing cached render result as-is
        
        return;
    } else if (notReady) {
        //glClearColor(0.0, 0.0, 0.0, 0.0); // Fully transparent
        //glClear(GL_COLOR_BUFFER_BIT);
        
        return;
    }
    
    NSAssert(rgbFrame.isDuplicate == FALSE, @"a duplicate frame should not cause a display update");
    
    size_t frameWidth;
    size_t frameHeight;
    
    size_t bytesPerRow;
    
    // This OpenGL player view is only useful when decoding CoreVideo frames, it is possible
    // that a misconfiguration could result in a normal video_frame that contains a UIImage
    // getting passed to an OpenGL view. Simply assert here in that case instead of attempting
    // to support the non-optimal case since that would just cover up a configuration error
    // anyway.
    
    if (cvImageBufferRef == NULL) {
        NSAssert(FALSE, @"video_framevideo_frame does not contain a CoreVideo pixel buffer");
    }
    
    m_video_width = frameWidth = (unsigned int)CVPixelBufferGetWidth(cvImageBufferRef);
    m_video_height = frameHeight = (unsigned int)CVPixelBufferGetHeight(cvImageBufferRef);
    bytesPerRow = CVPixelBufferGetBytesPerRow(cvImageBufferRef);
    
    // Use the color attachment of the pixel buffer to determine the appropriate color conversion matrix.
    
    if (renderBGRA) {
        // BGRA is a nop
    } else {
        // YUV depends on colorspace conversion
        
        CFTypeRef colorAttachments = CVBufferGetAttachment(cvImageBufferRef, kCVImageBufferYCbCrMatrixKey, NULL);
        
#if defined(DEBUG)
        if (colorAttachments != NULL) {
            BOOL is240M = (CFStringCompare(colorAttachments, kCVImageBufferYCbCrMatrix_SMPTE_240M_1995, 0) == kCFCompareEqualTo);
            assert(is240M == 0);
        }
#endif // DEBUG
        
        if (colorAttachments == NULL || (CFStringCompare(colorAttachments, kCVImageBufferYCbCrMatrix_ITU_R_601_4, 0) == kCFCompareEqualTo)) {
            _preferredConversion = kColorConversion601;
        } else if (CFStringCompare(colorAttachments, kCVImageBufferYCbCrMatrix_ITU_R_709_2, 0) == kCFCompareEqualTo) {
            _preferredConversion = kColorConversion709;
        } else {
            NSLog(@"unsupported colorspace attachment \"%@\"", colorAttachments);
            assert(0);
        }
    }
    
    if (self->textureCacheRef == NULL) {
        // This should not actually happen, but no specific way to deal with an error here
        return;
    }
    
    // Allocate a "texture ref" object that wraps around the existing memory allocated and written
    // by CoreVideo. As far as OpenGL is concerned, this is a new texture, but the memory that
    // backs the texture has already been fully written to at this point. The OpenGL id for the
    // texture changes from one frame to the next and CoreVideo keeps track of the specific
    // buffer used when the frame was decoded.
    
    self->textureRef = NULL;
    
    self->textureUVRef = NULL;
    
    self->textureAlphaRef = NULL;
    
    CVReturn err;
    
    if (renderBGRA) {
        
        glActiveTexture(GL_TEXTURE0);
        
        // The RGB pixel values are stored in a BGRX frame
        
        err = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                           self->textureCacheRef,
                                                           cvImageBufferRef,
                                                           (CFDictionaryRef) NULL,
                                                           GL_TEXTURE_2D, // not GL_RENDERBUFFER
                                                           GL_RGBA,
                                                           (GLsizei)frameWidth,
                                                           (GLsizei)frameHeight,
                                                           GL_BGRA,
                                                           GL_UNSIGNED_BYTE,
                                                           0,
                                                           &textureRef);
        
        if (textureRef == NULL) {
            NSLog(@"CVOpenGLESTextureCacheCreateTextureFromImage failed and returned NULL (error: %d)", err);
            return;
        }
        
        if (err) {
            if (textureRef) {
                CFRelease(textureRef);
            }
            NSLog(@"CVOpenGLESTextureCacheCreateTextureFromImage failed (error: %d)", err);
            return;
        }
        
        // Bind texture, OpenGL already knows about the texture but it could have been created
        // in another thread and it has to be bound in this context in order to sync the
        // texture for use with this OpenGL context. The next logging line can be uncommented
        // to see the actual texture id used internally by OpenGL.
        
        //NSLog(@"bind OpenGL texture %d", CVOpenGLESTextureGetName(textureRef));
        
        glBindTexture(CVOpenGLESTextureGetTarget(textureRef), CVOpenGLESTextureGetName(textureRef));
        
        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
    } else {
        // Y is an 8 bit texture
        
        glActiveTexture(GL_TEXTURE0);
        
        err = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                           self->textureCacheRef,
                                                           cvImageBufferRef,
                                                           (CFDictionaryRef) NULL,
                                                           GL_TEXTURE_2D, // not GL_RENDERBUFFER
                                                           GL_RED_EXT,
                                                           (GLsizei)frameWidth,
                                                           (GLsizei)frameHeight,
                                                           GL_RED_EXT,
                                                           GL_UNSIGNED_BYTE,
                                                           0,
                                                           &textureRef);
        
        if (textureRef == NULL) {
            NSLog(@"CVOpenGLESTextureCacheCreateTextureFromImage failed and returned NULL (error: %d)", err);
            return;
        }
        
        if (err) {
            if (textureRef) {
                CFRelease(textureRef);
            }
            NSLog(@"CVOpenGLESTextureCacheCreateTextureFromImage failed (error: %d)", err);
            return;
        }
        
        //NSLog(@"bind OpenGL Y texture %d", CVOpenGLESTextureGetName(textureRef));
        
        GLenum tex_y_target = CVOpenGLESTextureGetTarget(textureRef);
        m_tex_y_gl_id = CVOpenGLESTextureGetName(textureRef);
        glBindTexture(tex_y_target, m_tex_y_gl_id);
        
        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        // UV is an interleaved texture that is upsampled to the Y size in OpenGL
        
        glActiveTexture(GL_TEXTURE1);
        
        err = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                           self->textureCacheRef,
                                                           cvImageBufferRef,
                                                           (CFDictionaryRef) NULL,
                                                           GL_TEXTURE_2D, // not GL_RENDERBUFFER
                                                           GL_RG_EXT,
                                                           (GLsizei)frameWidth/2,
                                                           (GLsizei)frameHeight/2,
                                                           GL_RG_EXT,
                                                           GL_UNSIGNED_BYTE,
                                                           1,
                                                           &textureUVRef);
        
        if (textureUVRef == NULL) {
            NSLog(@"CVOpenGLESTextureCacheCreateTextureFromImage failed and returned NULL (error: %d)", err);
            return;
        }
        
        if (err) {
            if (textureUVRef) {
                CFRelease(textureUVRef);
            }
            NSLog(@"CVOpenGLESTextureCacheCreateTextureFromImage failed (error: %d)", err);
            return;
        }
        
        //NSLog(@"bind OpenGL Y texture %d", CVOpenGLESTextureGetName(textureUVRef));
        
        GLenum tex_uv_target = CVOpenGLESTextureGetTarget(textureUVRef);
        m_tex_uv_gl_id = CVOpenGLESTextureGetName(textureUVRef);
        glBindTexture(tex_uv_target, m_tex_uv_gl_id);
        
        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}

- (void) end_frame
{
    // Flush the CVOpenGLESTexture cache and release the texture.
    // This logic does not deallocate the "texture", it just deallocates the
    // CoreVideo object wrapper for the texture.
    
    CVOpenGLESTextureCacheFlush(self->textureCacheRef, 0);
    
    if (self->textureRef)
    {
        CFRelease(self->textureRef);
        self->textureRef = NULL;
    }
    
    if (self->textureUVRef)
    {
        CFRelease(self->textureUVRef);
        self->textureUVRef = NULL;
    }
    
    if (self->textureAlphaRef)
    {
        CFRelease(self->textureAlphaRef);
        self->textureAlphaRef = NULL;
    }
}

// drawRect from UIView, this method is invoked because this view extends GLKView

- (void)drawRect
{
    //NSLog(@"drawRect %dx%d", (int)rect.size.width, (int)rect.size.height);
    //NSLog(@"drawable width x height %dx%d", (int)self.drawableWidth, (int)self.drawableHeight);
    
    if (didSetupOpenGLMembers == FALSE) {
        didSetupOpenGLMembers = TRUE;
        BOOL worked = [self setupOpenGLMembers];
        NSAssert(worked, @"setupOpenGLMembers failed");
    }
    
    [self displayFrame];
}

- (const float*) preferred_conversion
{
    return _preferredConversion;
}

#pragma mark -  Animation cycle

// Map a time offset to a number of frames at the end of the next frame interval
//
// Assume 2 FPS so that frame duration is 0.5s

// 0.0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.75 0.8 0.9 1.0
//   1   1   1   1   1   1   1   1    1   2   2   2

+ (uint32_t) timeIntervalToFrameOffset:(CFTimeInterval)elapsed
                                   fps:(CFTimeInterval)fps
{
    const BOOL debug = FALSE;
    
    uint32_t frameOffset;
    
    float frameF = elapsed * fps;
    
    if (debug) {
        NSLog(@"frameF %0.5f", frameF);
    }
    
    frameOffset = (uint32_t)round(frameF);
    
    if (debug) {
        NSLog(@"elapsed time %0.3f with frameDuration %0.3f -> frame number %d", elapsed, 1.0/fps, frameOffset);
    }
    
    if (frameOffset == 0) {
        frameOffset = 1;
    }
    
    if (debug) {
        NSLog(@"return frame offset %d", frameOffset);
    }
    
    return frameOffset;
}

// Create GCD repeating timer which is staggered so that decoding of the next
// frame starts right after the display interval.

- (void) makeDispatchTimer:(double)inInterval
                     queue:(dispatch_queue_t)queue
                     block:(dispatch_block_t)block
{
#if defined(DEBUG)
    NSAssert(queue, @"queue");
#endif // DEBUG
    
    dispatch_source_t timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue);
    
    self->dispatchFirstTimeInterval = 0.0;
    
    if (timer)
    {
        double firstDelayInterval = (1.0/100.0);
        
        dispatch_time_t start = dispatch_time(DISPATCH_TIME_NOW, firstDelayInterval * NSEC_PER_SEC);
        uint64_t interval = inInterval * NSEC_PER_SEC;
        uint64_t leeway = (1ull * NSEC_PER_SEC) / 10;
        
        dispatch_source_set_timer(timer, start, interval, leeway);
        dispatch_source_set_event_handler(timer, block);
        dispatch_resume(timer);
    }
    
    self->_dispatchTimer = timer;
    
    return;
}

- (void) cancelDispatchTimer
{
    if (self->_dispatchTimer) {
        dispatch_source_cancel(self->_dispatchTimer);
#if OS_OBJECT_HAVE_OBJC_SUPPORT == 0
        // Remove this if you are on a Deployment Target of iOS6 or OSX 10.8 and above
        dispatch_release(self->_dispatchTimer);
#endif // OS_OBJECT_HAVE_OBJC_SUPPORT
        self->_dispatchTimer = NULL;
    }
}

// Invoked on GCD queue at a repeating interval such that the decoding will be finished *AFTER*
// the next display interval.

- (void) dispatchTimerFired
{
    const BOOL debugPrintTimes = FALSE;
    
#if defined(DEBUG)
    assert([NSThread currentThread] != [NSThread mainThread]);
#endif // DEBUG
    
    if (self.waitingForDecodeToStart) {
        self.waitingForDecodeToStart = 0;
    }
    
    // The first and second timer invocations can be a little odd sometimes.
    // The spacing between timer callbacks should be about (1.0/30) = 0.03
    // but sometimes the second invocation is delivered right after the first
    // one at a delta like 0.01. This would push the decoder one ahead if
    // not ignored.
    
    CFTimeInterval nowTime = CACurrentMediaTime();
    
    if (self->dispatchFirstTimeInterval == 0.0) {
        self->dispatchFirstTimeInterval = nowTime;
        self->dispatchPrevTimeInterval = 0.0;
    }
    
    CFTimeInterval startTime = self->dispatchFirstTimeInterval;
    
    CFTimeInterval prev = self->dispatchPrevTimeInterval;
    if (prev == 0.0) {
        prev = nowTime;
    }
    
    CFTimeInterval deltaLastTime = nowTime - prev;
    CFTimeInterval elapsedTime = (nowTime - startTime);
    
    if (debugPrintTimes) {
        NSLog(@"dispatchTimerFired : now %0.3f : start %0.3f : elapsed %0.3f : since prev %0.3f", nowTime, startTime, elapsedTime, deltaLastTime);
    }
    
    self->dispatchPrevTimeInterval = nowTime;
    
//    const CFTimeInterval halfExpectedInterval = (1.0/frames_per_second) / 2; // About 60 FPS
//    //const CFTimeInterval halfExpectedInterval = (1.0/30) / 2; // About 60 FPS
//    const CFTimeInterval tooSmallMaxInterval = halfExpectedInterval + (halfExpectedInterval / 8);
//
//    if (m_currentFrame == 4 && elapsedTime <= tooSmallMaxInterval) {
//        // Interval too small
//
//        if (debugPrintTimes) {
//            NSLog(@"second callback too soon");
//        }
//
//        return;
//    }
//
    //  if ((prev != 0) && ((nowTime - startTime) > 10.0)) {
    //    [self stopAnimator];
    //    return;
    //  }
    
    BOOL done = [self dispatchDecodeFrame];
    
    if (done) {
        [self cancelDispatchTimer];
        
        dispatch_sync(dispatch_get_main_queue(), ^{
            [self stopAnimator];
            
            [[NSNotificationCenter defaultCenter] postNotificationName:vplayer_did_stop_notif object:self];
        });
    }
}

// This method implements the tricky thread handoff logic that determines
// the next frame to display and decodes that frame. This logic has to check
// in with state from the main thread.
//
// Returns TRUE when all frames have been decoded.

- (BOOL) dispatchDecodeFrame
{
    const BOOL debugDecodeFrames = FALSE;
    
    __block int currentFrame = self.currentFrame; // atomic
    
    if (debugDecodeFrames) {
        NSLog(@"dispatchDecodeFrame invoked with currentFrame %d (aka %d in combined frames)", currentFrame, currentFrame/2);
    }
    
    __block int aheadButReallyDone = 0;
    
    int maxFrame = self.dispatchMaxFrame;
    
    if (currentFrame >= maxFrame) {
        if (debugDecodeFrames) {
            NSLog(@"dispatchDecodeFrame : done processing frames at %d", currentFrame);
        }
        
        return TRUE;
    }
    
    video_dec_player *weakSelf = self;
    
    video_frame* rgbFrame;
    
    if (debugDecodeFrames) {
        NSLog(@"advanceToFrame %d of %d (aka %d in combined frames)", currentFrame, maxFrame, currentFrame/2);
    }
    
    CFTimeInterval beforeTime = CACurrentMediaTime();
    
    if ((0)) {
        NSLog(@"decode start time       %0.5f", beforeTime);
    }
    
    int nextFrame = [self.class loadFramesInBackgroundThread:currentFrame
                                                frameDecoder:self.frameDecoder
                                                    rgbFrame:&rgbFrame];
    
    CFTimeInterval afterTime = CACurrentMediaTime();
    CFTimeInterval delta = afterTime - beforeTime;
    
    if ((0)) {
        NSLog(@"decode after time       %0.5f", afterTime);
    }
    
    if ((0)) {
        NSLog(@"decode delta %0.3f", delta);
    }
    
    dispatch_sync(dispatch_get_main_queue(), ^{
#if defined(DEBUG)
        NSAssert(rgbFrame, @"rgbFrame");
#endif // DEBUG
        
        __strong video_dec_player *strongSelf = weakSelf;
        
        if (strongSelf.state != ANIMATING) {
            // stopAnimator invoked after startAnimator
            currentFrame = maxFrame;
        } else {
            if (debugDecodeFrames) {
                NSLog(@"deliver to main time      %0.5f", CACurrentMediaTime());
            }
            
            [strongSelf deliverRGBAndAlphaFrames:nextFrame rgbFrame:rgbFrame];
            
            // Get the frame number for the next frame in terms of the combined frames
            
#if defined(DEBUG)
            //assert((nextFrame % 2) == 0);
#endif // DEBUG
            int nextCombinedFrameToDecode = nextFrame;// >> 1; // div 2
            
            if (debugDecodeFrames) {
                NSLog(@"decoder nextCombinedFrameToDecode %d as compared to nextDecodeFrame %d", nextCombinedFrameToDecode, nextDecodeFrame);
            }
            
            if (nextCombinedFrameToDecode < nextDecodeFrame) {
                if (debugDecodeFrames) {
                    NSLog(@"decoder current combined frame is behind by %d combined frames", nextDecodeFrame - nextCombinedFrameToDecode);
                }
                
                int lastDecodedFrame = currentFrame;
                currentFrame = nextDecodeFrame;// * 2;
                
                // Skip ahead, but don't skip over the last frame in the interval
                
                if (currentFrame >= maxFrame) {
                    int actualLastFrame = maxFrame - 2;
                    if (lastDecodedFrame == actualLastFrame) {
                        // When the previously decoded frame was the last frame then
                        // decode cycle is completed.
                        
                        aheadButReallyDone = 1;
                    } else {
                        // When skipping ahead, skip to the last frame in the animation cycle.
                        
                        currentFrame = actualLastFrame;
                    }
                }
            }
            else {
                currentFrame = nextFrame;
            }
        }
    });
    
    if (aheadButReallyDone) {
        if (debugDecodeFrames) {
            NSLog(@"dispatchDecodeFrame : done processing frames at %d", currentFrame);
        }
        
        return TRUE;
    } else {
        // Write currentFrame back to self.currentFrame
        self.currentFrame = currentFrame;
        
        if (debugDecodeFrames) {
            NSLog(@"dispatchDecodeFrame : NOT done processing frames at %d", currentFrame);
        }
        return FALSE;
    }
}

// Kick off repeating GCD timer invocation

- (void) startDispatchRender {
#if defined(DEBUG)
    assert([NSThread currentThread] == [NSThread mainThread]);
#endif // DEBUG
    
    __weak video_dec_player *weakSelf = self;
    
    // Note that the dispatch time depends on the display framerate
    // so that the decode event is always just after the frame display
    
    const CFTimeInterval kFrameDuration = frame_duration; // 30 FPS display refresh rate
    
    [self makeDispatchTimer:kFrameDuration
                      queue:self->_highPrioQueue
                      block:^{
                          [weakSelf dispatchTimerFired];
                      }];
    
    // dispatchMaxFrame should have been set at asset load time
#if defined(DEBUG)
    assert(self.dispatchMaxFrame > 0);
#endif // DEBUG
}

// This display link callback is invoked at fixed interval while animation is running.

- (void) displayLinkCallback:(CADisplayLink*)displayLink {
    const BOOL debugDisplayLink = FALSE;
    const BOOL debugDisplayRedrawn = FALSE;
    
    //NSLog(@"displayLinkCallback %f", (float)displayLink.timestamp);
    // Note that frame duration is 1/60 but the interval is 2 so 1/30 a second refresh rate
    
    if (debugDisplayLink) {
        CFTimeInterval effectiveDuration = displayLink.duration * displayLink.frameInterval;
        NSLog(@"displayLinkCallback with timestamp %0.4f and frame duration %0.4f (interval %0.4f)", displayLink.timestamp, effectiveDuration, displayLink.duration);
    }
    
    // Actual framerate of the video, note that the calculated framerate might
    // be slightly different than the screen refresh rate.
    
    const CFTimeInterval kFramesPerSecond = frames_per_second;
    const CFTimeInterval kFrameDuration = frame_duration;
    
    // Note that first image can be visible for 2 cycles since the first
    // callback is invoked on a screen sync and then decoding starts
    // after that.
    
    if (firstTimeInterval == 0) {
        firstTimeInterval = displayLink.timestamp;
        [self startDispatchRender];
        
        // Do not calculate frame offset or redraw on first invocation
        
        return;
    }
    
    if (nextDecodeFrame == 0 && self.waitingForDecodeToStart) {
        // If the first call to dispatchTimerFired has not happened yet then
        // simply reset the firstTimeInterval and continue to wait for
        // a sync time between the display callback and decode thread.
        
        firstTimeInterval = displayLink.timestamp;
        
        if (debugDisplayLink) {
            NSLog(@"waiting on self.waitingForDecodeToStart");
        }
        
        return;
    }
    
    CFTimeInterval elapsed = (displayLink.timestamp - firstTimeInterval);
    
    if (debugDisplayLink) {
        NSLog(@"elapsed %0.3f", elapsed);
    }
    
    int prevNextDecodeFrame = nextDecodeFrame;
    
    if (debugDisplayLink) {
        NSLog(@"previous nextDecodeFrame %d", prevNextDecodeFrame);
    }
    
    CFTimeInterval next_frame_idx = elapsed * kFramesPerSecond;
    nextDecodeFrame = (int)next_frame_idx;
    //NSLog(@"elapsed %0.5f nextDecodeFrame %d", elapsed, nextDecodeFrame);

    if (debugDisplayLink && prevNextDecodeFrame == nextDecodeFrame) {
        NSLog(@"repeated decoded frame");
    }
    
    // Each display link invocation will schedule a redraw, the result is that
    // a smooth 30 FPS video rate is maintained.
    
    if (debugDisplayRedrawn) {
        NSLog(@"disp now                %0.5f", displayLink.timestamp);
        NSLog(@"nextDecodeFrame = %d", nextDecodeFrame);
        NSLog(@"called setNeedsDisplay");
    }
    
    //[self setNeedsDisplay];
    
    //  if (lastDisplayLinkFrameOffset > 10) {
    //     NSLog(@"10");
    //  }
    
    return;
}

- (void)setupDisplayLink {
#if defined(DEBUG)
    assert([NSThread currentThread] == [NSThread mainThread]);
#endif // DEBUG
    
    CADisplayLink *displayLink = self.displayLink;
    if (displayLink != nil) {
        [displayLink removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    }
    displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(displayLinkCallback:)];
    //displayLink.frameInterval = 2; // 30 FPS
    displayLink.preferredFramesPerSecond = 60;
    // FIXME : NSDefaultRunLoopMode vs common mode?
    [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    self.displayLink = displayLink;
    
    firstTimeInterval = 0.0;
    nextDecodeFrame = 0;
}

- (void) startAnimator
{
#if defined(DEBUG)
    assert([NSThread currentThread] == [NSThread mainThread]);
#endif // DEBUG
    
    if (self.rgbFrame == nil)
    {
        NSAssert(FALSE, @"player must be prepared before startAnimator can be invoked");
    }
    
    NSAssert(self.state == READY || self.state == STOPPED, @"player must be prepared before startAnimator can be invoked");
    
    self.state = ANIMATING;
    
    __block int currentFrame = self.currentFrame;
    __block int maxFrame = self.dispatchMaxFrame;
    
#if defined(DEBUG)
    NSAssert(self.dispatchMaxFrame > 0, @"player must be prepared before startAnimator can be invoked");
#endif // DEBUG
    
    if (currentFrame >= maxFrame) {
        // In the case of only 2 frames, stop straight away without kicking off background thread, useful for testing
        self.state = STOPPED;
        return;
    }
    
    [self setupDisplayLink];
    self.displayLink.paused = FALSE;
    
    self.waitingForDecodeToStart = 1;
    
#if defined(DEBUG)
    CFTimeInterval nowTime = CACurrentMediaTime();
    NSLog(@"startAnimator : now %0.3f", nowTime);
#endif // DEBUG
}

- (void) stopAnimator
{
#if defined(DEBUG)
    assert([NSThread currentThread] == [NSThread mainThread]);
#endif // DEBUG
    
    self.rgbFrame = nil;
    
    self.displayLink.paused = TRUE;
    
    [self cancelDispatchTimer];
    
    self.state = STOPPED;
}

// Invoke this method to read from the named asset and being loading initial data

- (void) prepareToAnimate
{
#if defined(DEBUG)
    assert([NSThread currentThread] == [NSThread mainThread]);
#endif // DEBUG
    
    self.animatorPrepTimer = [NSTimer timerWithTimeInterval: 1.0/60
                                                     target: self
                                                   selector: @selector(_prepareToAnimateTimer:)
                                                   userInfo: NULL
                                                    repeats: FALSE];
    
    [[NSRunLoop currentRunLoop] addTimer: self.animatorPrepTimer forMode: NSDefaultRunLoopMode];
    
    self.currentFrame = -1;
    
    self.state = PREPPING;
}

// This method delivers the RGB and Alpha frames to the view in the main thread

- (void) deliverRGBAndAlphaFrames:(int)nextFrame
                         rgbFrame:(video_frame*)rgbFrame
{
    self.rgbFrame = rgbFrame;
    self.currentFrame = nextFrame;
}

// This timer callback method is invoked after the event loop is up and running in the
// case where prepareToAnimate is invoked as part of the app startup via viewDidLoad.

- (void) _prepareToAnimateTimer:(NSTimer*)timer
{
    if(!m_assetFilename)
    {
        return;
    }
    
    if (self->_highPrioQueue == nil) {
        self->_highPrioQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0);
        assert(self->_highPrioQueue);
    }
    
    self.currentFrame = 0;
    self.frameDecoder = [frame_decoder frame_dec];
    
    __block int currentFrame = self.currentFrame;
    __block Class c = self.class;
    
    __weak video_dec_player *weakSelf = self;
    
    dispatch_async(self->_highPrioQueue, ^{
        // Execute on background thread with blocks API invocation
        
        __strong video_dec_player *strongSelf = weakSelf;
        
        frame_decoder *frameDecoder = strongSelf.frameDecoder;
        assert(frameDecoder);
        
        // Configure frame decoder flags
        
        frameDecoder.dropFrames = TRUE;
        
        frameDecoder.produceCoreVideoPixelBuffers = TRUE;
        
        if (renderBGRA) {
        } else {
            frameDecoder.produceYUV420Buffers = TRUE;
        }
        
        NSAssert(strongSelf.assetFilename, @"assetFilename must be defined when prepareToAnimate is invoked");
        
        NSString *assetFullPath = [video_file_util getQualifiedFilenameOrResource:strongSelf.assetFilename];
        
        BOOL worked;
        worked = [frameDecoder openForReading:assetFullPath];
        
        if (worked == FALSE) {
            NSLog(@"error: cannot open RGB+Alpha mixed asset filename \"%@\"", assetFullPath);
            
            [[NSNotificationCenter defaultCenter] postNotificationName:vplayer_failed_to_load_notif
                                                                object:strongSelf];
            return;
        }
        
        frames_per_second = frameDecoder.nominalFrameRate;
        frame_duration = frameDecoder.frameDuration;
        worked = [frameDecoder allocateDecodeResources];
        
        if (worked == FALSE) {
            NSLog(@"error: cannot allocate RGB+Alpha mixed decode resources for filename \"%@\"", assetFullPath);
            return;
            //    return FALSE;
        }
        
        // Verify that the total number of frames is even since RGB and ALPHA frames must be matched.
        
        int numFrames = (int) frameDecoder.numFrames;
        NSLog(@"\n\n\nFrame count: %d\n\n\n", numFrames);
        
        // Set the dispatchMaxFrame field. Note that in some weird cases the
        // Simulator returns a nonsense result with an odd number of frames,
        // so set the max to a specific even number of frames so that the
        // simulator is able to run something.
        
        strongSelf.dispatchMaxFrame = numFrames;
        
        video_frame* rgbFrame;
        
        int nextFrame = [c loadFramesInBackgroundThread:currentFrame
                                           frameDecoder:frameDecoder
                                               rgbFrame:&rgbFrame];
        
        dispatch_sync(dispatch_get_main_queue(), ^{
#if defined(DEBUG)
            NSAssert(rgbFrame, @"rgbFrame");
#endif // DEBUG
            
            __strong video_dec_player *strongSelf = weakSelf;
            
            if (strongSelf.state == STOPPED) {
                // stopAnimator invoked after prepareToAnimate
            } else {
                [strongSelf deliverRGBAndAlphaFrames:nextFrame rgbFrame:rgbFrame];
                
                //[strongSelf setNeedsDisplay];
                
                strongSelf.state = READY;
                
                [[NSNotificationCenter defaultCenter] postNotificationName:vplayer_prepared_to_start_notif
                                                                    object:strongSelf];
            }
        });
    });
    
    return;
}

+ (int) loadFramesInBackgroundThread:(int)currentFrame
                        frameDecoder:(frame_decoder*)frameDecoder
                            rgbFrame:(video_frame**)rgbFramePtr
{
    video_frame *rgbFrame;
    
    rgbFrame = [frameDecoder advanceToFrame:currentFrame];
    currentFrame++;
    
    *rgbFramePtr = rgbFrame;
    
    return currentFrame;
}

@end


@implementation video_file_util

// Given a filename (typically an archive entry name), return the filename
// in the tmp dir that corresponds to the entry. For example,
// "2x2_black_blue_16BPP.mvid" -> "/tmp/2x2_black_blue_16BPP.mvid" where "/tmp"
// is the app tmp dir.

+ (NSString*) getTmpDirPath:(NSString*)filename
{
    NSString *tmpDir = NSTemporaryDirectory();
    NSAssert(tmpDir, @"tmpDir");
    NSString *outPath = [tmpDir stringByAppendingPathComponent:filename];
    NSAssert(outPath, @"outPath");
    return outPath;
}

+ (BOOL) fileExists:(NSString*)path
{
    return [[NSFileManager defaultManager] fileExistsAtPath:path];
}

+ (NSString*) getResourcePath:(NSString*)resFilename
{
    NSBundle* appBundle = [NSBundle mainBundle];
    NSString* movieFilePath = [appBundle pathForResource:resFilename ofType:nil];
    NSAssert(movieFilePath, @"movieFilePath is nil");
    return movieFilePath;
}

+ (NSString*) generateUniqueTmpPath
{
    NSString *tmpDir = NSTemporaryDirectory();
    
    NSDate *nowDate = [NSDate date];
    NSTimeInterval ti = [nowDate timeIntervalSinceReferenceDate];
    
    // Format number of seconds as a string with a decimal separator
    NSString *doubleString = [NSString stringWithFormat:@"%f", ti];
    
    // Remove the decimal point so that the file name consists of
    // numeric characters only.
    
    NSRange range = NSMakeRange(0, [doubleString length]);
    
    NSString *noDecimalString = [doubleString stringByReplacingOccurrencesOfString:@"."
                                                                        withString:@""
                                                                           options:0
                                                                             range:range];
    
    NSString *tmpPath = [tmpDir stringByAppendingPathComponent:noDecimalString];
    
    return tmpPath;
}

+ (NSString*) getQualifiedFilenameOrResource:(NSString*)filename
{
    if ([filename hasPrefix:@"/"]) {
        if ([[NSFileManager defaultManager] fileExistsAtPath:filename isDirectory:NULL]) {
            return filename;
        } else {
            return nil;
        }
    } else {
        return [video_file_util getResourcePath:filename];
    }
}

+ (void) renameFile:(NSString*)path toPath:(NSString*)toPath
{
    // The temp filename holding the maxvid data is now completely written, rename it to "XYZ.mvid"
    
    NSError *error = nil;
    BOOL worked;
    
    if ([[NSFileManager defaultManager] fileExistsAtPath:toPath]) {
        worked = [[NSFileManager defaultManager] removeItemAtPath:toPath error:&error];
        NSAssert(worked, @"removeItemAtPath failed : %@", error);
    }
    
#if defined(DEBUG)
    BOOL existedBefore = [[NSFileManager defaultManager] fileExistsAtPath:path];
    NSAssert(existedBefore, @"src file does not exist : %@", path);
    BOOL existedToToPathBefore = [[NSFileManager defaultManager] fileExistsAtPath:toPath];
    NSAssert(existedToToPathBefore == FALSE, @"dst file must not exist : %@", toPath);
#endif // DEBUG
    
    error = nil;
    worked = [[NSFileManager defaultManager] moveItemAtPath:path toPath:toPath error:&error];
    if (!worked) {
        BOOL exists = [[NSFileManager defaultManager] fileExistsAtPath:path];
        NSAssert(exists, @"src file does not exist : %@", path);
        NSAssert(worked, @"moveItemAtPath failed for decode result : %@", error);
    }
    
    return;
}

@end
