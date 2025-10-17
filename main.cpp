#include "include/raylib.h"
#include "button.hpp"

static void InitGame(void);
static void ResetRedCircle(void);

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

// Red circle variables
const int redCircleOriginalHealth = 20; // Originalinal health
const int redCircleMinSize = screenHeight / 7.2;
const int redCircleMaxSize = screenHeight / 2.4;
const float redCircleMinSpeed = screenWidth / 128; // Minimal speed
const float redCircleMaxSpeed = screenWidth / 64; // Maximal speed
const Color redCircleOriginalColor = RED; // Originalinal color
int redCircleSize; // Size
int redCircleX; // X position
int redCircleY; // Y position
int redCircleHealth = redCircleOriginalHealth; // Current health
float redCircleXSpeed; // Horizontal speed
float redCircleYSpeed; // Vertical speed
Color redCircleColor = redCircleOriginalColor; // Current color
bool redCircleDestroyed = false; // Status
bool redCircleXSquish = false;
bool redCircleYSquish = false;
    
// Green circle variables
const int greenCircleOriginalSize = screenHeight / 7.2; // Originalinal size
const int greenCircleOriginalX = screenWidth - greenCircleOriginalSize; // Originalinal X position
const int greenCircleOriginalY = screenHeight - greenCircleOriginalSize; // Originalinal Y position
const float greenCircleOriginalSpeed = screenWidth / 64; // Originalinal speed
const Color greenCircleOriginalColor = GREEN; // Originalinal color
int greenCircleSize = screenHeight / 7.2; // Current size
int greenCircleX = greenCircleOriginalX; // Current X position
int greenCircleY = greenCircleOriginalY; // Current Y position
float greenCircleSpeed = greenCircleOriginalSpeed; // Current speed
Color greenCircleColor = greenCircleOriginalColor;
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
    SetConfigFlags(FLAG_MSAA_4X_HINT);
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

    Sound raySound = LoadSound("ray.ogg"); // Setting up the sound

    // Creating the rectangles
    Rectangle redCircle;
    Rectangle greenCircleBox;
    Rectangle ray;
    Rectangle greenCircleLight;
    Rectangle rayLight;

    // Creating the buttons
    Button startButton;
    startButton.position = {screenWidth / 2 - (startButton.width / 2), (screenHeight / 20)};
    Button settingsButton;
    settingsButton.position = {screenWidth / 2 - (settingsButton.width / 2), screenHeight - (settingsButton.height) - (screenHeight / 20)};
    Button retryButton;
    retryButton.position = {screenWidth / 2 - (retryButton.width / 2), screenHeight / 2 - (retryButton.height / 2)};
    Button testButton;
    testButton.width = screenWidth / 1.5;
    testButton.position = {screenWidth / 2 - (testButton.width / 2), (screenHeight / 20)};
    Button backButton;
    backButton.position = {screenWidth / 2 - (backButton.width / 2), screenHeight - (backButton.height) - (screenHeight / 20)};

    // Creating a checkerboard
    Image checkerboardImage = GenImageChecked(screenWidth, screenHeight, screenHeight / 5, screenHeight / 5, PURPLE, DARKPURPLE);
    Texture2D checkerboardTexture = LoadTextureFromImage(checkerboardImage);

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
                    SetConfigFlags(FLAG_FULLSCREEN_MODE);
                    CloseWindow();
                    InitWindow(screenWidth, screenHeight, "Yarix game");
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

                // Setting up the red circle
                redCircle = {(float)redCircleX, (float)redCircleY, (float)redCircleSize, (float)redCircleSize};
                if (redCircleY < 0) redCircleY = 0;
                if (redCircleX < 0) redCircleX = 0;
                if (redCircleY > (screenHeight - redCircleSize)) redCircleY = screenHeight - redCircleSize;
                if (redCircleX > (screenWidth - redCircleSize)) redCircleX = screenWidth - redCircleSize;

                // Setting up the green circle and ray
                greenCircleBox = {(float)greenCircleX, (float)greenCircleY, (float)greenCircleSize, (float)greenCircleSize};
                ray = {(float)greenCircleX + (greenCircleSize / 4), 0, (float)greenCircleSize / 2, (float)screenHeight - (greenCircleSize / 2)};
                greenCircleLight = {(float)greenCircleX - (greenCircleSize / 2), (float)greenCircleY - (greenCircleSize / 2), (float)greenCircleSize * 2, (float)greenCircleSize * 2};
                rayLight = {(float)greenCircleX, 0, (float)greenCircleSize, (float)screenHeight - (greenCircleSize / 2)};
                if (greenCircleY < 0) greenCircleY = 0;
                if (greenCircleX < 0) greenCircleX = 0;
                if (greenCircleY > (screenHeight - greenCircleSize)) greenCircleY = screenHeight - greenCircleSize;
                if (greenCircleX > (screenWidth - greenCircleSize)) greenCircleX = screenWidth - greenCircleSize;

                if (!paused)
                {
                
                if (redCircleColor.a < 255 and redCircleHealth >= redCircleOriginalHealth) redCircleColor.a += 5;
                redCircleX += GetFrameTime()*60.0f*redCircleXSpeed;
                redCircleY += GetFrameTime()*60.0f*redCircleYSpeed;
                if ((redCircleX >= screenWidth - redCircleSize) or (redCircleX <= 0)) redCircleXSpeed *= -1;
                if ((redCircleY >= screenHeight - redCircleSize) or (redCircleY <= 0)) redCircleYSpeed *= -1;
                if (redCircleDestroyed) ResetRedCircle();

                if (!rayColorReverse and rayColor.g < 255) rayColor.g+=15;
                if (rayColor.g == 255) rayColorReverse = true;
                if (rayColorReverse and rayColor.g > 0) rayColor.g-=15;
                if (rayColor.g == 0) rayColorReverse = false;
                if (greenCircleColor.a < 255) greenCircleColor.a += 5;
                if ((IsKeyDown(KEY_A)) or (IsKeyDown(KEY_LEFT)) or (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) < -0.5 or (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT))) and greenCircleX > 0) greenCircleX -= (int)(GetFrameTime()*60.0f*greenCircleSpeed);
                if ((IsKeyDown(KEY_D)) or (IsKeyDown(KEY_RIGHT)) or (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) > 0.5 or (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT))) and greenCircleX < (screenWidth - greenCircleSize)) greenCircleX += (int)(GetFrameTime()*60.0f*greenCircleSpeed);
                if (((IsKeyDown(KEY_SPACE)) or IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) and rayEnergy > 0) rayActivated = true;
                if (!((IsKeyDown(KEY_SPACE)) or IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) or rayEnergy <= 0) rayActivated = false;
                if (!((IsKeyDown(KEY_SPACE)) or IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) and rayEnergy < 100) rayEnergy+=GetFrameTime()*60.0f*2; if (rayEnergy > 100) rayEnergy = 100;
                if (rayActivated and rayEnergy > 0) rayEnergy-=GetFrameTime()*60.0f*1; if (rayEnergy < 0) rayEnergy = 0;
                if (greenCircleY < 0) greenCircleY = 0;
                if (greenCircleX < 0) greenCircleX = 0;
                if (greenCircleY > (screenHeight - greenCircleSize)) greenCircleY = screenHeight - greenCircleSize;
                if (greenCircleX > (screenWidth - greenCircleSize)) greenCircleX = screenWidth - greenCircleSize;

                // Collisions

                // Enemy collision
                enemyCollision = (CheckCollisionRecs(redCircle, greenCircleBox));
                if (enemyCollision) GetCollisionRec(redCircle, greenCircleBox);
                if (enemyCollision and redCircleColor.a == 255) InitGame(), currentScreen = GAMEOVER;

                // Ray collision
                rayCollision = (CheckCollisionRecs(redCircle, ray));
                if (rayCollision) GetCollisionRec(redCircle, ray);
                if (redCircleColor.a == 255 and rayCollision and rayActivated)
                {
                    redCircleHealth--;
                    if (redCircleColor.r > 0) redCircleColor.r-=10;
                    if (redCircleXSpeed > 10) redCircleXSpeed--;
                    if (redCircleXSpeed < -10) redCircleXSpeed++;
                    if (redCircleYSpeed > 10) redCircleYSpeed--;
                    if (redCircleYSpeed < -10) redCircleYSpeed++;
                }
                if (redCircleHealth <= 0)
                {
                    if (giveScore) score++; if (bestScore < score) bestScore = score; giveScore = false;
                    redCircleXSpeed = 0; redCircleYSpeed = 0;
                    if (redCircleColor.a > 0) redCircleColor.a -= 20;
                    if (redCircleColor.a == 15) redCircleColor.a = 0, redCircleDestroyed = true;
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
                    startButton.draw(mousePoint);
                    settingsButton.draw(mousePoint);
                    DrawRectangleGradientV(0, 0, screenWidth, screenHeight, BLANK, {0, 0, 0, 175});
                    DrawTextEx(GetFontDefault(), "Play", {startButton.position.x + (startButton.width / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)150, 15).x/2, startButton.position.y + (startButton.height / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)150, 15).x/4}, 150, 15, WHITE);
                    DrawTextEx(GetFontDefault(), "Options", {settingsButton.position.x + (settingsButton.width / 2) - MeasureTextEx(GetFontDefault(), "Options", (float)150, 15).x/2, settingsButton.position.y + (settingsButton.height / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)150, 15).x/4}, 150, 15, WHITE);
                } break;

                case SETTINGS:
                {
                    ClearBackground(BLACK);
                    testButton.draw(mousePoint);
                    backButton.draw(mousePoint);
                    DrawRectangleGradientV(0, 0, screenWidth, screenHeight, BLANK, {0, 0, 0, 175});
                    DrawTextEx(GetFontDefault(), "Fullscreen", {testButton.position.x + (testButton.width / 2) - MeasureTextEx(GetFontDefault(), "Fullscreen", (float)150, 15).x/2, testButton.position.y + (testButton.height / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)150, 15).x/4}, 150, 15, WHITE);
                    DrawTextEx(GetFontDefault(), "Back", {backButton.position.x + (backButton.width / 2) - MeasureTextEx(GetFontDefault(), "Back", (float)150, 15).x/2, backButton.position.y + (backButton.height / 2) - MeasureTextEx(GetFontDefault(), "Play", (float)150, 15).x/4}, 150, 15, WHITE);
                } break;

                case INGAME:
                {
                    if (paused) BeginShaderMode(grayscale);

                        DrawTexture(checkerboardTexture, 0, 0, WHITE); // Drawing the background

                        // Drawing the red circle and the green circle
                        if (!redCircleDestroyed) DrawRectangleRounded(redCircle, true, 25, redCircleColor);
                        DrawRectangleRounded(greenCircleBox, true, 25, greenCircleColor);

                        DrawRectangleGradientV(0, 0, screenWidth, screenHeight, BLANK, {0, 0, 0, 175}); // Drawing the gradient

                        // Drawing the ray and making the green circle glow when the ray is activated
                        if (rayActivated) DrawRectangleRounded(greenCircleLight, true, 25, {255, 255, 255, 50}), DrawRectangleRec(rayLight, {255, 255, 255, 50});
                        if (rayActivated) DrawRectangleRec(ray, rayColor), DrawRectangleRounded(greenCircleBox, true, 25, GREEN);

                        DrawText(TextFormat("Energy: %03i", rayEnergy), 10, 10, 50, WHITE); // Displaying energy count on the screen

                        DrawText(TextFormat("Score: %i", score), 10, 70, 50, WHITE); // Displaying score on the screen

                        DrawText(TextFormat("Best score: %i", bestScore), 10, 130, 50, WHITE); // Displaying best score on the screen

                        if (paused) DrawTextEx(GetFontDefault(), "Paused", {screenWidth / 2.0f - MeasureTextEx(GetFontDefault(), "Paused", (float)50, 5).x/2, screenHeight / 2.0f - MeasureTextEx(GetFontDefault(), "Play", (float)50, 5).x/4}, 50, 5, WHITE); // Displaying text if the game is paused

                    EndShaderMode();
                } break;

                case GAMEOVER:
                {
                    ClearBackground(BLACK);
                    retryButton.draw(mousePoint);
                    DrawRectangleGradientV(0, 0, screenWidth, screenHeight, BLANK, {0, 0, 0, 175});
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
    redCircleX = GetRandomValue(0, screenWidth - redCircleSize);
    redCircleY = GetRandomValue(0, screenHeight - redCircleSize);
    redCircleSize = GetRandomValue(redCircleMinSize, redCircleMaxSize);
    redCircleXSpeed = GetRandomValue(redCircleMinSpeed, redCircleMaxSpeed);
    redCircleYSpeed = GetRandomValue(redCircleMinSpeed, redCircleMaxSpeed);
    redCircleHealth = redCircleOriginalHealth;
    redCircleColor = redCircleOriginalColor;
    redCircleColor.a = 0;
    redCircleDestroyed = false;
    greenCircleX = greenCircleOriginalX;
    greenCircleY = greenCircleOriginalY;
    greenCircleColor = greenCircleOriginalColor;
    greenCircleColor.a = 0;
    rayEnergy = 100;
    rayColor = {255, 255, 0, 255};
    rayColorReverse = false;
    rayActivated = false;
    giveScore = true;
    if (score > bestScore) bestScore = score;
    bestScore = score;
    score = 0;
}

void ResetRedCircle(void)
{
    redCircleX = GetRandomValue(0, screenWidth - redCircleSize);
    redCircleY = GetRandomValue(0, screenHeight - redCircleSize);
    redCircleSize = GetRandomValue(redCircleMinSize, redCircleMaxSize);
    redCircleXSpeed = GetRandomValue(redCircleMinSpeed, redCircleMaxSpeed);
    redCircleYSpeed = GetRandomValue(redCircleMinSpeed, redCircleMaxSpeed);
    redCircleHealth = redCircleOriginalHealth;
    redCircleColor = redCircleOriginalColor;
    redCircleColor.a = 0;
    redCircleDestroyed = false;
    giveScore = true;
}