#include "raylib.h"
#include "reasings.h"
#include "raymath.h"
#include "presentation.h"

#include <stdio.h>
#include <stdbool.h>
#include <math.h>

const int INITIAL_SCREEN_WIDTH = 2880 / 3;
const int INITIAL_SCREEN_HEIGHT = 1920 / 3;

int screenWidth = INITIAL_SCREEN_WIDTH;
int screenHeight = INITIAL_SCREEN_HEIGHT;
int gameScreenWidth = INITIAL_SCREEN_WIDTH;
int gameScreenHeight = INITIAL_SCREEN_HEIGHT;


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



// ---------------------------
// Function Declaration
// ---------------------------

// Todo: add declarations
void reset(int *score);

// ---------------------------
// Function definition
// ---------------------------

Rectangle GetRandomSource() {
    return (Rectangle) { 32 * GetRandomValue(0, 1), 32 * GetRandomValue(0, 4), 32, 32 };
}
void SetRandomSourceRec(Rectangle *rect) {
    rect->x =  32 * GetRandomValue(0, 1);
    rect->y = 32 * GetRandomValue(0, 4);
}
void ApplyShake(Tray *tray, float *elementX, float *elementY) {
    if (tray->shakeDuration > 0.0f) {
        // Generate random offsets within the intensity range
        float offsetX = (GetRandomValue(0, 100) / 100.0f - 0.5f) * tray->shakeIntensity * 2.0f;
        float offsetY = (GetRandomValue(0, 100) / 100.0f - 0.5f) * tray->shakeIntensity * 2.0f;

        // Apply offsets to the element's position
        *elementX += offsetX;
        *elementY += offsetY;

        // Decrease shake duration over time
        tray->shakeDuration -= 0.016f; // Assuming ~16ms frame time
    } else {
        tray->isShaking = false;
        tray->dest.x = tray->originalPosition.x;
        tray->dest.y = tray->originalPosition.y;
    }
}


int CompareTrays(const void* a, const void* b) {
    // Attempt 1 - Deference input
    // Tray A = (* (Tray*) a);  // typecastint to Tray* and deference to get the value
    // Tray B = (* (Tray*) b);

    // Attempt 2 - Access pointer members
    const Tray *A = (const Tray*) a;  // typecastint to Tray*
    const Tray *B = (const Tray*) b;
    // printf("\nid: %d\n", A.id);
    // return A.id - B.id;
    // return B.id - A.id;
    // return B->id - A->id;
}
void initStars(Animation *stars, Texture2D *starsTexture, Spritesheet starsSheet) {
    for (int i = 0; i < NO_OF_STARS; ++i) {
        stars[i] = (Animation) {
            .texture = starsTexture,
            .position = (Vector2) { 0 },
            .sheet = starsSheet,
            .isAnimating = false
         };
    }

}
void initTrays(Game *game) {
    Tray *trays = game->trays;
    Color *colors = game->colors;
    Texture2D *texture = game->trayTexture;

    int trayStartX = -(TRAY_WIDTH * NO_OF_TRAYS) / 2;
    for (int i = 0; i < NO_OF_TRAYS; ++i) {
        Rectangle dest = {
            trayStartX + gameScreenWidth / 2 + (TRAY_WIDTH * i) + (i * GAP) - (GAP * (NO_OF_TRAYS - 1)) / 2,
            gameScreenHeight - TRAY_HEIGHT - PADDING,
            TRAY_WIDTH,
            TRAY_HEIGHT
        };

        trays[i] = (Tray) {
            .texture = texture,
            .color = colors[i],
            .dest = dest,
            .isShaking = false,
            .shakeDuration = 0.0f,
            .shakeIntensity = 0.0f,
            .originalPosition = { dest.x, dest.y }
        };
    }
}
void initCards(Game *game) {
    Card *cards = game->cards;
    Color *colors = game->colors;
    Texture2D *textures = game->cardTextures;

    int cardStartX = -(CARD_WIDTH * NO_OF_CARDS) / 2;
    for (int i = 0; i < NO_OF_CARDS; ++i) {
        Vector2 startPosition = {
            cardStartX + gameScreenWidth / 2 + (CARD_WIDTH * i) + (i * GAP) - (GAP * (NO_OF_CARDS - 1)) / 2,
            PADDING
        };
        int id = GetRandomValue(0, NO_OF_TRAYS - 1);

        if (isDrawCard && id >= 3) id = GetRandomValue(0, 2);

        cards[i].dest = (Rectangle) {
            startPosition.x,
            startPosition.y,
            CARD_WIDTH,
            CARD_HEIGHT
        };
        cards[i].color = colors[id];

        // flags
        cards[i].isDragging = false;
        cards[i].reachedTarget = false;
        cards[i].scoredPoints = false;

        // tween
        cards[i].currentPosition = startPosition;   // This is set to the mousePosition at runtime
        cards[i].targetPosition = startPosition;
        cards[i].state = IDLE;
        cards[i].frameCounter = 0;
        cards[i].duration = 30.0f;                  // Length in frame (30 frame = 500ms)

        // img
        cards[i].nPatchTexture = game->nPatchTexture;
        cards[i].nPatchSrc = game->nPatchSrc;
        cards[i].imgTexture = textures[id];
        cards[i].imgSrc = GetRandomSource();
    }
}

void handleInput(Game *game, float scale) {
    Tray *trays = game->trays;
    Card *cards = game->cards;
    Color *colors = game->colors;
    Animation *stars = game->stars;

    if (IsKeyPressed(KEY_F)) {
        if (!IsWindowFullscreen()) {
            int monitor = GetCurrentMonitor();
            screenWidth  = GetMonitorWidth(monitor);
            screenHeight = GetMonitorHeight(monitor);
            SetWindowSize(screenWidth, screenHeight);
            ToggleFullscreen();
        } else {
            ToggleFullscreen();
            screenWidth = INITIAL_SCREEN_WIDTH;
            screenHeight = INITIAL_SCREEN_HEIGHT;
            SetWindowSize(screenWidth, screenHeight);
        }
    }
    if (IsKeyPressed(KEY_R)) {
        reset(&game->score);
        initCards(game);
    }


    // Update virtual mouse (clamped mouse value behind game screen)
    // Vector2 mouse = GetMousePosition();
    // Vector2 touchPosition = GetTouchPosition(0);

    Vector2 mouse = GetTouchPosition(0);
    Vector2 virtualMouse = { 0 };
    virtualMouse.x = (mouse.x - (screenWidth - (gameScreenWidth*scale))*0.5f)/scale;
    virtualMouse.y = (mouse.y - (screenHeight - (gameScreenHeight*scale))*0.5f)/scale;
    virtualMouse = Vector2Clamp(virtualMouse, (Vector2){ 0, 0 }, (Vector2){ (float)gameScreenWidth, (float)gameScreenHeight });

    game->virtualMouse = virtualMouse;

    // Handle Tray
    for (int i = 0; i < NO_OF_TRAYS; ++i) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            // colors[i] = original[i];
        }
    }

    // Handle Cards
    for (int i = 0; i < NO_OF_CARDS; ++i) {
        // Pointer arithmetic to get the next card
        Card *card = (cards + i);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(virtualMouse, card->dest)) {
                card->isDragging = true;
            }
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            if (card->isDragging) {
                card->dest.x = virtualMouse.x - card->dest.width / 2;
                card->dest.y = virtualMouse.y - card->dest.height / 2;
            }
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {

            if (card->isDragging) {
                card->isDragging = false;

                bool hit = false;
                int sum = 0;

                Tray *tray;
                for (int j = 0; j < NO_OF_TRAYS; ++j) {
                    tray = trays + j;
                    if (CheckCollisionRecs(card->dest, tray->dest) && ColorIsEqual(card->color, colors[j])) {
                        hit = true;
                        ++(game->counter);
                        break;
                    }
                }

                // Did the card enter the correct tray?
                if (hit) {
                    // Well done, but has it already entered the zone?
                    if (card->reachedTarget) continue;

                    #if defined(DEBUG)
                        printf("HIT %d\n", game->counter);
                    #endif

                    if (!card->scoredPoints) {
                        ++(game->score);

                        // Find a slot thats not animating and start animating
                        for (int i = 0; i < NO_OF_STARS; ++i) {
                            Animation *star = stars + i;
                            if (!star->isAnimating) {
                                star->position = (Vector2) { virtualMouse.x - star->texture->width / NO_FRAMES_STARS / 2, virtualMouse.y - star->texture->height / 2 };
                                star->isAnimating = true;
                                break;
                            }
                        }

                        // Apply screen shake to the current Tray
                        tray->isShaking = true;   // Shake for 0.5 seconds
                        tray->shakeDuration = 0.10f;   // Shake for 0.5 seconds
                        tray->shakeIntensity = 1.0f;  // Shake ny up to 10 pixels

                        if (isAudio && !isOff) PlaySFX(sfx.click);
                    }
                    card->reachedTarget = true;
                    card->scoredPoints = true;
                } else {
                    // No, tween the card back to its original position

                    #if defined(DEBUG)
                        printf("Reset Card...\n");
                    #endif

                    if (isTweenCard && !isOff) {
                        card->state = TWEEN;
                        card->currentPosition = (Vector2) { virtualMouse.x - card->dest.width / 2, virtualMouse.y - card->dest.height / 2 };
                    } else {
                        card->dest.x = card->targetPosition.x;
                        card->dest.y = card->targetPosition.y;
                    }

                    if (isAudio && !isOff) PlaySFX(sfx.stop);
                }

                // Have all cards been moved to the correct zone?
                for (int j = 0; j < NO_OF_CARDS; ++j) {
                    sum += cards[j].reachedTarget;
                }

                // Yes? Reset cards
                if (sum >= NO_OF_CARDS) {
                    initCards(game);

                    if (isAudio && !isOff) PlaySFX(sfx.popup);
                }
            }

        }
    }

}

/**
 * Card cards[] is interpreted as Card *card
 */
void updateCards(Card cards[]) {
    if (isTweenCard && !isOff) {
        for (int i = 0; i < NO_OF_CARDS; ++i) {
            Card *card = (cards + i);
            // Card *card = &cards[i];
            if (card->state == TWEEN) {
                card->frameCounter++;

                float x = EaseBackOut(
                        (float) card->frameCounter,
                        card->currentPosition.x,
                        card->targetPosition.x - card->currentPosition.x,
                        card->duration
                    );
                float y = EaseBackOut(
                        (float) card->frameCounter,
                        card->currentPosition.y,
                        card->targetPosition.y - card->currentPosition.y,
                        card->duration
                    );

                card->dest.x = x;
                card->dest.y = y;

                if (card->frameCounter >= card->duration) {
                    card->frameCounter = 0;
                    card->state = IDLE;
                    card->dest.x = card->targetPosition.x;
                    card->dest.y = card->targetPosition.y;
                }

            }
        }
    }
}
void updateTrays(Tray *trays) {
    for (int i = 0; i < NO_OF_TRAYS; ++i) {
        // Tray *tray = (game->trays + i);
        Tray *tray = &trays[i];
        if (tray->isShaking) {
            ApplyShake(&trays[i], &tray->dest.x, &tray->dest.y);
        }
    }
}
void updateStars(Animation *stars) {
    // Loop through all stars, and animate otherwise do nothing to empty array
    for (int i = 0; i < NO_OF_STARS; ++i) {
        Animation *star = stars + i;

        if (star->isAnimating) {
            star->sheet.frameCounter++;
            // Slow down frame speed
            if (star->sheet.frameCounter >= (GetFPS() / star->sheet.frameSpeed)) {
                // Time to update current frame index and reset counter
                star->sheet.frameCounter = 0;
                star->sheet.currentFrame++;
                // Ensure frame index stays within bounds
                if (star->sheet.currentFrame > NO_FRAMES_STARS - 1) {
                    // Star animation is complete
                    // Reset it members
                    // Remove current reference from the list
                    star->sheet.currentFrame = 0;
                    star->isAnimating = false;
                }
                // Update source rect (index * width)
                star->sheet.srcRec.x = (float) star->sheet.currentFrame * (float) star->texture->width / NO_FRAMES_STARS;
            }
        }

    }
}

void drawBackground(Texture2D layers[], double *increment, int order[]) {
    if (isDrawBackground && !isOff) {
        (*increment) += (0.09) * GetFrameTime();

        int width = layers[0].width;
        int height = layers[0].height;
        int startX = 0;
        int startY = -height / 2 ;

        int row = 0;

        // For each layer on the y axis
        while (startY < gameScreenHeight + height * 2) {

            int index = order[row % 4];
            int clampedW = (width - gameScreenWidth) / 2;
            int speed = 0;

            if (isPrarallaxBackground && !isOff) {
                speed = (sin(*increment * index) * clampedW) + clampedW; // 0 < speed < 900
            }

            int xPos = startX - speed;

            DrawTexture(layers[index], xPos, startY, WHITE);

            startY += height / 1.6;
            row++;

        }
    }
}
void drawTrays(Tray trays[]) {
    for (int i = 0; i < NO_OF_TRAYS; ++i) {
        Tray tray = trays[i];
        if (isDrawTray && !isOff) {
            // DrawRectangleRounded(trays[i], 0.3f, 16, colors[i]);    // Show bounds
            DrawTextureV(*trays->texture, (Vector2){tray.dest.x - 7, tray.dest.y + 7}, BLACK);
            DrawTextureV(*trays->texture, (Vector2){tray.dest.x, tray.dest.y}, tray.color);
        } else {
            DrawRectangleRounded(trays[i].dest, 0.3f, 16, tray.color);
        }
    }
}
void drawCards(Card cards[], Texture2D check) {
    for (int i = 0; i < NO_OF_CARDS; ++i) {
        Card card = cards[i];
        if (isDrawCard && !isOff) {
            DrawTextureNPatch(card.nPatchTexture, card.nPatchSrc, card.dest, (Vector2) { 0 }, 0, WHITE);
            DrawTexturePro(card.imgTexture, card.imgSrc, card.dest, (Vector2) { 0 }, 0, WHITE);
            // DrawRectangleRoundedLinesEx(card.dest, 0.3f, 16, 6, ColorAlpha(PINK, 0.5f));
        } else {
            DrawRectangleRoundedLinesEx(card.dest, 0.3f, 16, 2, ColorAlpha(BLACK, 0.3f));
            DrawRectangleRounded(card.dest, 0.3f, 16, card.color);
        }

        // Draw empty square
        if (card.reachedTarget) {
            int x = (card.targetPosition.x + CARD_WIDTH / 2) - check.width / 2;
            int y = (card.targetPosition.y + CARD_HEIGHT / 2) - check.height / 2;
            DrawRectangleLines(card.targetPosition.x, card.targetPosition.y, CARD_WIDTH, CARD_HEIGHT, ColorAlpha(GRAY, 0.4f));
            DrawTexture(check, x, y, WHITE);
        }
    }
}
void drawCursor(Vector2 virtualMouse, Texture2D cursor, Texture2D cursorPressed) {
    if (isShowCursor && IsCursorOnScreen() && !isOff) {
        // Subtract the offset of cursor tip
        virtualMouse = Vector2SubtractValue(virtualMouse, 17);
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            DrawTexture(cursorPressed, virtualMouse.x, virtualMouse.y, WHITE);
        } else {
            DrawTexture(cursor, virtualMouse.x, virtualMouse.y, WHITE);
        }
    }
}
void drawScore(int score) {
    DrawText((TextFormat("Score: %d", score)), 20, 20, 30, GRAY);
}
void drawStars(Animation *stars) {
    // Loop through all stars, and draw otherwise do nothing to empty array
    for (int i = 0; i < NO_OF_STARS; ++i) {

        Animation *star = stars + i;
        if (star->isAnimating && isAnimateStars && !isOff) {
            DrawTextureRec(*(star->texture), star->sheet.srcRec, star->position , WHITE);
        }
    }
}

void reset(int *score) {
    *score = 0;
}
