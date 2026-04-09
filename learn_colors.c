#include "learn_colors_audio.h"
#include "learn_colors.h"

const int INITIAL_SCREEN_WIDTH = 2880 / 3;
const int INITIAL_SCREEN_HEIGHT = 1920 / 3;

int screenWidth = INITIAL_SCREEN_WIDTH;
int screenHeight = INITIAL_SCREEN_HEIGHT;
int gameScreenWidth = INITIAL_SCREEN_WIDTH;
int gameScreenHeight = INITIAL_SCREEN_HEIGHT;

static Context ctx = { 0 };


// Function definition

// Utils
Rectangle getRandomSource() {
    return (Rectangle) { 32 * GetRandomValue(0, 1), 32 * GetRandomValue(0, 4), 32, 32 };
}
void setRandomSourceRec(Rectangle *rect) {
    rect->x =  32 * GetRandomValue(0, 1);
    rect->y = 32 * GetRandomValue(0, 4);
}
void applyShake(Tray *tray, float *elementX, float *elementY) {
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
int compareTrays(const void* a, const void* b) {
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
    (void) A;
    (void) B;
    return 0;
}

// Input
void handleInput(Game *game, float scale) {
    Tray *trays = game->trays;
    Card *cards = game->cards;
    Color *colors = game->colors;
    Animation *stars = game->stars;
#ifdef PLATFORM_WEB
#else
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
#endif
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

// Draw
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

// Score
void reset(int *score) {
    *score = 0;
}
void drawScore(int score) {
    DrawText((TextFormat("Score: %d", score)), 20, 20, 30, GRAY);
}

// Stars
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
void drawStars(Animation *stars) {
    // Loop through all stars, and draw otherwise do nothing to empty array
    for (int i = 0; i < NO_OF_STARS; ++i) {

        Animation *star = stars + i;
        if (star->isAnimating && isAnimateStars && !isOff) {
            DrawTextureRec(*(star->texture), star->sheet.srcRec, star->position , WHITE);
        }
    }
}

// Trays
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
void updateTrays(Tray *trays) {
    for (int i = 0; i < NO_OF_TRAYS; ++i) {
        // Tray *tray = (game->trays + i);
        Tray *tray = &trays[i];
        if (tray->isShaking) {
            applyShake(&trays[i], &tray->dest.x, &tray->dest.y);
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

// Cards
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
        cards[i].imgSrc = getRandomSource();
    }
}
void updateCards(Card cards[]) {
    /**
     * Card cards[] is interpreted as Card *card
     */
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


void GameLoop() {
        // Compute required framebuffer scaling
        float scale = MIN((float) screenWidth / gameScreenWidth, (float )screenHeight / gameScreenHeight);

        #if defined(DEBUG)
            if (HasValueChanged(scale, previousScale)) {
                Vector2 dpi = GetWindowScaleDPI();
                int monitor = GetCurrentMonitor();
                printf("%-14s: %s %d\n", "monitor", GetMonitorName(monitor), monitor);
                printf("%-14s: %.2f\n", "scale", scale);
                printf("%-14s: %d x %d\n", "resolution", screenWidth, screenHeight);
                printf("%-14s: %d x %d\n", "render", GetRenderWidth(), GetRenderHeight());
                printf("%-14s: %.2f, %.2f\n", "dpi", dpi.x, dpi.y);
                printf("-------------------\n");
                previousScale = scale;
            }
        #endif

        // Input
        handleInput(ctx.game, scale);

        // Update
        updateCards(ctx.game->cards);
        updateTrays(ctx.game->trays);
        updateStars(ctx.game->stars);

        // Draw to texture
        BeginTextureMode(ctx.target);
            ClearBackground(WHITE);
            drawBackground(ctx.cloudsTexture, &ctx.increment, ctx.order);
            drawTrays(ctx.game->trays);
            drawCards(ctx.game->cards, ctx.checkTexture);
            drawCursor(ctx.game->virtualMouse, ctx.cursorTexture, ctx.cursorPressedTexture);
            drawScore(ctx.game->score);
            drawStars(ctx.game->stars);
            DrawRectangleLinesEx((Rectangle){0,0,screenWidth,screenHeight}, 1, Fade(BLACK, 0.2));
            DrawFPS(gameScreenWidth - MeasureText("60 FPS", 20) - 20, 20);
        EndTextureMode();


        // Draw to screen
        float x = (screenWidth - ((float) gameScreenWidth * scale)) * 0.5f;
        float y = (screenHeight - ((float) gameScreenHeight * scale)) * 0.5f;
        Rectangle renderSource = { 0.0f, 0.0f, (float) ctx.target.texture.width, (float) -ctx.target.texture.height };
        Rectangle renderDest = { x, y, (float) gameScreenWidth * scale, (float) gameScreenHeight * scale };
        Vector2 origin = { 0, 0 };
        float rotation = 0.0f;

        BeginDrawing();
        // BeginScissorMode(0, 0, GetScreenWidth(), 200);
            ClearBackground(BLACK);
            // Draw render texture to screen, properly scaled
            DrawTexturePro(ctx.target.texture, renderSource, renderDest, origin, rotation, WHITE);

        // EndScissorMode();
        EndDrawing();
}

int main() {

    // Setup config
    printf("-------------------\n");
    printf("INIT WINDOW\n");
    printf("-------------------\n");

    // SetConfigFlags( FLAG_WINDOW_UNDECORATED );
    InitWindow(screenWidth, screenHeight, "Learn Colors");
    SetMousePosition(-10, -10);
    if (isShowCursor) {
        // Cursor stuff not working :(
        // HideCursor();
        // DisableCursor();
        // SetMouseCursor(MOUSE_CURSOR_CROSSHAIR);
        SetMouseOffset(-17, -17);
    }

    // Initialization
    printf("-------------------\n");
    printf("LOAD TEXTURES\n");
    printf("-------------------\n");
    // Render texture initialization, used to hold the rendering result so we can easily resize it
    RenderTexture2D target = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
    // SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);  // Texture scale filter to use
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);  // Texture scale filter to use

    Texture2D checkTexture = LoadTexture("resources/sprites/check.png");
    Texture2D cloudsTexture[NO_OF_CLOUDS];
    for (int i = 0; i < NO_OF_CLOUDS; ++i) {
        cloudsTexture[i] = LoadTexture(TextFormat("resources/backgrounds/clouds_%d.png", i + 1));
    }
    Texture2D cursorTexture = LoadTexture("resources/ui/icon_hand_1.png");
    Texture2D cursorPressedTexture = LoadTexture("resources/ui/icon_hand_2.png");
    Texture2D trayTexture = LoadTexture("resources/sprites/tray.png");
    Texture2D starsTexture = LoadTexture("resources/ui/medal_stars.png");
    Texture2D redTexture = LoadTexture("resources/sprites/red.png");
    Texture2D greenTexture = LoadTexture("resources/sprites/green.png");
    Texture2D blueTexture = LoadTexture("resources/sprites/blue.png");
    Texture2D nPatchTexture = LoadTexture("resources/ui/ninepatch_button.png");

    trayTexture.width = TRAY_WIDTH;
    trayTexture.height = TRAY_HEIGHT;

    // Game vars

    // Compound Literals (C99)
    Rectangle greenSrc = (Rectangle) { 0, 0, 32, 32 };
    Rectangle redSrc = (Rectangle) { 0, 0, 32, 32 };
    Rectangle blueSrc = (Rectangle) { 0, 0, 32, 32 };
    // Designated Initializers (C99)
    NPatchInfo srcInfo = {
        .source = { 0.0f, 64.0f, 64.0f, 64.0f },
        .left = 16,
        .top = 16,
        .right = 16,
        .bottom = 16,
        .layout = NPATCH_NINE_PATCH
    };

    setRandomSourceRec(&greenSrc);
    setRandomSourceRec(&redSrc);
    setRandomSourceRec(&blueSrc);

    Spritesheet starsSheet = {
        .srcRec = (Rectangle) { 0, 0, starsTexture.width / NO_FRAMES_STARS, starsTexture.height },
        .currentFrame = 0,
        .currentLine = 0,
        .frameCounter = 0,
        .frameSpeed = 10
     };

    Animation stars[NO_OF_STARS];

    Game game = {
        .colors = (Color[]) { RED, GREEN, BLUE, ORANGE, PINK, PURPLE, SKYBLUE, GRAY },
        .cardTextures = (Texture2D[]) { redTexture, greenTexture, blueTexture },
        .trayTexture = &trayTexture,
        .frameCounter = 0,
        .score = 0,
        .counter = 0,
        .stars = stars,
        .nPatchTexture = nPatchTexture,
        .nPatchSrc = srcInfo,
        .virtualMouse = { 0 }
     };

    // Rectangle trays[NO_OF_TRAYS];
    initStars(stars, &starsTexture, starsSheet);
    initTrays(&game);
    initCards(&game);

    double increment = 0.0;

    int order[20];      // Random value used to displace the backgrounds velocity
    for (int i = 0; i < 20; i++) {
        order[i] = GetRandomValue(0, 3);
    }

    // Audio
    if (isAudio && !isOff) {
        printf("-------------------\n");
        printf("LOAD AUDIO\n");
        printf("-------------------\n");
        LoadSFX();
    }


    ctx.game = &game;
    ctx.target = target;
    ctx.cloudsTexture = cloudsTexture;
    ctx.increment = increment;
    ctx.order = order;
    ctx.checkTexture = checkTexture;
    ctx.cursorTexture = cursorTexture;
    ctx.cursorPressedTexture = cursorPressedTexture;


    printf("-------------------\n");
    printf("GAME\n");
    printf("-------------------\n");
    // Main game loop
    #if defined(PLATFORM_WEB)
        emscripten_set_main_loop(GameLoop, 0, 1);
    #else
        SetTargetFPS(60);
        while (!WindowShouldClose()) {
            GameLoop();
        }
    #endif

    printf("-------------------\n");
    printf("DESTROY\n");
    printf("-------------------\n");

    // Textures
    UnloadRenderTexture(target);
    UnloadTexture(checkTexture);
    for (int i = 0; i < NO_OF_CLOUDS; ++i) {
        UnloadTexture(cloudsTexture[i]);
    }
    UnloadTexture(cursorTexture);
    UnloadTexture(cursorPressedTexture);
    UnloadTexture(trayTexture);
    UnloadTexture(starsTexture);
    UnloadTexture(redTexture);
    UnloadTexture(greenTexture);
    UnloadTexture(blueTexture);

    // Audio
    if (isAudio && !isOff) {
        UnloadSFX();
    }

    CloseWindow();


    return 0;
}
