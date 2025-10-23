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

Rectangle buttonRec = {0, 0, screenWidth / 2.5, screenHeight / 2.5};
Rectangle buttonBounds = {screenWidth / 2.0f - buttonRec.width / 2.0f, screenHeight / 2.0f - buttonRec.height / 2.0f, (float)buttonRec.width, buttonRec.height};
Color buttonColor = GREEN;
int buttonState = 0;
bool buttonAction = false;

bool paused = false;

const int redSquareOriginalHealth = 20;
const int redSquareMinSize = screenHeight / 7.2;
const int redSquareMaxSize = screenHeight / 2.4;
const float redSquareMinSpeed = screenWidth / 96;
const float redSquareMaxSpeed = screenWidth / 64;
const Color redSquareOriginalColor = RED;
int redSquareSize;
int redSquareX;
int redSquareY;
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
int greenSquareX = greenSquareOriginalX;
int greenSquareY = greenSquareOriginalY;
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
    InitWindow(screenWidth, screenHeight, "Yarix game");
    InitAudioDevice();
    SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
    SetConfigFlags(FLAG_VSYNC_HINT);

    Shader grayscale = LoadShader(0, "grayscale.fs");

    Music menu = LoadMusicStream("mus_menu.ogg");
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
    startButton.position = {(screenWidth / 2) - (startButton.width / 2), (screenHeight / 5)};
    Button optionsButton;
    optionsButton.position = {(screenWidth / 2) - (optionsButton.width / 2), screenHeight / 5 + optionsButton.height};
    Button exitButton;
    exitButton.position = {screenWidth / 2 - (optionsButton.width / 2), (screenHeight / 5) + optionsButton.height + exitButton.height};
    Button AudioButton;
    AudioButton.position = {(screenWidth / 2) - (AudioButton.width / 2), (screenHeight / 5)};
    Button fullscreenButton;
    fullscreenButton.position = {screenWidth / 2 - (AudioButton.width / 2), (screenHeight / 5) + fullscreenButton.height};
    Button backButton;
    backButton.position = {screenWidth / 2 - (AudioButton.width / 2), (screenHeight / 5) + fullscreenButton.height + backButton.height};
    Button retryButton;
    retryButton.position = {screenWidth / 2 - (retryButton.width / 2), screenHeight / 2 - (retryButton.height / 2)};

    Image checkerboardMenuImage = GenImageChecked(screenWidth, screenHeight, screenHeight / 5, screenHeight / 5, DARKGRAY, BLACK);
    Texture2D checkerboardMenuTexture = LoadTextureFromImage(checkerboardMenuImage);

    Image checkerboardIngameImage = GenImageChecked(screenWidth, screenHeight, screenHeight / 5, screenHeight / 5, BLUE, DARKBLUE);
    Texture2D checkerboardIngameTexture = LoadTextureFromImage(checkerboardIngameImage);

    while (!WindowShouldClose())
    {
        Vector2 mousePoint = GetMousePosition();
        switch(currentScreen)
        {
            case MENU:
            {
                UpdateMusicStream(menu);

                highScore = LoadStorageValue(STORAGE_POSITION_HIGHSCORE);

                if (startButton.isReleased(mousePoint)) ResetGame(), PlayMusicStream(mus), currentScreen = INGAME;
                if (optionsButton.isReleased(mousePoint)) currentScreen = OPTIONS;
                if (exitButton.isReleased(mousePoint)) {CloseWindow(); return 0;}
            } break;

            case OPTIONS:
            {
                UpdateMusicStream(menu);

                if (fullscreenButton.isReleased(mousePoint)) ToggleFullscreen();
                if (backButton.isReleased(mousePoint)) currentScreen = MENU;
            } break;

            case INGAME:
            {

                UpdateMusicStream(mus);
                UpdateMusicStream(musPaused);

                if (rayActivated and !IsSoundPlaying(raySound)) PlaySound(raySound);
                if (!rayActivated and IsSoundPlaying(raySound)) StopSound(raySound);

                if (IsKeyPressed(KEY_ENTER) or IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT)) paused = !paused;
                if (paused and IsMusicStreamPlaying(mus)) StopMusicStream(mus), PlayMusicStream(musPaused);
                if (!paused and !IsMusicStreamPlaying(mus)) StopMusicStream(musPaused), PlayMusicStream(mus);

                redSquare = {(float)redSquareX, (float)redSquareY, (float)redSquareSize, (float)redSquareSize};
                if (redSquareY < 0) redSquareY = 0;
                if (redSquareX < 0) redSquareX = 0;
                if (redSquareY > (screenHeight - redSquareSize)) redSquareY = screenHeight - redSquareSize;
                if (redSquareX > (screenWidth - redSquareSize)) redSquareX = screenWidth - redSquareSize;

                greenSquareBox = {(float)greenSquareX, (float)greenSquareY, (float)greenSquareSize, (float)greenSquareSize};
                ray = {(float)greenSquareX + (greenSquareSize / 4), 0, (float)greenSquareSize / 2, (float)screenHeight - (greenSquareSize / 2)};
                greenSquareLight = {(float)greenSquareX - (greenSquareSize / 2), (float)greenSquareY - (greenSquareSize / 2), (float)greenSquareSize * 2, (float)greenSquareSize * 2};
                rayLight = {(float)greenSquareX, 0, (float)greenSquareSize, (float)screenHeight - (greenSquareSize / 2)};
                if (greenSquareY < 0) greenSquareY = 0;
                if (greenSquareX < 0) greenSquareX = 0;
                if (greenSquareY > (screenHeight - greenSquareSize)) greenSquareY = screenHeight - greenSquareSize;
                if (greenSquareX > (screenWidth - greenSquareSize)) greenSquareX = screenWidth - greenSquareSize;

                if (!paused)
                {
                
                if (redSquareColor.a < 255 and redSquareHealth >= redSquareOriginalHealth) redSquareColor.a += 5;
                redSquareX += (int)(GetFrameTime()*60.0f*redSquareXSpeed);
                redSquareY += (int)(GetFrameTime()*60.0f*redSquareYSpeed);
                if ((redSquareX >= screenWidth - redSquareSize) or (redSquareX <= 0)) redSquareXSpeed *= -1;
                if ((redSquareY >= screenHeight - redSquareSize) or (redSquareY <= 0)) redSquareYSpeed *= -1;
                if (redSquareDestroyed) ResetRedSquare();

                if (!rayColorReverse and rayColor.g < 255) rayColor.g+=15;
                if (rayColor.g == 255) rayColorReverse = true;
                if (rayColorReverse and rayColor.g > 0) rayColor.g-=15;
                if (rayColor.g == 0) rayColorReverse = false;

                if (greenSquareColor.a < 255) greenSquareColor.a += 5;

                if ((IsKeyDown(KEY_A)) or (IsKeyDown(KEY_LEFT)) or (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) < -0.5 or (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT))) and greenSquareX > 0) greenSquareX -= (int)(GetFrameTime()*60.0f*greenSquareSpeed);
                if ((IsKeyDown(KEY_D)) or (IsKeyDown(KEY_RIGHT)) or (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) > 0.5 or (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT))) and greenSquareX < (screenWidth - greenSquareSize)) greenSquareX += (int)(GetFrameTime()*60.0f*greenSquareSpeed);
                if (((IsKeyDown(KEY_SPACE)) or IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) and rayEnergy > 0) rayActivated = true;
                if (!((IsKeyDown(KEY_SPACE)) or IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) or rayEnergy <= 0) rayActivated = false;
                if (!((IsKeyDown(KEY_SPACE)) or IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) and rayEnergy < 100) rayEnergy++; if (rayEnergy > 100) rayEnergy = 100;

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
                UpdateMusicStream(gameOver);

                retryButton.changeColor(mousePoint);

                if (retryButton.isReleased(mousePoint)) StopMusicStream(gameOver), PlayMusicStream(mus), currentScreen = INGAME;
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

                    DrawRectangleGradientV(0, 0, screenWidth, screenHeight, BLANK, {0, 0, 0, 200});

                    DrawTextEx(GetFontDefault(), "Green Square Attacks", {(screenWidth / 2) - MeasureTextEx(GetFontDefault(), "Green Square Attacks", (float)100, 10).x/2, 75 - MeasureTextEx(GetFontDefault(), "Play", (float)100, 10).x/4}, 100, 10, WHITE);
                    DrawTextEx(GetFontDefault(), "Play", {startButton.position.x + (startButton.width / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)150, 15).x/2, startButton.position.y + (startButton.height / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)150, 15).x/4}, 150, 15, WHITE);
                    DrawTextEx(GetFontDefault(), "Options", {optionsButton.position.x + (optionsButton.width / 2) - MeasureTextEx(GetFontDefault(), "Options", (float)150, 15).x/2, optionsButton.position.y + (optionsButton.height / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)150, 15).x/4}, 150, 15, WHITE);
                    DrawTextEx(GetFontDefault(), "Exit", {exitButton.position.x + (exitButton.width / 2) - MeasureTextEx(GetFontDefault(), "Exit", (float)150, 15).x/2, exitButton.position.y + (exitButton.height / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)150, 15).x/4}, 150, 15, WHITE);
                } break;

                case OPTIONS:
                {
                    ClearBackground(BLACK);

                    DrawTexture(checkerboardMenuTexture, 0, 0, WHITE);

                    fullscreenButton.draw(mousePoint);
                    backButton.draw(mousePoint);

                    DrawRectangleGradientV(0, 0, screenWidth, screenHeight, BLANK, {0, 0, 0, 200});

                    DrawTextEx(GetFontDefault(), "Fullscreen", {fullscreenButton.position.x + (fullscreenButton.width / 2) - MeasureTextEx(GetFontDefault(), "Fullscreen", (float)150, 15).x/2, fullscreenButton.position.y + (fullscreenButton.height / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)150, 15).x/4}, 150, 15, WHITE);
                    DrawTextEx(GetFontDefault(), "Back", {backButton.position.x + (backButton.width / 2) - MeasureTextEx(GetFontDefault(), "Back", (float)150, 15).x/2, backButton.position.y + (backButton.height / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)150, 15).x/4}, 150, 15, WHITE);
                } break;

                case INGAME:
                {
                    if (paused) BeginShaderMode(grayscale);

                        DrawTexture(checkerboardIngameTexture, 0, 0, WHITE);

                        if (!redSquareDestroyed) DrawRectangleRec(redSquare, redSquareColor);

                        DrawRectangleRec(greenSquareBox, greenSquareColor);

                        DrawRectangleGradientV(0, 0, screenWidth, screenHeight, BLANK, {0, 0, 0, 200});

                        if (rayActivated) DrawRectangleRec(greenSquareLight, {255, 255, 255, 50}), DrawRectangleRec(rayLight, {255, 255, 255, 50});
                        if (rayActivated) DrawRectangleRec(ray, rayColor), DrawRectangleRec(greenSquareBox, GREEN);

                        DrawText(TextFormat("Energy: %03i", rayEnergy), 10, 10, 50, WHITE);
                        DrawText(TextFormat("Score: %i", score), 10, 70, 50, WHITE);
                        DrawText(TextFormat("High score: %i", highScore), 10, 130, 50, WHITE);

                        if (paused) DrawTextEx(GetFontDefault(), "Paused", {screenWidth / 2.0f - MeasureTextEx(GetFontDefault(), "Paused", (float)50, 5).x/2, screenHeight / 2.0f - MeasureTextEx(GetFontDefault(), "Play", (float)50, 5).x/4}, 50, 5, WHITE);

                    EndShaderMode();
                } break;

                case GAMEOVER:
                {
                    ClearBackground(BLACK);

                    DrawTexture(checkerboardMenuTexture, 0, 0, WHITE);

                    retryButton.draw(mousePoint);

                    DrawRectangleGradientV(0, 0, screenWidth, screenHeight, BLANK, {0, 0, 0, 200});

                    DrawTextEx(GetFontDefault(), "Retry", {screenWidth / 2.0f - MeasureTextEx(GetFontDefault(), "Retry", (float)150, 15).x/2, screenHeight / 2.0f - MeasureTextEx(GetFontDefault(), "Play", (float)150, 15).x/4}, 150, 15, WHITE);

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
    redSquareX = GetRandomValue(0, screenWidth - redSquareSize);
    redSquareY = GetRandomValue(0, screenHeight - redSquareSize);
    redSquareSize = GetRandomValue(redSquareMinSize, redSquareMaxSize);
    redSquareXSpeed = GetRandomValue(redSquareMinSpeed, redSquareMaxSpeed);
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
    redSquareX = GetRandomValue(0, screenWidth - redSquareSize);
    redSquareY = GetRandomValue(0, screenHeight - redSquareSize);
    redSquareSize = GetRandomValue(redSquareMinSize, redSquareMaxSize);
    redSquareXSpeed = GetRandomValue(redSquareMinSpeed, redSquareMaxSpeed);
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