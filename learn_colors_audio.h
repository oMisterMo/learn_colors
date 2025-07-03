#ifndef LEARN_COLORS_
#define LEARN_COLORS_

#include "raylib.h"

// --------------------
#define MAX_SOUNDS 10
Sound soundArray[MAX_SOUNDS] = { 0 };
int currentSound;
// --------------------

typedef struct SFX {
    Sound click;
    Sound select;
    Sound stop;
    Sound popup;
} SFX;

SFX sfx = { 0 };

void LoadSFX() {
    InitAudioDevice();
    
    sfx.click = LoadSound("resources/sfx/button_click.wav");
    sfx.select = LoadSound("resources/sfx/piece_select.wav");
    sfx.stop = LoadSound("resources/sfx/piece_stop.wav");
    sfx.popup = LoadSound("resources/sfx/popup.wav");

    currentSound = 0;
}

void PlaySFX(Sound sound) {
    PlaySound(sound);
}

void UnloadSFX() {
    UnloadSound(sfx.click);
    UnloadSound(sfx.select);
    UnloadSound(sfx.stop);
    UnloadSound(sfx.popup);
    CloseAudioDevice();
}

#endif // LEARN_COLORS_
