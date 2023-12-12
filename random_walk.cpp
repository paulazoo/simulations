#include <SDL2/SDL.h>
#include <random>
#include <iostream>
#include <vector>

using namespace std;

const int WINDOW_HEIGHT = 500;
const int WINDOW_WIDTH = 500;
const float RENDERER_SCALE = 5.0;

int main()
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window;
    SDL_Renderer *renderer;
    window = SDL_CreateWindow("SDL Window",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH,
            WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetScale(renderer, RENDERER_SCALE, RENDERER_SCALE);

    // Setup
    int x = WINDOW_WIDTH/2 /RENDERER_SCALE;
    int y = WINDOW_HEIGHT/2 /RENDERER_SCALE;
    vector<int> x_history;
    vector<int> y_history;
    random_device random_seed;
    uniform_int_distribution<uint8_t> U(0,4);
    cout << "setup done." << endl;

    // A basic main loop to prevent blocking
    bool is_running = true;
    SDL_Event event;
    while (is_running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                is_running = false;
            }
        }

        uint8_t direction = U(random_seed);
        switch(direction)
        {
            case 0:
                x += 1;
                break;
            case 1:
                x -= 1;
                break;
            case 2:
                y += 1;
                break;
            case 3:
                y -= 1;
                break;
        }
        // Infinite space
        if (x > WINDOW_WIDTH /RENDERER_SCALE)
        {
            x = 0;
        } else if (x < 0)
        {
            x = WINDOW_WIDTH /RENDERER_SCALE;
        }

        if (y > WINDOW_HEIGHT /RENDERER_SCALE)
        {
            y = 0;
        } else if (y < 0)
        {
            y = WINDOW_HEIGHT /RENDERER_SCALE;
        }

        x_history.push_back(x);
        y_history.push_back(y);

        // Blank black canvas
        SDL_SetRenderDrawColor(renderer,0,0,0,SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        // Draw
        int times = x_history.size();
        for (int i = max(0, times - 5000); i < times; ++i)
        {
            // the higher the i, the more recent
            int x_to_draw = x_history[i];
            int y_to_draw = y_history[i];
            int color = i*255 / times;
            SDL_SetRenderDrawColor(renderer, color, color, color, SDL_ALPHA_OPAQUE);
            SDL_RenderDrawPoint(renderer, x_to_draw, y_to_draw);
        }

        // Render
        SDL_RenderPresent(renderer);
        SDL_Delay(5);
    }
     
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
}