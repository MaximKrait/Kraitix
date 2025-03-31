#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>

// Константы
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int PLAYER_SIZE = 20;
const int PLAYER_SPEED = 5;
const int INITIAL_LIVES = 3;
const Uint32 FRAME_DELAY = 16; // Для 60 FPS
const int ENEMY_SIZE = 20;
const int ENEMY_SPEED = 2;
const int NUM_ENEMIES = 5;
const int SCORE_INCREMENT_INTERVAL = 1000; // 1 секунда
const int IMMUNITY_DURATION = 3000; // 3 секунды

// Глобальные переменные
SDL_Window* gWindow = nullptr;
SDL_Renderer* gRenderer = nullptr;
TTF_Font* gFont = nullptr;

// Структура для врагов
struct Enemy {
    int x, y;
    bool active;
};

// Функция инициализации SDL и TTF
bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }

    return true;
}

// Функция закрытия
void close() {
    if (gFont != nullptr) {
        TTF_CloseFont(gFont);
        gFont = nullptr;
    }

    if (gRenderer != nullptr) {
        SDL_DestroyRenderer(gRenderer);
        gRenderer = nullptr;
    }

    if (gWindow != nullptr) {
        SDL_DestroyWindow(gWindow);
        gWindow = nullptr;
    }

    TTF_Quit();
    SDL_Quit();
}

// Функция рендеринга текста
void renderText(const std::string& message, int x, int y, SDL_Color color) {
    if (gFont == nullptr) {
        std::cerr << "Font not loaded!" << std::endl;
        return;
    }

    SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, message.c_str(), color);
    if (textSurface != nullptr) {
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
        if (textTexture != nullptr) {
            // Центрируем текст по горизонтали
            int textWidth = textSurface->w;
            int textHeight = textSurface->h;
            SDL_Rect renderQuad = { x - textWidth / 16, y, textWidth, textHeight };

            SDL_RenderCopy(gRenderer, textTexture, nullptr, &renderQuad);
            SDL_DestroyTexture(textTexture);
        }
        SDL_FreeSurface(textSurface);
    } else {
        std::cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
    }
}


// Функция отображения главного меню
void showMenu(int& screenWidth, int& screenHeight, bool& fullscreen) {
    bool menuOpen = true;
    SDL_Event e;
    SDL_Color white = {0xFF, 0xFF, 0xFF, 0xFF};

    while (menuOpen) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                menuOpen = false;
                return;
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) {
                menuOpen = false;
            }
        }

        SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(gRenderer);

        // Рендер текста по центру экрана
        std::string message0 = "Movement with WASD";
        std::string message1 = "Press ENTER to start the game";  
        
        int textWidth, textHeight;
        
        // Рендер первого сообщения
        TTF_SizeText(gFont, message0.c_str(), &textWidth, &textHeight);
        int textX = (screenWidth - textWidth) / 2;
        int textY = (screenHeight - textHeight) / 2 - 20;
        renderText(message0, textX, textY, white);

        // Рендер второго сообщения
        TTF_SizeText(gFont, message1.c_str(), &textWidth, &textHeight);
        textY += 40;  // Расстояние между сообщениями
        renderText(message1, textX, textY, white);

        SDL_RenderPresent(gRenderer);
        SDL_Delay(100);
    }
}


// Функция проверки и обновления позиции врагов
void updateEnemies(std::vector<Enemy>& enemies, int playerX, int playerY, int screenWidth, int screenHeight) {
    for (auto& enemy : enemies) {
        if (enemy.active) {
            // Двигаемся к игроку
            if (enemy.x < playerX) enemy.x += ENEMY_SPEED;
            if (enemy.x > playerX) enemy.x -= ENEMY_SPEED;
            if (enemy.y < playerY) enemy.y += ENEMY_SPEED;
            if (enemy.y > playerY) enemy.y -= ENEMY_SPEED;

            // Проверка границ
            if (enemy.x < 0) enemy.x = 0;
            if (enemy.x > screenWidth - ENEMY_SIZE) enemy.x = screenWidth - ENEMY_SIZE;
            if (enemy.y < 0) enemy.y = 0;
            if (enemy.y > screenHeight - ENEMY_SIZE) enemy.y = screenHeight - ENEMY_SIZE;
        }
    }

    // Проверка на пересечение врагов
    for (size_t i = 0; i < enemies.size(); ++i) {
        for (size_t j = i + 1; j < enemies.size(); ++j) {
            if (enemies[i].active && enemies[j].active) {
                if (abs(enemies[i].x - enemies[j].x) < ENEMY_SIZE &&
                    abs(enemies[i].y - enemies[j].y) < ENEMY_SIZE) {
                    // Переместить врагов в случайные позиции, если они пересекаются
                    enemies[j].x = rand() % (screenWidth - ENEMY_SIZE);
                    enemies[j].y = rand() % (screenHeight - ENEMY_SIZE);
                }
            }
        }
    }
}

// Функция отрисовки врагов
void renderEnemies(const std::vector<Enemy>& enemies) {
    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0x00, 0xFF); // Красный цвет для врагов
    for (const auto& enemy : enemies) {
        if (enemy.active) {
            SDL_Rect fillRect = { enemy.x, enemy.y, ENEMY_SIZE, ENEMY_SIZE };
            SDL_RenderFillRect(gRenderer, &fillRect);
        }
    }
}

// Функция перемещения игрока в безопасное место
void respawnPlayer(int& playerX, int& playerY, int screenWidth, int screenHeight) {
    playerX = rand() % (screenWidth - PLAYER_SIZE);
    playerY = rand() % (screenHeight - PLAYER_SIZE);
}

// Основная функция
int main(int argc, char* args[]) {
    srand(static_cast<unsigned>(time(0))); // Инициализация генератора случайных чисел

    if (!init()) {
        std::cerr << "Failed to initialize!" << std::endl;
        return -1;
    }

    int screenWidth = WINDOW_WIDTH;
    int screenHeight = WINDOW_HEIGHT;
    bool fullscreen = false;

    gWindow = SDL_CreateWindow("Pixel Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
    if (gWindow == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        close();
        return -1;
    }

    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (gRenderer == nullptr) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        close();
        return -1;
    }

    gFont = TTF_OpenFont("arial.ttf", 28); // Убедитесь, что у вас есть шрифт Arial.ttf в текущем каталоге
    if (gFont == nullptr) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        close();
        return -1;
    }

    showMenu(screenWidth, screenHeight, fullscreen);

    bool quit = false;
    int playerX = screenWidth / 2;
    int playerY = screenHeight / 2;
    int score = 0;
    int lives = INITIAL_LIVES;
    Uint32 lastScoreUpdate = SDL_GetTicks();
    bool immune = false;
    Uint32 immuneStartTime = 0;

    std::vector<Enemy> enemies(NUM_ENEMIES);
    for (auto& enemy : enemies) {
        enemy.x = rand() % (screenWidth - ENEMY_SIZE);
        enemy.y = rand() % (screenHeight - ENEMY_SIZE);
        enemy.active = true;
    }

    Uint32 lastTick = SDL_GetTicks();
    Uint32 lastCollision = 0;

    while (!quit) {
        Uint32 currentTick = SDL_GetTicks();
        Uint32 deltaTime = currentTick - lastTick;

        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        const Uint8* currentKeyStates = SDL_GetKeyboardState(nullptr);
        if (!immune) {
            if (currentKeyStates[SDL_SCANCODE_W]) {
                playerY -= PLAYER_SPEED;
            }
            if (currentKeyStates[SDL_SCANCODE_S]) {
                playerY += PLAYER_SPEED;
            }
            if (currentKeyStates[SDL_SCANCODE_A]) {
                playerX -= PLAYER_SPEED;
            }
            if (currentKeyStates[SDL_SCANCODE_D]) {
                playerX += PLAYER_SPEED;
            }
        }

        // Проверка границ игрока
        if (playerX < 0) playerX = 0;
        if (playerX > screenWidth - PLAYER_SIZE) playerX = screenWidth - PLAYER_SIZE;
        if (playerY < 0) playerY = 0;
        if (playerY > screenHeight - PLAYER_SIZE) playerY = screenHeight - PLAYER_SIZE;

        if (!immune && (currentTick - lastCollision) >= 100) {
            for (auto& enemy : enemies) {
                if (enemy.active &&
                    playerX < enemy.x + ENEMY_SIZE && playerX + PLAYER_SIZE > enemy.x &&
                    playerY < enemy.y + ENEMY_SIZE && playerY + PLAYER_SIZE > enemy.y) {

                    lives--;
if (lives <= 0) {
    std::cout << "Game Over!" << std::endl;

    // Очистка экрана
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    SDL_RenderClear(gRenderer);

    // Центр экрана
    int centerX = screenWidth / 2;
    int centerY = screenHeight / 2;

    // Выводим "Game Over"
    renderText("Game Over!", centerX, centerY - 60, {255, 0, 0, 255});

    // Финальный счёт
    std::string scoreText = "Final Score: " + std::to_string(score);
    renderText(scoreText, centerX, centerY - 20, {255, 255, 255, 255});

    // Сообщение о рестарте
    renderText("Press ENTER to restart", centerX, centerY + 40, {200, 200, 200, 255});

    // Обновляем экран
    SDL_RenderPresent(gRenderer);

    // Ожидание нажатия ENTER или выхода
    bool waiting = true;
    SDL_Event e;
    while (waiting) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
                waiting = false;
            } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) {
                // Перезапуск игры
                score = 0;
                lives = 3;
                playerX = screenWidth / 2;  // Сброс позиции игрока, если нужно
                waiting = false;
            }
        }
    }
}

 {
                        respawnPlayer(playerX, playerY, screenWidth, screenHeight);
                        immune = true;
                        immuneStartTime = currentTick;
                        lastCollision = currentTick;
                    }
                }
            }
        }

        // Обновление врагов
        if (!immune) {
            updateEnemies(enemies, playerX, playerY, screenWidth, screenHeight);
        }

        // Обновление счета
        if (currentTick - lastScoreUpdate >= SCORE_INCREMENT_INTERVAL) {
            score++;
            lastScoreUpdate = currentTick;
        }

        // Отрисовка
        SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(gRenderer);

        if (immune) {
            Uint32 immuneTime = currentTick - immuneStartTime;
            if ((immuneTime / 500) % 2 == 0) {
                SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
            } else {
                SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
            }
        } else {
            SDL_SetRenderDrawColor(gRenderer, 0x00, 0xFF, 0x00, 0xFF); // Зелёный цвет для игрока
        }
        SDL_Rect playerRect = { playerX, playerY, PLAYER_SIZE, PLAYER_SIZE };
        SDL_RenderFillRect(gRenderer, &playerRect);

        renderEnemies(enemies);

        SDL_Color white = {0xFF, 0xFF, 0xFF, 0xFF};
        renderText("Score: " + std::to_string(score), 10, 10, white);
        renderText("Lives: " + std::to_string(lives), 10, 50, white);

        SDL_RenderPresent(gRenderer);

        // Обработка бессмертия
        if (immune && (currentTick - immuneStartTime >= IMMUNITY_DURATION)) {
            immune = false;
        }

        SDL_Delay(FRAME_DELAY);
        lastTick = currentTick;
    }

    close();
    return 0;
}

