#include "audio.h"
#include "address_map.h"
#include <stdlib.h>

SoundInstance active_sounds[MAX_ACTIVE_SOUNDS];
const short *bgm_data = NULL;
int bgm_pos = 0;
int bgm_len = 0;

void audio_update(){
    struct audio_t* audio_ptr = ((struct audio_t*) AUDIO_BASE);

    while(audio_ptr->wsrc > 0 && audio_ptr->wslc > 0){
        // Dummy read to keep the codec clock flowing
        volatile int dummy_l = audio_ptr->ldata;
        volatile int dummy_r = audio_ptr->rdata;
        (void)dummy_l; (void)dummy_r;

        int mixed_sample = 0;

        if(bgm_data != NULL){
            mixed_sample += bgm_data[bgm_pos];
            if(++bgm_pos >= bgm_len) bgm_pos = 0;
        }

        for(int i = 0; i < MAX_ACTIVE_SOUNDS; i++){
            if(active_sounds[i].active){
                mixed_sample += (int)(active_sounds[i].samples[active_sounds[i].position] * active_sounds[i].volume);
                if(++active_sounds[i].position >= active_sounds[i].length){
                    if(active_sounds[i].loop) active_sounds[i].position = 0;
                    else active_sounds[i].active = 0;
                }
            }
        }

        if(mixed_sample >  32767) mixed_sample =  32767;
        if(mixed_sample < -32768) mixed_sample = -32768;

        audio_ptr->ldata = mixed_sample;
        audio_ptr->rdata = mixed_sample;
    }
}

void play_bgm(const short *data, int length){
    bgm_data = data;
    bgm_len = length; 
    bgm_pos = 0; //start from beginning
}

void play_sfx(const short *data, int length, float volume, int loop){
    for(int i = 0; i < MAX_ACTIVE_SOUNDS; i++){
        if (!active_sounds[i].active) {
            active_sounds[i].samples = data;
            active_sounds[i].length = length;
            active_sounds[i].position = 0;
            active_sounds[i].volume = volume;
            active_sounds[i].active = 1;
            active_sounds[i].loop = loop;
            return; // Found a slot, exit
        }
    }
}
