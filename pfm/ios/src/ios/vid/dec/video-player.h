#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>
#import <Foundation/Foundation.h>

#import "frame-decoder.h"

// These notifications are delived from the video player

#define vplayer_prepared_to_start_notif @"vplayer_prepared_to_start_notif"
#define vplayer_failed_to_load_notif @"vplayer_failed_to_load_notif"

#define vplayer_did_start_notif @"vplayer_did_start_notif"
#define vplayer_did_stop_notif @"vplayer_did_stop_notif"


@interface video_dec_player : NSObject //GLKView

// Set this property to indicate the name of the asset to be
// loaded as a result of calling startAnimator.

@property (atomic, copy) NSString *assetFilename;

@property (atomic, retain) frame_decoder *frameDecoder;
@property (strong, nonatomic) EAGLContext *context;
@property (readonly, nonatomic) unsigned int video_width;
@property (readonly, nonatomic) unsigned int video_height;
@property (readonly, nonatomic) unsigned int tex_y_gl_id;
@property (readonly, nonatomic) unsigned int tex_uv_gl_id;

// In DEBUG mode, this property can be set to a directory and each rendered
// output frame will be captured as BGRA and saved in a PNG.

#if defined(DEBUG)
@property (nonatomic, copy) NSString *captureDir;
#endif // DEBUG

- (void) drawRect;
- (void) end_frame;
- (const float*) preferred_conversion;

// Invoke this metho to read from the named asset and being loading initial data

- (void) prepareToAnimate;

// After an animator has been prepared and the vplayer_prepared_to_start_notif has
// been delivered this startAnimator API can be invoked to actually kick off the playback loop.

- (void) startAnimator;

// Stop playback of animator, nop is not currently animating

- (void) stopAnimator;

@end


@interface video_file_util : NSObject {
    
}

// Given a filename like "foo.txt", return the fully qualified path
// in the temp dir (like "/tmp/foo.txt"). The tmp dir is the app defined
// temp directory.

+ (NSString*) getTmpDirPath:(NSString*)filename;

// Return TRUE if a file exists with the indicated path, FALSE otherwise.

+ (BOOL) fileExists:(NSString*)path;

// Return the path for a resource file

+ (NSString*) getResourcePath:(NSString*)resFilename;

// Return a fully qualified path to a unique filename in the tmp dir.
// This filename will not be the same as a previously used filename
// and it will not conflict with an existing file in the tmp dir.
// This method should only be invoked from the main thread, in order
// to ensure that there are no thread race conditions present when
// generating the tmp filename.

+ (NSString*) generateUniqueTmpPath;

// If the filename is fully qualified, then check that the file
// exists and return nil if it does not exist. If the filename
// is a simple filename, then check that a resource file with
// that filename exists and return nil if the resource does not exist.
// In either case, a fully qualified path of a file that is known
// to exist is returned, otherwise nil.

+ (NSString*) getQualifiedFilenameOrResource:(NSString*)filename;

// Rename the file at path to the file indicated by toPath.
// This util method will remove an existing file at toPath
// and assert that the move operation was successful.

+ (void) renameFile:(NSString*)path toPath:(NSString*)toPath;

@end
