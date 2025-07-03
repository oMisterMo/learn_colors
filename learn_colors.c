#include "learn_colors_audio.h"
#include "learn_colors.h"


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

    SetRandomSourceRec(&greenSrc);
    SetRandomSourceRec(&redSrc);
    SetRandomSourceRec(&blueSrc);

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

    SetTargetFPS(60);

    printf("-------------------\n");
    printf("GAME\n");
    printf("-------------------\n");
    while(!WindowShouldClose()) {

        // Compute required framebuffer scaling
        float scale = MIN((float) screenWidth / gameScreenWidth, (float )screenHeight / gameScreenHeight);

        #if defined(DEBUG)
            if (HasValueChanged(scale, previousScale)) {
                Vector2 dpi = GetWindowScaleDPI();
                int monitor = GetCurrentMonitor();
                printf("monitor: %s %d\n", GetMonitorName(monitor), monitor);
                printf("scale: %.2f\n", scale);
                printf("resolution: %d x %d\n", screenWidth, screenHeight);
                printf("render: %d x %d\n", GetRenderWidth(), GetRenderHeight());
                // printf("dpi: %.2f, %.2f\n", dpi.x, dpi.y);
                printf("----------------\n");
                previousScale = scale;
            }
        #endif

        // Input
        handleInput(&game, scale);

        // Update
        updateCards(game.cards);
        updateTrays(game.trays);
        updateStars(game.stars);

        BeginTextureMode(target);
            ClearBackground(WHITE);
            drawBackground(cloudsTexture, &increment, order);
            drawTrays(game.trays);
            drawCards(game.cards, checkTexture);
            drawCursor(game.virtualMouse, cursorTexture, cursorPressedTexture);
            drawScore(game.score);
            drawStars(stars);
            DrawRectangleLinesEx((Rectangle){0,0,screenWidth,screenHeight}, 1, Fade(BLACK, 0.2));
            DrawFPS(gameScreenWidth - MeasureText("60 FPS", 20) - 20, 20);
        EndTextureMode();


        // Draw
        float x = (screenWidth - ((float) gameScreenWidth * scale)) * 0.5f;
        float y = (screenHeight - ((float) gameScreenHeight * scale)) * 0.5f;
        Rectangle renderSource = { 0.0f, 0.0f, (float) target.texture.width, (float) -target.texture.height };
        Rectangle renderDest = { x, y, (float) gameScreenWidth * scale, (float) gameScreenHeight * scale };
        Vector2 origin = { 0, 0 };
        BeginDrawing();
        // BeginScissorMode(0, 0, GetScreenWidth(), 200);
            ClearBackground(BLACK);
            // Draw render texture to screen, properly scaled
            DrawTexturePro(target.texture, renderSource, renderDest, origin, 0.0f, WHITE);

        // EndScissorMode();
        EndDrawing();

    }

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
