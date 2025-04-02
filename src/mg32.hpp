#include <SDL2/SDL.h>

#include <string>

namespace mg32
{
    class Bank
    {
        public:

        SDL_Texture* data;
        int tile_width;
        int tile_height;

        Bank(SDL_Renderer* renderer, std::string filename,int tw, int th);
        virtual ~Bank();
    };


}
