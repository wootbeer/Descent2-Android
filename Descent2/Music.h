//
//  Music.h
//  Descent
//
//

#ifndef ANDROID_NDK
#import <AudioToolbox/AudioToolbox.h>

void startMidiLoop(unsigned int interval, MusicPlayer *musicPlayer);
void stopMidiLoop();
#endif

long getRedbookTrackCount();
void setMusicVolume(float volume);
int playRedbookTrack(int tracknum, int loop);
void stopMusic();
