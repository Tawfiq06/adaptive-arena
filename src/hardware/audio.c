#include "audio.h"
#include "address_map.h"
#include <stdlib.h>


void audio_update(){
    struct audio_t* audio_ptr = ((struct audio_t*) AUDIO_BASE);

    //only if there is space in output FIFO
    while(audio_ptr->wsrc > 0 && audio_ptr->wslc > 0){
        int mixed_sample = 0; 

        /* 1. Add background music */
        if(bgm_data != NULL){
            mixed_sample += bgm_data[bgm_pos];
            bgm_pos++;
            //now loop bg music when it reaches the end
            if(bgm_pos >= bgm_len) bgm_pos = 0; 
        }

        /* 2. add active SFX */
        for(int i = 0; i < MAX_ACTIVE_SOUNDS; i++){
            if(active_sounds[i].active){
                short sample = active_sounds[i].samples[active_sounds[i].position];
                mixed_sample += (int) (sample * active_sounds[i].volume);

                active_sounds[i].position++;

                if(active_sounds[i].position >= active_sounds[i].length){
                    if(active_sounds[i].loop){
                        active_sounds[i].position = 0;
                    }
                    else{
                        active_sounds[i].active = 0;
                    }
                }
            }   
        }

        // 3. Clipping
        if (mixed_sample > 32767) mixed_sample = 32767;
        if (mixed_sample < -32768) mixed_sample = -32768;

        audio_ptr->ldata = mixed_sample;
        audio_ptr->rdata = mixed_sample;
    }
}

void play_bgm(const short *data, int length){
    current_bgm = data;
    bgm_length = length; 
    bgm_position = 0; //start from beginning
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
