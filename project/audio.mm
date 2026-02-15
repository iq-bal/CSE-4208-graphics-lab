#include "audio.h"
#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>

static AVAudioPlayer *bgPlayer = nil;

void startBackgroundMusic(const char *filePath) {
  @autoreleasepool {
    NSString *path = [NSString stringWithUTF8String:filePath];

    // Build an absolute URL from the path
    NSURL *url = nil;
    if ([path hasPrefix:@"/"]) {
      url = [NSURL fileURLWithPath:path];
    } else {
      // Relative path – resolve from current working directory
      NSString *cwd = [[NSFileManager defaultManager] currentDirectoryPath];
      NSString *full = [cwd stringByAppendingPathComponent:path];
      url = [NSURL fileURLWithPath:full];
    }

    NSError *error = nil;
    bgPlayer = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:&error];
    if (error) {
      NSLog(@"[Audio] Failed to load %@: %@", path, error.localizedDescription);
      return;
    }

    bgPlayer.numberOfLoops = -1; // Loop forever
    bgPlayer.volume = 0.5f;      // 50 % volume – adjust to taste
    [bgPlayer prepareToPlay];
    [bgPlayer play];
  }
}

void stopBackgroundMusic() {
  if (bgPlayer) {
    [bgPlayer stop];
    bgPlayer = nil;
  }
}
