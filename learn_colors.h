#ifndef LEARN_COLORS_H
#define LEARN_COLORS_H

#include "raylib.h"
#include "reasings.h"
#include "raymath.h"
#include "presentation.h"

#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

#define NO_OF_TRAYS 3
#define NO_OF_CARDS 4
#define GAP 70              // Space between cards & trays
#define PADDING 70          // Space above & below

#define NO_OF_COLORS 8
#define NO_OF_CLOUDS 4      // 4 cloud sprites
#define NO_OF_STARS 4
#define NO_FRAMES_STARS 8

// https://gcc.gnu.org/onlinedocs/gcc-13.3.0/cpp/Defined.html - simplify
// #if (defined(isDrawTray) && isDrawTray < 1)
#if (isDrawTray < 1)
    // Size of tray without texture
    #define TRAY_WIDTH 150
    #define TRAY_HEIGHT 100
#else
    // Size of trey with texture
    #define TRAY_WIDTH 250
    #define TRAY_HEIGHT 183
#endif

#if (isDrawCard < 1)
    // Size of card without texture
    #define CARD_WIDTH 100
    #define CARD_HEIGHT 100
#else
    // Size of card with texture, should be multiple of 32 to avoid scaling issues
    #define CARD_WIDTH 128
    #define CARD_HEIGHT 128
#endif

// Comment out the line below to remove all debug related stuff
#define DEBUG 1

#ifdef DEBUG
    bool HasValueChanged(float value, float previous) {
        return value != previous;
    }
    float previousScale = 0;
#endif

#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))

typedef enum {
    IDLE = 0,
    TWEEN,
} TweenState;

typedef struct Spritesheet {
    Rectangle srcRec;         // Draw a part of a texture defined by a rectangle
    int currentFrame;           // The current frame, x-axis. ( frameRect.x * currentFrame )
    int currentLine;            // The current frame, y-axis. ( frameRect.y * currentLine )
    int frameCounter;
    int frameSpeed;
} Spritesheet;

typedef struct Animation {
    Texture2D *texture;
    Vector2 position;
    Spritesheet sheet;
    bool isAnimating;
} Animation;

typedef struct Card {
    Rectangle dest;             // Actual position
    Color color;
    bool isDragging;
    bool reachedTarget;
    bool scoredPoints;

    // tween
    Vector2 currentPosition;    // Tween start position
    Vector2 targetPosition;     // Could be consts
    int state;                  // IDLE | TWEEN
    int frameCounter;           // Current time in tween
    float duration;             // How long to tween in frames e.g 30 frames = 500ms, 60 = 1sec

    // img
    Texture2D nPatchTexture;
    NPatchInfo nPatchSrc;
    Texture2D imgTexture;
    Rectangle imgSrc;
} Card;

typedef struct Tray {
    Texture2D *texture;
    Color color;
    Rectangle dest;
    bool isShaking;
    float shakeDuration;     // Duration of the shake effect
    float shakeIntensity;    // Intensity of the shake (pixels)
    Vector2 originalPosition;
} Tray;

typedef struct Game {
    Card cards[NO_OF_CARDS];
    Tray trays[NO_OF_TRAYS];
    Color *colors;
    Texture2D *cardTextures;
    Texture2D *trayTexture;
    Animation *stars;
    int frameCounter;
    int score;
    int counter;
    Texture2D nPatchTexture;
    NPatchInfo nPatchSrc;
    Vector2 virtualMouse;
} Game;

typedef struct Context {
    Game *game;
    RenderTexture2D target;
    Texture2D *cloudsTexture;   // 4
    double increment;
    int *order; // 20
    Texture2D checkTexture;
    Texture2D cursorTexture;
    Texture2D cursorPressedTexture;
} Context;

// Function declarations
Rectangle getRandomSource(void);
void setRandomSourceRec(Rectangle *rect);
void applyShake(Tray *tray, float *elementX, float *elementY);
int compareTrays(const void* a, const void* b);
void initStars(Animation *stars, Texture2D *starsTexture, Spritesheet starsSheet);
void initTrays(Game *game);
void initCards(Game *game);
void handleInput(Game *game, float scale);
void updateCards(Card cards[]);
void updateTrays(Tray *trays);
void updateStars(Animation *stars);
void drawBackground(Texture2D layers[], double *increment, int order[]);
void drawTrays(Tray trays[]);
void drawCards(Card cards[], Texture2D check);
void drawCursor(Vector2 virtualMouse, Texture2D cursor, Texture2D cursorPressed);
void drawScore(int score);
void drawStars(Animation *stars);
void reset(int *score);
void GameLoop();

#endif // LEARN_COLORS_H
