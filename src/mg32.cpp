#include "mg32.hpp"

#include <SDL2/SDL_image.h>

#include <iostream>

using namespace mg32;

using namespace std;

Bank::Bank(SDL_Renderer* renderer, string filename,int tw, int th) : tile_width(tw), tile_height(th)
{
    SDL_Surface* surface = IMG_Load(filename.c_str());

    if (!surface) {
        cerr<<"Failed to load image:"<<filename<<endl;
    }

    data = SDL_CreateTextureFromSurface(renderer, surface);

    width = surface->w;
    height = surface->h;

    SDL_FreeSurface(surface);
}

Bank::~Bank()
{

}
