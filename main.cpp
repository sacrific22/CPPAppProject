#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "game.h"

// irrKlang
#include <irrKlang.h>
using namespace irrklang;

ISoundEngine* engine = nullptr; // глобальный движок звука

enum class GameState {
    MENU,
    SETTINGS,
    GAME,
    EXIT
};

// Функции отрисовки текста и кнопок
SDL_Texture* renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
    SDL_Color color, SDL_Rect& outRect) {
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surf) {
        std::cerr << "TTF_Render error: " << TTF_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    outRect = { 0, 0, surf->w, surf->h };
    SDL_FreeSurface(surf);
    return tex;
}

void drawButton(SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
    int x, int y, int w, int h, bool selected) {
    SDL_Color bgColor = selected ? SDL_Color{ 50,100,200,255 } : SDL_Color{ 70,70,70,200 };
    SDL_Color borderColor = SDL_Color{ 255,255,255,255 };
    SDL_Color textColor = selected ? SDL_Color{ 255,255,255,255 } : SDL_Color{ 200,200,200,255 };

    SDL_Rect rect{ x,y,w,h };
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderDrawRect(renderer, &rect);

    SDL_Rect textRect;
    SDL_Texture* tex = renderText(renderer, font, text, textColor, textRect);
    textRect.x = x + (w - textRect.w) / 2;
    textRect.y = y + (h - textRect.h) / 2;
    SDL_RenderCopy(renderer, tex, nullptr, &textRect);
    SDL_DestroyTexture(tex);
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL error: " << SDL_GetError() << std::endl;
        return -1;
    }
    if (TTF_Init() < 0) {
        std::cerr << "TTF_Init error: " << TTF_GetError() << std::endl;
        return -1;
    }
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
        std::cerr << "IMG_Init error: " << IMG_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("SDL2 Menu",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font* font = TTF_OpenFont("assets/Handwritten Script.otf", 36);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return -1;
    }

    // irrKlang init
    engine = createIrrKlangDevice();
    if (!engine) {
        std::cerr << "Could not startup irrKlang engine\n";
        return -1;
    }

    // Запускаем музыку (будет играть и в меню, и в игре)
    engine->play2D("assets/bgMusic.wav", true); // true = зациклено

    // Фон меню
    SDL_Texture* bgTexture = IMG_LoadTexture(renderer, "assets/bg.png");

    GameState state = GameState::MENU;
    std::vector<std::string> menuItems = { "Play", "Settings", "Exit" };
    int selectedIndex = 0;
    float brightness = 1.0f;

    // Скины
    std::vector<std::string> skinFiles = {
        "assets/default.png",
        "assets/skin1.png",
        "assets/skin2.png",
        "assets/skin3.png"
    };
    std::vector<std::string> skinNames = {
        "Peasant Boris",
        "Mage Adnan",
        "Warrior Ulugbek",
        "Cleric GPT"
    };
    std::vector<SDL_Texture*> skinTextures;
    for (const auto& s : skinFiles) {
        skinTextures.push_back(IMG_LoadTexture(renderer, s.c_str()));
    }
    int currentSkin = 0;

    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            else if (e.type == SDL_KEYDOWN) {
                SDL_Scancode sc = e.key.keysym.scancode;

                switch (state) {
                case GameState::MENU:
                    if (sc == SDL_SCANCODE_UP)
                        selectedIndex = (selectedIndex + (int)menuItems.size() - 1) % (int)menuItems.size();
                    else if (sc == SDL_SCANCODE_DOWN)
                        selectedIndex = (selectedIndex + 1) % (int)menuItems.size();
                    else if (sc == SDL_SCANCODE_RETURN) {
                        if (selectedIndex == 0) state = GameState::GAME;
                        else if (selectedIndex == 1) state = GameState::SETTINGS;
                        else if (selectedIndex == 2) state = GameState::EXIT;
                    }
                    break;

                case GameState::SETTINGS:
                    if (sc == SDL_SCANCODE_LEFT) brightness = std::clamp(brightness - 0.1f, 0.f, 1.f);
                    else if (sc == SDL_SCANCODE_RIGHT) brightness = std::clamp(brightness + 0.1f, 0.f, 1.f);
                    else if (sc == SDL_SCANCODE_A) currentSkin = (currentSkin + (int)skinFiles.size() - 1) % (int)skinFiles.size();
                    else if (sc == SDL_SCANCODE_D) currentSkin = (currentSkin + 1) % (int)skinFiles.size();
                    else if (sc == SDL_SCANCODE_ESCAPE || sc == SDL_SCANCODE_RETURN) state = GameState::MENU;
                    break;

                case GameState::GAME:
                    if (sc == SDL_SCANCODE_ESCAPE) state = GameState::MENU;
                    break;

                case GameState::EXIT:
                    running = false;
                    break;
                }
            }
        }

        if (state == GameState::EXIT) running = false;

        if (bgTexture) SDL_RenderCopy(renderer, bgTexture, nullptr, nullptr);
        else {
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            SDL_RenderClear(renderer);
        }

        if (state == GameState::MENU) {
            int btnW = 250, btnH = 70, startY = 200;
            for (int i = 0; i < (int)menuItems.size(); i++) {
                drawButton(renderer, font, menuItems[i],
                    275, startY + i * 100, btnW, btnH,
                    (i == selectedIndex));
            }
        }
        else if (state == GameState::SETTINGS) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 180);
            SDL_Rect blurRect{ 100, 50, 600, 500 };
            SDL_RenderFillRect(renderer, &blurRect);

            SDL_Rect r;
            SDL_Texture* title = renderText(renderer, font, "Settings", { 255,255,255,255 }, r);
            r.x = 300; r.y = 60; SDL_RenderCopy(renderer, title, nullptr, &r); SDL_DestroyTexture(title);

            SDL_Texture* bright = renderText(renderer, font, "Brightness:", { 255,255,255,255 }, r);
            r.x = 200; r.y = 150; SDL_RenderCopy(renderer, bright, nullptr, &r); SDL_DestroyTexture(bright);

            SDL_Rect bar{ 400, 160, static_cast<int>(200 * brightness), 20 };
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderFillRect(renderer, &bar);

            SDL_Texture* skin = renderText(renderer, font, "Skin: " + skinNames[currentSkin], { 255,255,255,255 }, r);
            r.x = 200; r.y = 250; SDL_RenderCopy(renderer, skin, nullptr, &r); SDL_DestroyTexture(skin);

            if (skinTextures[currentSkin]) {
                int texW, texH;
                if (SDL_QueryTexture(skinTextures[currentSkin], nullptr, nullptr, &texW, &texH) == 0) {
                    float scale = 4.0f;
                    SDL_Rect previewRect{ 500, 220, (int)(texW * scale), (int)(texH * scale) };
                    SDL_RenderCopy(renderer, skinTextures[currentSkin], nullptr, &previewRect);
                }
            }
        }
        else if (state == GameState::GAME) {
            bool backToMenu = runGame(renderer, font, skinTextures[currentSkin]);
            if (backToMenu) {
                state = GameState::MENU;
            }
            else {
                running = false;
            }
        }

        SDL_RenderPresent(renderer);
    }

    for (auto tex : skinTextures) if (tex) SDL_DestroyTexture(tex);
    if (bgTexture) SDL_DestroyTexture(bgTexture);

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    engine->drop(); // закрыть irrKlang
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}
