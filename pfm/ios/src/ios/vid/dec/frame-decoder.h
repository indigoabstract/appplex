#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>

@class video_frame;
@class video_fb;
@class AVAssetReader;
@class AVAssetReaderOutput;

@interface frame_decoder : NSObject
{
@private
  NSURL *m_assetURL;
  AVAssetReader *m_aVAssetReader;
    AVAssetReaderOutput *m_aVAssetReaderOutput;
    AVAssetReaderOutput *m_aVAssetAudioReaderOutput;

  NSTimeInterval m_frameDuration;
    NSTimeInterval m_nominalFrameRate;
  NSUInteger     m_numFrames;
  int            frameIndex;
  
  CGSize detectedMovieSize;
  float prevFrameDisplayTime;
  int numTrailingNopFrames;
  
  BOOL m_isOpen;
  BOOL m_isReading;
  BOOL m_readingFinished;
  BOOL m_produceCoreVideoPixelBuffers;
  BOOL m_produceYUV420Buffers;
  BOOL m_dropFrames;
}

@property (nonatomic, readonly) NSUInteger  numFrames;

// If this flag is set to TRUE (the default is FALSE) then each video_frame produced
// by this frame decoder will contain a CVPixelBufferRef instead of an
// image. This optimized path avoids multiple pointless framebuffer copies.
// Because video data can be very very large, this optimal execution path can
// save a significant amount of execution time, but it is only valid if the rendering
// target is able to accept a CoreVideo pixel buffer directly.
// This value should not be changed while actually decoding frames, it should
// only be set before rendering of frames begins.

@property (nonatomic, assign) BOOL produceCoreVideoPixelBuffers;

// If self.produceCoreVideoPixelBuffers is TRUE and this flag is set then the
// H264 decoder will produce YUV 4:2:0 buffers as opposed to a flat BGRA buffer.
// A 4:2:0 features subsampling for the UV components.

@property (nonatomic, assign) BOOL produceYUV420Buffers;

// This flag defaults to TRUE, when it is TRUE then the decoder will drop
// a frame if the indicated display time less than the expected interval
// time to the next frame. Note that this frame drop logic is not always
// correct, in the case where a video encodes a frame at a time and no
// frame should be dropped based on the timing info, then this flag should
// be set to FALSE.

@property (nonatomic, assign) BOOL dropFrames;

+ (frame_decoder*) frame_dec;

// Open resource identified by path

- (BOOL) openForReading:(NSString*)path;

// Close resource opened earlier

- (void) close;

// Reset current frame index to -1, before the first frame

- (void) rewind;

// Advance the current frame index to the indicated frame index. Return the new frame
// object, the frame contains the platform specific image object. If a duplicate (no-op)
// frame is found then the frame data has not changed. A no-op frame is indicated by
// the frame.isDuplicate property being set to TRUE. Note that this advanceToFrame
// method should never return nil, even in the case where the frame data cannot be
// loaded, a valid video_frame should be returned with a nil image property to indicate failure.

- (video_frame*) advanceToFrame:(NSUInteger)newFrameIndex;

// Decoding frames may require additional resources that are not required
// to open the file and examine the header contents. This method will
// allocate decoding resources that are required to actually decode the
// video frames from a specific file. It is possible that allocation
// could fail, for example if decoding would require too much memory.
// The caller would need to check for a FALSE return value to determine
// how to handle the case where allocation of decode resources fails.

- (BOOL) allocateDecodeResources;

- (void) releaseDecodeResources;

// Returns TRUE if resources are "limited" meaning decode resource are not allocated.

- (BOOL) isResourceUsageLimit;

// Return a copy of the last frame returned via advanceToFrame.
// This copy will not be associated with the frame decoder and
// it will not contain any external references to shared memory.
// This method is useful only for the case where holding onto a
// ref to the final frame will waste significant resources, for
// example if the normal frames hold references to mapped memory.

- (video_frame*) duplicateCurrentFrame;

// Properties

// Dimensions of each frame
- (NSUInteger) width;
- (NSUInteger) height;

// True when resource has been opened via openForReading
- (BOOL) isOpen;

// Total frame count
- (NSUInteger) numFrames;

// Current frame index, can be -1 at init or after rewind
- (NSInteger) frameIndex;

// Time each frame shold be displayed
- (NSTimeInterval) nominalFrameRate;

/*!
 @property        nominalFrameRate
 @abstract        For tracks that carry a full frame per media sample, indicates the frame rate of the track in units of frames per second.
 @discussion        For field-based video tracks that carry one field per media sample, the value of this property is the field rate, not the frame rate.
 */
- (NSTimeInterval) frameDuration;

// TRUE if the decoded frame supports and alpha channel.
- (BOOL) hasAlphaChannel;

// True when all mvid frames are keyframes. This means that none of
// the frames are delta frames that require a previous state in order
// to apply a delta.

- (BOOL) isAllKeyframes;

@end

