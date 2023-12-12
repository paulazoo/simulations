#include <SDL2/SDL.h>
#include <random>
#include <iostream>
#include <vector>

using namespace std;

int main()
{
    const int windowHeight = 500;
    const int windowWidth = 500;
    const float rendererScale = 5.0;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window;
    SDL_Renderer *renderer;
    window = SDL_CreateWindow("SDL Window",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth,
            windowHeight, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetScale(renderer, rendererScale, rendererScale);

    // Setup
    int x = windowWidth/2 /rendererScale;
    int y = windowHeight/2 /rendererScale;
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
        if (x > windowWidth /rendererScale)
        {
            x = 0;
        } else if (x < 0)
        {
            x = windowWidth /rendererScale;
        }

        if (y > windowHeight /rendererScale)
        {
            y = 0;
        } else if (y < 0)
        {
            y = windowHeight /rendererScale;
        }

        x_history.push_back(x);
        y_history.push_back(y);

        // Blank canvas
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