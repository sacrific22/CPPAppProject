#include "game.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

struct Fruit {
    SDL_Rect rect;
    int type;
    int speed;
};

bool runGame(SDL_Renderer* renderer, TTF_Font* font, SDL_Texture* skinTexture,float brightness) {
    // Загружаем фон игры
    SDL_Texture* bgTexture = IMG_LoadTexture(renderer, "assets/gamebg.png");

    // Загружаем фрукты
    SDL_Texture* fruits[5];
    fruits[0] = IMG_LoadTexture(renderer, "assets/apple.png");
    fruits[1] = IMG_LoadTexture(renderer, "assets/banana.png");
    fruits[2] = IMG_LoadTexture(renderer, "assets/cherry.png");
    fruits[3] = IMG_LoadTexture(renderer, "assets/strawberry.png");
    fruits[4] = IMG_LoadTexture(renderer, "assets/orange.png");

    int fruitScores[5] = { 10, 20, 15, 25, 30 };

    SDL_Rect player = { 400 - 32, 600 - 80, 64, 64 };

    bool running = true;
    SDL_Event e;
    std::vector<Fruit> fruitList;
    int score = 0;
    Uint32 lastSpawn = SDL_GetTicks();

    srand((unsigned)time(0));

    bool winScreen = false;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                return false; // закрыть всё
            }
            else if (e.type == SDL_KEYDOWN) {
                if (!winScreen) {
                    if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                        return true; // в меню
                    }
                }
                else {
                    if (e.key.keysym.scancode == SDL_SCANCODE_RETURN) {
                        return true; // Return to menu
                    }
                }
            }
        }

        if (!winScreen) {
            // Управление с ускорением Shift
            const Uint8* state = SDL_GetKeyboardState(NULL);
            int speed = 5;
            if (state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT]) speed = 10;

            if (state[SDL_SCANCODE_LEFT] && player.x > 0) player.x -= speed;
            if (state[SDL_SCANCODE_RIGHT] && player.x < 800 - player.w) player.x += speed;

            // Спавн фруктов
            if (SDL_GetTicks() - lastSpawn > 1500) {
                Fruit f;
                f.type = rand() % 5;
                f.rect = { rand() % (800 - 48), 0, 48, 48 };
                f.speed = 3 + rand() % 4;
                fruitList.push_back(f);
                lastSpawn = SDL_GetTicks();
            }

            // Обновление фруктов
            for (size_t i = 0; i < fruitList.size();) {
                fruitList[i].rect.y += fruitList[i].speed;

                if (SDL_HasIntersection(&fruitList[i].rect, &player)) {
                    score += fruitScores[fruitList[i].type];
                    fruitList.erase(fruitList.begin() + i);
                }
                else if (fruitList[i].rect.y > 600) {
                    score -= 10;
                    fruitList.erase(fruitList.begin() + i);
                }
                else {
                    ++i;
                }
            }
        }

        // Рендер
        if (bgTexture) SDL_RenderCopy(renderer, bgTexture, nullptr, nullptr);
        else {
            SDL_SetRenderDrawColor(renderer, 50, 150, 200, 255);
            SDL_RenderClear(renderer);
        }

        if (!winScreen) {
            // Игрок (выбранный скин)
            SDL_RenderCopy(renderer, skinTexture, NULL, &player);

            // Фрукты
            for (auto& f : fruitList) {
                SDL_RenderCopy(renderer, fruits[f.type], NULL, &f.rect);
            }

            // Очки
            SDL_Color white = { 255,255,255,255 };
            std::string txt = "Score: " + std::to_string(score);
            SDL_Surface* surf = TTF_RenderUTF8_Solid(font, txt.c_str(), white);
            SDL_Texture* msg = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_Rect r = { 10, 10, surf->w, surf->h };
            SDL_RenderCopy(renderer, msg, NULL, &r);
            SDL_FreeSurface(surf);
            SDL_DestroyTexture(msg);

            // Проверка победы
            if (score >= 500) {
                winScreen = true;
            }
        }
        else {
            // Эффект затемнения
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
            SDL_Rect blurRect{ 100, 100, 600, 400 };
            SDL_RenderFillRect(renderer, &blurRect);

            // YOU WIN!
            SDL_Color yellow = { 255,255,0,255 };
            SDL_Surface* surf = TTF_RenderUTF8_Solid(font, "YOU WIN!", yellow);
            SDL_Texture* txt = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_Rect r{ 300, 150, surf->w, surf->h };
            SDL_RenderCopy(renderer, txt, NULL, &r);
            SDL_FreeSurface(surf);
            SDL_DestroyTexture(txt);

            // Return to menu
            SDL_Color white = { 255,255,255,255 };
            surf = TTF_RenderUTF8_Solid(font, "Return to menu (Enter)", white);
            txt = SDL_CreateTextureFromSurface(renderer, surf);
            r = { 250, 300, surf->w, surf->h };
            SDL_RenderCopy(renderer, txt, NULL, &r);
            SDL_FreeSurface(surf);
            SDL_DestroyTexture(txt);
        }
        // Brightness effect overlay ( darkening all scenes )
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        Uint8 overlay = static_cast<Uint8>((1.0f - brightness) * 255);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, overlay);
        SDL_Rect screenRect{ 0, 0, 800, 600 };
        SDL_RenderFillRect(renderer, &screenRect);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    if (bgTexture) SDL_DestroyTexture(bgTexture);
    for (int i = 0; i < 5; i++) if (fruits[i]) SDL_DestroyTexture(fruits[i]);

    return true;
}
