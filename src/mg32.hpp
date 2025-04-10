#include <SDL2/SDL.h>

#include <string>

namespace mg32
{
    class Bank
    {
        public:

        SDL_Texture* data;
        int width;
        int height;
        int tile_width;
        int tile_height;

        Bank(SDL_Renderer* renderer, std::string filename,int tw, int th);
        virtual ~Bank();
    };

    enum class Command
    {
        Blit,
        BlitEx
    };

    class DrawCommand
    {
        public:

        Command command;

        int z;
        int flip;
        double angle;
        SDL_Point pivot;

        SDL_Texture* texture;
        SDL_Rect src;
        SDL_Rect dst;

        DrawCommand* left;
        DrawCommand* right;
    };
}
