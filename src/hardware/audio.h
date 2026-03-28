#ifndef AUDIO_H
#define AUDIO_H

#define MAX_ACTIVE_SOUNDS 8 //how many sounds to play at once
#define AUDIO_SAMPLE_RATE 8000

struct audio_t {
    volatile unsigned int control;
	volatile unsigned char rarc;
	volatile unsigned char ralc;
	volatile unsigned char wsrc;
	volatile unsigned char wslc;
	volatile unsigned int ldata;
	volatile unsigned int rdata;
};

typedef struct {
    const int *samples;
    int length;
    int position;
    int loop;
    int active; 
    float volume; //0.0 to 1.0
} SoundInstance;

extern SoundInstance active_sounds[MAX_ACTIVE_SOUNDS];
extern const int *bgm_data; //pointer to current background track
extern int bgm_pos;
extern int bgm_len;

void play_sfx(const int *data, int len, float volume, int loop);
void play_bgm(const int *data, int len);
void audio_update();


#endif