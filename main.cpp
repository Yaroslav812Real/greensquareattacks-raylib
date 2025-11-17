#include "include/raylib.h"
#include "stdlib.h"
#include "button.hpp"

#define STORAGE_DATA_FILE "highScore.data"

typedef enum GameScreen {MENU, OPTIONS, INGAME, GAMEOVER} GameScreen;
typedef enum {STORAGE_POSITION_HIGHSCORE = 0} StorageData;

static bool SaveStorageValue(unsigned int position, int value);
static int LoadStorageValue(unsigned int position);
static void ResetGame(void);
static void ResetRedSquare(void);

const int screenWidth = 1280; const int screenHeight = 720;

GameScreen currentScreen = MENU;

float buttonWidth = screenWidth / 1.5;
float buttonHeight = screenHeight / 4;
float buttonMiniWidth = screenWidth / 3;
float buttonMiniHeight = screenHeight / 8;

bool paused = false;

bool audio = true;
bool fullscreen = false;
bool vsync = true;
bool gradient = true;
bool squares = false;

const int redSquareOriginalHealth = 20;
const int redSquareMinSize = screenHeight / 14.4;
const int redSquareMaxSize = screenHeight / 2.4;
const float redSquareMinSpeed = screenWidth / 64;
const float redSquareMaxSpeed = screenWidth / 48;
const Color redSquareOriginalColor = RED;
int redSquareXSize;
int redSquareYSize;
float redSquareX;
float redSquareY;
int redSquareHealth = redSquareOriginalHealth;
float redSquareXSpeed;
float redSquareYSpeed;
Color redSquareColor = redSquareOriginalColor;
bool redSquareDestroyed = false;
bool redSquareXSquish = false;
bool redSquareYSquish = false;

const int greenSquareOriginalSize = screenHeight / 7.2;
const int greenSquareOriginalX = screenWidth - greenSquareOriginalSize;
const int greenSquareOriginalY = screenHeight - greenSquareOriginalSize;
const float greenSquareOriginalSpeed = screenWidth / 64;
const Color greenSquareOriginalColor = GREEN;
int greenSquareSize = screenHeight / 7.2;
float greenSquareX = greenSquareOriginalX;
float greenSquareY = greenSquareOriginalY;
float greenSquareSpeed = greenSquareOriginalSpeed;
Color greenSquareColor = greenSquareOriginalColor;
bool enemyCollision = false;

int rayEnergy = 100;
Color rayColor = {255, 255, 0, 255};
bool rayColorReverse = false;
bool rayActivated = false;
bool rayCollision = false;

bool giveScore = true;
int score = 0;
int highScore = 0;

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "Yarix game");
    InitAudioDevice();
    SetTargetFPS(60);

    Shader grayscale = LoadShader(0, "grayscale.fs");

    Music menu = LoadMusicStream("mus_menu.ogg");
    Music settings = LoadMusicStream("mus_settings.ogg");
    Music mus = LoadMusicStream("mus_sampletext.ogg");
    Music musPaused = LoadMusicStream("mus_sampletextpaused.ogg");
    Music gameOver = LoadMusicStream("mus_gameover.ogg");
    PlayMusicStream(menu);

    Sound raySound = LoadSound("sfx_ray.ogg");

    Rectangle redSquare;
    Rectangle greenSquareBox;
    Rectangle ray;
    Rectangle greenSquareLight;
    Rectangle rayLight;

    Button startButton;
    startButton.position = {(screenWidth / 2) - (buttonWidth / 2), (screenHeight / 5)};
    Button optionsButton;
    optionsButton.position = {(screenWidth / 2) - (buttonWidth / 2), (screenHeight / 5) + buttonHeight};
    Button exitButton;
    exitButton.position = {(screenWidth / 2) - (buttonWidth / 2), (screenHeight / 5) + (buttonHeight * 2)};
    Button audioMiniButton;
    audioMiniButton.position = {(screenWidth / 2) - (buttonMiniWidth / 2), (screenHeight / 10)};
    audioMiniButton.size = {buttonMiniWidth, buttonMiniHeight};
    audioMiniButton.toggleButton = true;
    audioMiniButton.toggle = true;
    Button fullscreenMiniButton;
    fullscreenMiniButton.position = {(screenWidth / 2) - (buttonMiniWidth / 2), (screenHeight / 10) + buttonMiniHeight};
    fullscreenMiniButton.size = {buttonMiniWidth, buttonMiniHeight};
    fullscreenMiniButton.toggleButton = true;
    Button vsyncMiniButton;
    vsyncMiniButton.position = {(screenWidth / 2) - (buttonMiniWidth / 2), (screenHeight / 10) + (buttonMiniHeight * 2)};
    vsyncMiniButton.size = {buttonMiniWidth, buttonMiniHeight};
    vsyncMiniButton.toggleButton = true;
    vsyncMiniButton.toggle = true;
    Button gradientMiniButton;
    gradientMiniButton.position = {(screenWidth / 2) - (buttonMiniWidth / 2), (screenHeight / 10) + (buttonMiniHeight * 3)};
    gradientMiniButton.size = {buttonMiniWidth, buttonMiniHeight};
    gradientMiniButton.toggleButton = true;
    gradientMiniButton.toggle = true;
    Button squaresMiniButton;
    squaresMiniButton.position = {(screenWidth / 2) - (buttonMiniWidth / 2), (screenHeight / 10) + (buttonMiniHeight * 4)};
    squaresMiniButton.size = {buttonMiniWidth, buttonMiniHeight};
    squaresMiniButton.toggleButton = true;
    Button backMiniButton;
    backMiniButton.position = {(screenWidth / 2) - (buttonMiniWidth / 2), (screenHeight / 10) + (buttonMiniHeight * 6)};
    backMiniButton.size = {buttonMiniWidth, buttonMiniHeight};
    Button retryButton;
    retryButton.position = {(screenWidth / 2) - (buttonWidth / 2), (screenHeight / 5) + buttonHeight};
    retryButton.size = {buttonWidth, buttonHeight};
    Button backButton;
    backButton.position = {(screenWidth / 2) - (buttonWidth / 2), (screenHeight / 5) + (buttonHeight * 2)};
    backButton.size = {buttonWidth, buttonHeight};

    Image checkerboardMenuImage = GenImageChecked(screenWidth, screenHeight, screenHeight / 5, screenHeight / 5, DARKGRAY, BLACK);
    Texture2D checkerboardMenuTexture = LoadTextureFromImage(checkerboardMenuImage);

    Image checkerboardIngameImage = GenImageChecked(screenWidth, screenHeight, screenHeight / 5, screenHeight / 5, BLUE, DARKBLUE);
    Texture2D checkerboardIngameTexture = LoadTextureFromImage(checkerboardIngameImage);

    while (!WindowShouldClose())
    {
        Vector2 mousePoint = GetMousePosition();
        const bool keyLeft = (IsKeyDown(KEY_A) or IsKeyDown(KEY_LEFT));
        const bool keyRight = (IsKeyDown(KEY_D) or IsKeyDown(KEY_RIGHT));
        const bool keyShoot = IsKeyDown(KEY_SPACE);
        const bool keyPause = IsKeyPressed(KEY_ENTER);
        switch(currentScreen)
        {
            case MENU:
            {
                if (audio) UpdateMusicStream(menu);

                highScore = LoadStorageValue(STORAGE_POSITION_HIGHSCORE);

                if (startButton.isReleased(mousePoint)) StopMusicStream(menu), ResetGame(), PlayMusicStream(mus), currentScreen = INGAME;
                if (optionsButton.isReleased(mousePoint)) StopMusicStream(menu), PlayMusicStream(settings), currentScreen = OPTIONS;
                if (exitButton.isReleased(mousePoint)) {CloseWindow(); return 0;}
            } break;

            case OPTIONS:
            {
                if (audio) UpdateMusicStream(settings);

                if (audioMiniButton.isReleased(mousePoint)) audio = !audio;
                if ((audio and !audioMiniButton.toggle) or (!audio and audioMiniButton.toggle)) audioMiniButton.toggle = !audioMiniButton.toggle;
                if (fullscreenMiniButton.isReleased(mousePoint)) fullscreen = !fullscreen, ToggleFullscreen();
                if ((fullscreen and !fullscreenMiniButton.toggle) or (!fullscreen and fullscreenMiniButton.toggle)) fullscreenMiniButton.toggle = !fullscreenMiniButton.toggle;
                if (vsyncMiniButton.isReleased(mousePoint)) vsync = !vsync, SetConfigFlags(FLAG_VSYNC_HINT);
                if ((vsync and !vsyncMiniButton.toggle) or (!vsync and vsyncMiniButton.toggle)) vsyncMiniButton.toggle = !vsyncMiniButton.toggle;
                if (gradientMiniButton.isReleased(mousePoint)) gradient = !gradient;
                if ((gradient and !gradientMiniButton.toggle) or (!gradient and gradientMiniButton.toggle)) gradientMiniButton.toggle = !gradientMiniButton.toggle;
                if (squaresMiniButton.isReleased(mousePoint)) squares = !squares;
                if ((squares and !squaresMiniButton.toggle) or (!squares and squaresMiniButton.toggle)) squaresMiniButton.toggle = !squaresMiniButton.toggle;
                if (backMiniButton.isReleased(mousePoint)) StopMusicStream(settings), PlayMusicStream(menu), currentScreen = MENU;
            } break;

            case INGAME:
            {

                if (audio) UpdateMusicStream(mus);
                if (audio) UpdateMusicStream(musPaused);

                if (rayActivated and !IsSoundPlaying(raySound) and audio) PlaySound(raySound);
                if (!rayActivated and IsSoundPlaying(raySound)) StopSound(raySound);

                if (keyPause) paused = !paused;
                if (paused and IsMusicStreamPlaying(mus)) StopMusicStream(mus), PlayMusicStream(musPaused);
                if (!paused and !IsMusicStreamPlaying(mus)) StopMusicStream(musPaused), PlayMusicStream(mus);

                redSquare = {(float)redSquareX, (float)redSquareY, (float)redSquareXSize, (float)redSquareYSize};
                if (redSquareY < 0) redSquareY = 0;
                if (redSquareX < 0) redSquareX = 0;
                if (redSquareY > (screenHeight - redSquareYSize)) redSquareY = screenHeight - redSquareYSize;
                if (redSquareX > (screenWidth - redSquareXSize)) redSquareX = screenWidth - redSquareXSize;

                greenSquareBox = {(float)greenSquareX, (float)greenSquareY, (float)greenSquareSize, (float)greenSquareSize};
                ray = {(float)greenSquareX + (greenSquareSize / 4), 0, (float)greenSquareSize / 2, (float)screenHeight - (greenSquareSize / 2)};
                greenSquareLight = {(float)greenSquareX - (greenSquareSize / 2), (float)greenSquareY - (greenSquareSize / 2), (float)greenSquareSize * 2, (float)greenSquareSize * 2};
                rayLight = {(float)greenSquareX, 0, (float)greenSquareSize, (float)screenHeight - (greenSquareSize / 2)};
                if (greenSquareY < 0) greenSquareY = 0;
                if (greenSquareX < 0) greenSquareX = 0;
                if (greenSquareY > (screenHeight - greenSquareSize)) greenSquareY = screenHeight - greenSquareSize;
                if (greenSquareX > (screenWidth - greenSquareSize)) greenSquareX = screenWidth - greenSquareSize;

                if (paused and backButton.isReleased(mousePoint)) {StopMusicStream(musPaused); PlayMusicStream(menu); currentScreen = MENU; paused = false;}

                if (!paused)
                {
                
                    if (redSquareColor.a < 255 and redSquareHealth >= redSquareOriginalHealth) redSquareColor.a += 5;
                    redSquareX += redSquareXSpeed;
                    redSquareY += redSquareYSpeed;
                    if ((redSquareX >= screenWidth - redSquareXSize) or (redSquareX <= 0)) redSquareXSpeed *= -1;
                    if ((redSquareY >= screenHeight - redSquareYSize) or (redSquareY <= 0)) redSquareYSpeed *= -1;
                    if (redSquareDestroyed) ResetRedSquare();

                    if (!rayColorReverse and rayColor.g < 255) rayColor.g+=15;
                    if (rayColor.g == 255) rayColorReverse = true;
                    if (rayColorReverse and rayColor.g > 0) rayColor.g-=15;
                    if (rayColor.g == 0) rayColorReverse = false;

                    if (greenSquareColor.a < 255) greenSquareColor.a += 5;

                    if (keyLeft and greenSquareX > 0) greenSquareX -= greenSquareSpeed;
                    if (keyRight and greenSquareX < (screenWidth - greenSquareSize)) greenSquareX += greenSquareSpeed;
                    if (keyShoot and rayEnergy > 0) rayActivated = true;
                    if (!keyShoot or rayEnergy <= 0) rayActivated = false;
                    if (!keyShoot and rayEnergy < 100) rayEnergy++; if (rayEnergy > 100) rayEnergy = 100;

                    if (rayActivated and rayEnergy > 0) rayEnergy--;
                    if (rayEnergy < 0) rayEnergy = 0;

                    if (greenSquareY < 0) greenSquareY = 0;
                    if (greenSquareX < 0) greenSquareX = 0;
                    if (greenSquareY > (screenHeight - greenSquareSize)) greenSquareY = screenHeight - greenSquareSize;
                    if (greenSquareX > (screenWidth - greenSquareSize)) greenSquareX = screenWidth - greenSquareSize;

                    enemyCollision = (CheckCollisionRecs(redSquare, greenSquareBox));
                    if (enemyCollision) GetCollisionRec(redSquare, greenSquareBox);
                    if (enemyCollision and redSquareColor.a == 255) ResetGame(), currentScreen = GAMEOVER;

                    rayCollision = (CheckCollisionRecs(redSquare, ray));
                    if (rayCollision) GetCollisionRec(redSquare, ray);
                    if (redSquareColor.a == 255 and rayCollision and rayActivated)
                    {
                        redSquareHealth--;
                        if (redSquareColor.r > 0) redSquareColor.r-=10;
                        if (redSquareXSpeed > 10) redSquareXSpeed--;
                        if (redSquareXSpeed < -10) redSquareXSpeed++;
                        if (redSquareYSpeed > 10) redSquareYSpeed--;
                        if (redSquareYSpeed < -10) redSquareYSpeed++;
                    }
                    if (redSquareHealth <= 0)
                    {
                        if (giveScore) {score++; if (score > highScore) {highScore = score; SaveStorageValue(STORAGE_POSITION_HIGHSCORE, highScore);} giveScore = false;}
                        redSquareXSpeed = 0; redSquareYSpeed = 0;
                        if (redSquareColor.a > 0) redSquareColor.a -= 20;
                        if (redSquareColor.a == 15) redSquareColor.a = 0, redSquareDestroyed = true;
                    }

                }
                
            } break;

            case GAMEOVER:
            {
                StopSound(raySound);

                StopMusicStream(mus);
                if (!IsMusicStreamPlaying(gameOver)) PlayMusicStream(gameOver);
                if (audio) UpdateMusicStream(gameOver);

                if (retryButton.isReleased(mousePoint)) StopMusicStream(gameOver), PlayMusicStream(mus), currentScreen = INGAME;
                if (backButton.isReleased(mousePoint)) StopMusicStream(gameOver), PlayMusicStream(menu), currentScreen = MENU;
            } break;
            default: break;
        }

        BeginDrawing();
            switch(currentScreen)
            {
                case MENU:
                {
                    ClearBackground(BLACK);

                    DrawTexture(checkerboardMenuTexture, 0, 0, WHITE);

                    startButton.draw(mousePoint);
                    optionsButton.draw(mousePoint);
                    exitButton.draw(mousePoint);

                    if (gradient) DrawRectangleGradientV(0, 0, screenWidth, screenHeight, BLANK, {0, 0, 0, 200});

                    DrawTextEx(GetFontDefault(), "Green Square Attacks", {(screenWidth / 2) - MeasureTextEx(GetFontDefault(), "Green Square Attacks", (float)100, 10).x/2, 75 - MeasureTextEx(GetFontDefault(), "Play", (float)100, 10).x/4}, 100, 10, WHITE);
                    DrawTextEx(GetFontDefault(), "Play", {startButton.position.x + (buttonWidth / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)150, 15).x/2, startButton.position.y + (buttonHeight / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)150, 15).x/4}, 150, 15, WHITE);
                    DrawTextEx(GetFontDefault(), "Options", {optionsButton.position.x + (buttonWidth / 2) - MeasureTextEx(GetFontDefault(), "Options", (float)150, 15).x/2, optionsButton.position.y + (buttonHeight / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)150, 15).x/4}, 150, 15, WHITE);
                    DrawTextEx(GetFontDefault(), "Exit", {exitButton.position.x + (buttonWidth / 2) - MeasureTextEx(GetFontDefault(), "Exit", (float)150, 15).x/2, exitButton.position.y + (buttonHeight / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)150, 15).x/4}, 150, 15, WHITE);
                } break;

                case OPTIONS:
                {
                    ClearBackground(BLACK);

                    DrawTexture(checkerboardMenuTexture, 0, 0, WHITE);

                    audioMiniButton.draw(mousePoint);
                    fullscreenMiniButton.draw(mousePoint);
                    vsyncMiniButton.draw(mousePoint);
                    gradientMiniButton.draw(mousePoint);
                    squaresMiniButton.draw(mousePoint);
                    backMiniButton.draw(mousePoint);

                    if (gradient) DrawRectangleGradientV(0, 0, screenWidth, screenHeight, BLANK, {0, 0, 0, 200});

                    DrawTextEx(GetFontDefault(), "Audio", {audioMiniButton.position.x + (buttonMiniWidth / 2) - MeasureTextEx(GetFontDefault(), "Audio", (float)75, 7.5).x/2, audioMiniButton.position.y + (buttonMiniHeight / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)75, 7.5).x/4}, 75, 7.5, WHITE);
                    DrawTextEx(GetFontDefault(), "Fullscreen", {fullscreenMiniButton.position.x + (buttonMiniWidth / 2) - MeasureTextEx(GetFontDefault(), "Fullscreen", (float)75, 7.5).x/2, fullscreenMiniButton.position.y + (buttonMiniHeight / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)75, 7.5).x/4}, 75, 7.5, WHITE);
                    DrawTextEx(GetFontDefault(), "V-Sync", {vsyncMiniButton.position.x + (buttonMiniWidth / 2) - MeasureTextEx(GetFontDefault(), "V-Sync", (float)75, 7.5).x/2, vsyncMiniButton.position.y + (buttonMiniHeight / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)75, 7.5).x/4}, 75, 7.5, WHITE);
                    DrawTextEx(GetFontDefault(), "Gradient", {gradientMiniButton.position.x + (buttonMiniWidth / 2) - MeasureTextEx(GetFontDefault(), "Gradient", (float)75, 7.5).x/2, gradientMiniButton.position.y + (buttonMiniHeight / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)75, 7.5).x/4}, 75, 7.5, WHITE);
                    DrawTextEx(GetFontDefault(), "Squares", {squaresMiniButton.position.x + (buttonMiniWidth / 2) - MeasureTextEx(GetFontDefault(), "Squares", (float)75, 7.5).x/2, squaresMiniButton.position.y + (buttonMiniHeight / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)75, 7.5).x/4}, 75, 7.5, WHITE);
                    DrawTextEx(GetFontDefault(), "Back", {backMiniButton.position.x + (buttonMiniWidth / 2) - MeasureTextEx(GetFontDefault(), "Back", (float)75, 7.5).x/2, backMiniButton.position.y + (buttonMiniHeight / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)75, 7.5).x/4}, 75, 7.5, WHITE);
                } break;

                case INGAME:
                {
                    if (paused) BeginShaderMode(grayscale);

                        DrawTexture(checkerboardIngameTexture, 0, 0, WHITE);

                        if (!redSquareDestroyed) DrawRectangleRec(redSquare, redSquareColor);

                        DrawRectangleRec(greenSquareBox, greenSquareColor);

                        if (!paused and gradient) DrawRectangleGradientV(0, 0, screenWidth, screenHeight, BLANK, {0, 0, 0, 200});

                        if (rayActivated) DrawRectangleRec(greenSquareLight, {255, 255, 255, 50}), DrawRectangleRec(rayLight, {255, 255, 255, 50});
                        if (rayActivated) DrawRectangleRec(ray, rayColor), DrawRectangleRec(greenSquareBox, GREEN);
                    
                    EndShaderMode();

                    if (paused) 
                    {
                        backButton.draw(mousePoint);

                        if (gradient) DrawRectangleGradientV(0, 0, screenWidth, screenHeight, BLANK, {0, 0, 0, 200});
                            
                        DrawTextEx(GetFontDefault(), "Back", {backButton.position.x + (buttonWidth / 2) - MeasureTextEx(GetFontDefault(), "Back", (float)150, 15).x/2, backButton.position.y + (buttonHeight / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)150, 15).x/4}, 150, 15, WHITE);

                        DrawTextEx(GetFontDefault(), "Paused", {screenWidth / 2.0f - MeasureTextEx(GetFontDefault(), "Paused", (float)100, 10).x/2, screenHeight / 2.0f - MeasureTextEx(GetFontDefault(), "Play", (float)100, 10).x/4}, 100, 10, WHITE);
                    }

                    DrawText(TextFormat("Energy: %03i", rayEnergy), 10, 10, 50, WHITE);
                    DrawText(TextFormat("Score: %i", score), 10, 70, 50, WHITE);
                    DrawText(TextFormat("High score: %i", highScore), 10, 130, 50, WHITE);
                } break;

                case GAMEOVER:
                {
                    ClearBackground(BLACK);

                    DrawTexture(checkerboardMenuTexture, 0, 0, WHITE);

                    retryButton.draw(mousePoint);
                    backButton.draw(mousePoint);

                    if (gradient) DrawRectangleGradientV(0, 0, screenWidth, screenHeight, BLANK, {0, 0, 0, 200});

                    DrawTextEx(GetFontDefault(), "Retry", {retryButton.position.x + (buttonWidth / 2) - MeasureTextEx(GetFontDefault(), "Retry", (float)150, 15).x/2, retryButton.position.y + (buttonHeight / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)150, 15).x/4}, 150, 15, WHITE);
                    DrawTextEx(GetFontDefault(), "Back", {backButton.position.x + (buttonWidth / 2) - MeasureTextEx(GetFontDefault(), "Back", (float)150, 15).x/2, backButton.position.y + (buttonHeight / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)150, 15).x/4}, 150, 15, WHITE);

                    DrawText(TextFormat("Energy: N/A"), 10, 10, 50, WHITE);
                    DrawText(TextFormat("Score: N/A"), 10, 70, 50, WHITE);
                    DrawText(TextFormat("High score: %i", highScore), 10, 130, 50, WHITE);
                } break;
                default: break;
            }
        EndDrawing();
    }
    UnloadShader(grayscale);
    UnloadMusicStream(menu);
    UnloadMusicStream(settings);
    UnloadMusicStream(mus);
    UnloadMusicStream(musPaused);
    UnloadMusicStream(gameOver);
    UnloadSound(raySound);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}

void ResetGame(void)
{
    redSquareX = GetRandomValue(0, screenWidth - redSquareXSize);
    redSquareY = GetRandomValue(0, screenHeight - redSquareYSize);
    redSquareXSize = GetRandomValue(redSquareMinSize, redSquareMaxSize);
    if (!squares) redSquareYSize = GetRandomValue(redSquareMinSize, redSquareMaxSize);
    else redSquareYSize = redSquareXSize;
    redSquareXSpeed = GetRandomValue(redSquareMinSpeed, redSquareMaxSpeed) / 2;
    redSquareYSpeed = GetRandomValue(redSquareMinSpeed, redSquareMaxSpeed);
    redSquareHealth = redSquareOriginalHealth;
    redSquareColor = redSquareOriginalColor;
    redSquareColor.a = 0;
    redSquareDestroyed = false;

    greenSquareX = greenSquareOriginalX;
    greenSquareY = greenSquareOriginalY;
    greenSquareColor = greenSquareOriginalColor;
    greenSquareColor.a = 0;

    rayEnergy = 100;
    rayColor = {255, 255, 0, 255};
    rayColorReverse = false;
    rayActivated = false;

    giveScore = true;
    if (score > highScore) highScore = score;
    score = 0;
}

void ResetRedSquare(void)
{
    redSquareX = GetRandomValue(0, screenWidth - redSquareXSize);
    redSquareY = GetRandomValue(0, screenHeight - redSquareYSize);
    redSquareXSize = GetRandomValue(redSquareMinSize, redSquareMaxSize);
    if (!squares) redSquareYSize = GetRandomValue(redSquareMinSize, redSquareMaxSize);
    else redSquareYSize = redSquareXSize;
    redSquareXSpeed = GetRandomValue(redSquareMinSpeed, redSquareMaxSpeed) / 2;
    redSquareYSpeed = GetRandomValue(redSquareMinSpeed, redSquareMaxSpeed);
    redSquareHealth = redSquareOriginalHealth;
    redSquareColor = redSquareOriginalColor;
    redSquareColor.a = 0;
    redSquareDestroyed = false;
    giveScore = true;
}

bool SaveStorageValue(unsigned int position, int value)
{
    bool success = false;
    int dataSize = 0;
    unsigned int newDataSize = 0;
    unsigned char *fileData = LoadFileData(STORAGE_DATA_FILE, &dataSize);
    unsigned char *newFileData = NULL;

    if (fileData != NULL)
    {
        if (dataSize <= (position*sizeof(int)))
        {
            newDataSize = (position + 1)*sizeof(int);
            newFileData = (unsigned char *)RL_REALLOC(fileData, newDataSize);

            if (newFileData != NULL)
            {
                int *dataPtr = (int *)newFileData;
                dataPtr[position] = value;
            }
            else
            {
                TraceLog(LOG_WARNING, "FILEIO: [%s] Failed to realloc data (%u), position in bytes (%u) bigger than actual file size", STORAGE_DATA_FILE, dataSize, position*sizeof(int));
                newFileData = fileData;
                newDataSize = dataSize;
            }
        }
        else
        {
            newFileData = fileData;
            newDataSize = dataSize;
            int *dataPtr = (int *)newFileData;
            dataPtr[position] = value;
        }
        success = SaveFileData(STORAGE_DATA_FILE, newFileData, newDataSize);
        RL_FREE(newFileData);
        TraceLog(LOG_INFO, "FILEIO: [%s] Saved storage value: %i", STORAGE_DATA_FILE, value);
    }
    else
    {
        TraceLog(LOG_INFO, "FILEIO: [%s] File created successfully", STORAGE_DATA_FILE);
        dataSize = (position + 1)*sizeof(int);
        fileData = (unsigned char *)RL_MALLOC(dataSize);
        int *dataPtr = (int *)fileData;
        dataPtr[position] = value;
        success = SaveFileData(STORAGE_DATA_FILE, fileData, dataSize);
        UnloadFileData(fileData);
        TraceLog(LOG_INFO, "FILEIO: [%s] Saved storage value: %i", STORAGE_DATA_FILE, value);
    }
    return success;
}

int LoadStorageValue(unsigned int position)
{
    int value = 0;
    int dataSize = 0;
    unsigned char *fileData = LoadFileData(STORAGE_DATA_FILE, &dataSize);
    if (fileData != NULL)
    {
        if (dataSize < ((int)(position*4))) TraceLog(LOG_WARNING, "FILEIO: [%s] Failed to find storage position: %i", STORAGE_DATA_FILE, position);
        else
        {
            int *dataPtr = (int *)fileData;
            value = dataPtr[position];
        }
        UnloadFileData(fileData);
        TraceLog(LOG_INFO, "FILEIO: [%s] Loaded storage value: %i", STORAGE_DATA_FILE, value);
    }
    return value;
}