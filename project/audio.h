#ifndef AUDIO_H
#define AUDIO_H

// Start playing background music (loops forever).
// Call once at startup. The audio file path should be relative to the
// executable.
void startBackgroundMusic(const char *filePath);

// Stop the background music.
void stopBackgroundMusic();

#endif // AUDIO_H
