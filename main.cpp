#include "include/raylib.h"
#include "button.hpp"

static void InitGame(void);
static void ResetRedSquare(void);

const int screenWidth = 1280; const int screenHeight = 720; // Setting up the screen resolution

// Game screen variable
typedef enum GameScreen {MENU, SETTINGS, INGAME, GAMEOVER} GameScreen;
GameScreen currentScreen = MENU;

// Button variables
Rectangle buttonRec = {0, 0, screenWidth / 2.5, screenHeight / 2.5};
Rectangle buttonBounds = {screenWidth / 2.0f - buttonRec.width / 2.0f, screenHeight / 2.0f - buttonRec.height / 2.0f, (float)buttonRec.width, buttonRec.height};
Color buttonColor = GREEN;
int buttonState = 0;
bool buttonAction = false;

bool paused = false; // Pause variable

// Red square variables
const int redSquareOriginalHealth = 20; // Original health
const int redSquareMinSize = screenHeight / 7.2;
const int redSquareMaxSize = screenHeight / 2.4;
const float redSquareMinSpeed = screenWidth / 128; // Minimal speed
const float redSquareMaxSpeed = screenWidth / 64; // Maximal speed
const Color redSquareOriginalColor = RED; // Original color
int redSquareSize; // Size
int redSquareX; // X position
int redSquareY; // Y position
int redSquareHealth = redSquareOriginalHealth; // Current health
float redSquareXSpeed; // Horizontal speed
float redSquareYSpeed; // Vertical speed
Color redSquareColor = redSquareOriginalColor; // Current color
bool redSquareDestroyed = false; // Status
bool redSquareXSquish = false;
bool redSquareYSquish = false;
    
// Green square variables
const int greenSquareOriginalSize = screenHeight / 7.2; // Original size
const int greenSquareOriginalX = screenWidth - greenSquareOriginalSize; // Original X position
const int greenSquareOriginalY = screenHeight - greenSquareOriginalSize; // Original Y position
const float greenSquareOriginalSpeed = screenWidth / 64; // Original speed
const Color greenSquareOriginalColor = GREEN; // Original color
int greenSquareSize = screenHeight / 7.2; // Current size
int greenSquareX = greenSquareOriginalX; // Current X position
int greenSquareY = greenSquareOriginalY; // Current Y position
float greenSquareSpeed = greenSquareOriginalSpeed; // Current speed
Color greenSquareColor = greenSquareOriginalColor;
bool enemyCollision = false; // Collision status

// Ray variables
int rayEnergy = 100; // Energy
Color rayColor = {255, 255, 0, 255}; // Color
bool rayColorReverse = false; // Color status
bool rayActivated = false; // Activation status
bool rayCollision = false; // Collision status

// Score variables
bool giveScore = true;
int score = 0;
int bestScore = 0;

int main(void)
{

    // Setting up the game
    InitWindow(screenWidth, screenHeight, "Yarix game");
    InitAudioDevice();
    SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
    SetConfigFlags(FLAG_VSYNC_HINT);

    Shader grayscale = LoadShader(0, "grayscale.fs"); // Setting up the shader

    // Setting up the music
    Music menu = LoadMusicStream("mus_menu.ogg");
    Music mus = LoadMusicStream("mus_sampletext.ogg");
    Music musPaused = LoadMusicStream("mus_sampletextpaused.ogg");
    Music gameOver = LoadMusicStream("mus_gameover.ogg");
    PlayMusicStream(menu);

    Sound raySound = LoadSound("sfx_ray.ogg"); // Setting up the sound

    // Creating the rectangles
    Rectangle redSquare;
    Rectangle greenSquareBox;
    Rectangle ray;
    Rectangle greenSquareLight;
    Rectangle rayLight;

    // Creating the buttons
    Button startButton;
    startButton.position = {screenWidth / 2 - (startButton.width / 2), (screenHeight / 5)};
    Button settingsButton;
    settingsButton.position = {screenWidth / 2 - (settingsButton.width / 2), screenHeight - (settingsButton.height) - (screenHeight / 20)};
    Button retryButton;
    retryButton.position = {screenWidth / 2 - (retryButton.width / 2), screenHeight / 2 - (retryButton.height / 2)};
    Button testButton;
    testButton.width = screenWidth / 1.5;
    testButton.position = {screenWidth / 2 - (testButton.width / 2), (screenHeight / 5)};
    Button backButton;
    backButton.position = {screenWidth / 2 - (backButton.width / 2), screenHeight - (backButton.height) - (screenHeight / 20)};

    // Creating a checkerboard background for menus
    Image checkerboardMenuImage = GenImageChecked(screenWidth, screenHeight, screenHeight / 5, screenHeight / 5, DARKGRAY, BLACK);
    Texture2D checkerboardMenuTexture = LoadTextureFromImage(checkerboardMenuImage);

    // Creating a checkerboard
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
                if (settingsButton.isReleased(mousePoint)) currentScreen = SETTINGS;
                if (startButton.isReleased(mousePoint)) InitGame(), PlayMusicStream(mus), currentScreen = INGAME;
            } break;

            case SETTINGS:
            {
                UpdateMusicStream(menu);
                if (backButton.isReleased(mousePoint)) currentScreen = MENU;
                if (testButton.isReleased(mousePoint))
                {
                    ToggleFullscreen();
                }
            } break;

            case INGAME:
            {
                // Updating the music
                UpdateMusicStream(mus);
                UpdateMusicStream(musPaused);

                // Playing the sound
                if (rayActivated and !IsSoundPlaying(raySound)) PlaySound(raySound);
                if (!rayActivated and IsSoundPlaying(raySound)) StopSound(raySound);

                // Letting the player to pause the game
                if (IsKeyPressed(KEY_ENTER) or IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT)) paused = !paused;
                if (paused and IsMusicStreamPlaying(mus)) StopMusicStream(mus), PlayMusicStream(musPaused);
                if (!paused and !IsMusicStreamPlaying(mus)) StopMusicStream(musPaused), PlayMusicStream(mus);

                // Setting up the red square
                redSquare = {(float)redSquareX, (float)redSquareY, (float)redSquareSize, (float)redSquareSize};
                if (redSquareY < 0) redSquareY = 0;
                if (redSquareX < 0) redSquareX = 0;
                if (redSquareY > (screenHeight - redSquareSize)) redSquareY = screenHeight - redSquareSize;
                if (redSquareX > (screenWidth - redSquareSize)) redSquareX = screenWidth - redSquareSize;

                // Setting up the green square and ray
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
                if (rayActivated and rayEnergy > 0) rayEnergy--; if (rayEnergy < 0) rayEnergy = 0;
                if (greenSquareY < 0) greenSquareY = 0;
                if (greenSquareX < 0) greenSquareX = 0;
                if (greenSquareY > (screenHeight - greenSquareSize)) greenSquareY = screenHeight - greenSquareSize;
                if (greenSquareX > (screenWidth - greenSquareSize)) greenSquareX = screenWidth - greenSquareSize;

                // Collisions

                // Enemy collision
                enemyCollision = (CheckCollisionRecs(redSquare, greenSquareBox));
                if (enemyCollision) GetCollisionRec(redSquare, greenSquareBox);
                if (enemyCollision and redSquareColor.a == 255) InitGame(), currentScreen = GAMEOVER;

                // Ray collision
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
                    if (giveScore) score++; if (bestScore < score) bestScore = score; giveScore = false;
                    redSquareXSpeed = 0; redSquareYSpeed = 0;
                    if (redSquareColor.a > 0) redSquareColor.a -= 20;
                    if (redSquareColor.a == 15) redSquareColor.a = 0, redSquareDestroyed = true;
                }

                }
            } break;

            case GAMEOVER:
            {
                StopMusicStream(mus);
                StopSound(raySound);
                if (!IsMusicStreamPlaying(gameOver)) PlayMusicStream(gameOver);
                UpdateMusicStream(gameOver);
                retryButton.changeColor(mousePoint);
                if (retryButton.isReleased(mousePoint)) StopMusicStream(gameOver), PlayMusicStream(mus), currentScreen = INGAME;
            } break;
            default: break;
        }

        // Drawing
        BeginDrawing();
            switch(currentScreen)
            {
                case MENU:
                {
                    ClearBackground(BLACK);
                    DrawTexture(checkerboardMenuTexture, 0, 0, WHITE);
                    startButton.draw(mousePoint);
                    settingsButton.draw(mousePoint);
                    DrawRectangleGradientV(0, 0, screenWidth, screenHeight, BLANK, {0, 0, 0, 200});
                    DrawTextEx(GetFontDefault(), "Green Square Attacks", {(screenWidth / 2) - MeasureTextEx(GetFontDefault(), "Green Square Attacks", (float)100, 10).x/2, 75 - MeasureTextEx(GetFontDefault(), "Play", (float)100, 10).x/4}, 100, 10, WHITE);
                    DrawTextEx(GetFontDefault(), "Play", {startButton.position.x + (startButton.width / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)150, 15).x/2, startButton.position.y + (startButton.height / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)150, 15).x/4}, 150, 15, WHITE);
                    DrawTextEx(GetFontDefault(), "Options", {settingsButton.position.x + (settingsButton.width / 2) - MeasureTextEx(GetFontDefault(), "Options", (float)150, 15).x/2, settingsButton.position.y + (settingsButton.height / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)150, 15).x/4}, 150, 15, WHITE);
                } break;

                case SETTINGS:
                {
                    ClearBackground(BLACK);
                    DrawTexture(checkerboardMenuTexture, 0, 0, WHITE);
                    testButton.draw(mousePoint);
                    backButton.draw(mousePoint);
                    DrawRectangleGradientV(0, 0, screenWidth, screenHeight, BLANK, {0, 0, 0, 200});
                    DrawTextEx(GetFontDefault(), "Fullscreen", {testButton.position.x + (testButton.width / 2) - MeasureTextEx(GetFontDefault(), "Fullscreen", (float)150, 15).x/2, testButton.position.y + (testButton.height / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)150, 15).x/4}, 150, 15, WHITE);
                    DrawTextEx(GetFontDefault(), "Back", {backButton.position.x + (backButton.width / 2) - MeasureTextEx(GetFontDefault(), "Back", (float)150, 15).x/2, backButton.position.y + (backButton.height / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)150, 15).x/4}, 150, 15, WHITE);
                } break;

                case INGAME:
                {
                    if (paused) BeginShaderMode(grayscale);

                        DrawTexture(checkerboardIngameTexture, 0, 0, WHITE); // Drawing the background

                        // Drawing the red square and the green square
                        if (!redSquareDestroyed) DrawRectangleRec(redSquare, redSquareColor);
                        DrawRectangleRec(greenSquareBox, greenSquareColor);

                        DrawRectangleGradientV(0, 0, screenWidth, screenHeight, BLANK, {0, 0, 0, 200}); // Drawing the gradient

                        // Drawing the ray and making the green square glow when the ray is activated
                        if (rayActivated) DrawRectangleRec(greenSquareLight, {255, 255, 255, 50}), DrawRectangleRec(rayLight, {255, 255, 255, 50});
                        if (rayActivated) DrawRectangleRec(ray, rayColor), DrawRectangleRec(greenSquareBox, GREEN);

                        DrawText(TextFormat("Energy: %03i", rayEnergy), 10, 10, 50, WHITE); // Displaying energy count on the screen

                        DrawText(TextFormat("Score: %i", score), 10, 70, 50, WHITE); // Displaying score on the screen

                        DrawText(TextFormat("Best score: %i", bestScore), 10, 130, 50, WHITE); // Displaying best score on the screen

                        if (paused) DrawTextEx(GetFontDefault(), "Paused", {screenWidth / 2.0f - MeasureTextEx(GetFontDefault(), "Paused", (float)50, 5).x/2, screenHeight / 2.0f - MeasureTextEx(GetFontDefault(), "Play", (float)50, 5).x/4}, 50, 5, WHITE); // Displaying text if the game is paused

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
                    DrawText(TextFormat("Score: %i", bestScore), 10, 70, 50, WHITE);
                    DrawText(TextFormat("Best score: %i", bestScore), 10, 130, 50, WHITE);
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

void InitGame(void)
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
    if (score > bestScore) bestScore = score;
    bestScore = score;
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