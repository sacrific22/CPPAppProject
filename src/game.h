#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#pragma comment(lib, "shell32.lib")


// Запуск игры
// Возвращает true, если нужно вернуться в меню
// false, если закрыли игру
bool runGame(SDL_Renderer* renderer, TTF_Font* font, SDL_Texture* skinTexture);
