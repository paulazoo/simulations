#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <random>
#include <tuple>

using namespace std;

const int WINDOW_HEIGHT = 500;
const int WINDOW_WIDTH = 500;
const float RENDERER_SCALE = 2.0;
const double WINDOW_CENTER_X = WINDOW_WIDTH/2 /RENDERER_SCALE;
const double WINDOW_CENTER_Y = WINDOW_HEIGHT/2 /RENDERER_SCALE;

// Objective function
// // Rosenbrock Function (Banana Function)
// double f(vector<double> position) {
//     vector<double> x = {position[0] -WINDOW_CENTER_X, position[1] -WINDOW_CENTER_Y};

//     double sum = 0.0;
//     for (size_t i = 0; i < x.size() - 1; ++i) {
//         double term1 = pow(x[i + 1] - pow(x[i], 2), 2);
//         double term2 = pow(1 - x[i], 2);
//         sum += 100.0 * term1 + term2;
//     }
//     return sum;
// }

// // Multimodal Ackley Function
// double f(vector<double> position) {
//     vector<double> x = {position[0] -WINDOW_CENTER_X, position[1] -WINDOW_CENTER_Y};

//     const double a = 20.0;
//     const double b = 0.2;
//     const double c = 2 * M_PI;

//     size_t dimensions = x.size();

//     double sum1 = 0.0;
//     double sum2 = 0.0;

//     for (size_t i = 0; i < dimensions; ++i) {
//         sum1 += x[i] * x[i];
//         sum2 += cos(c * x[i]);
//     }

//     double term1 = -a * exp(-b * sqrt(sum1 / dimensions));
//     double term2 = -exp(sum2 / dimensions);

//     return term1 + term2 + a + exp(1.0);
// }

// Sin and Cos Waves
double f(vector<double> position) {
    vector<double> x = {position[0] -WINDOW_CENTER_X, position[1] -WINDOW_CENTER_Y};
    return sin(x[0]) + cos(x[1]);
}


using ParticleState = tuple<vector<vector<double>>, vector<vector<double>>, vector<vector<double>>>;

// Initialize the particles
ParticleState initialize_particles(int num_particles, mt19937 gen) {
    uniform_real_distribution<double> initial_position_distribution(0.0, WINDOW_WIDTH/RENDERER_SCALE);
    uniform_real_distribution<double> initial_velocity_distribution(-1.0, 1.0);

    vector<vector<double>> particle_positions;
    vector<vector<double>> particle_velocities;
    vector<vector<double>> particle_best_positions;

    for (int i = 0; i < num_particles; ++i) {
        vector<double> position = {initial_position_distribution(gen), initial_position_distribution(gen)};  // Random initial position in the range [-10, 10]
        particle_positions.push_back(position);

        vector<double> velocity = {initial_velocity_distribution(gen), initial_velocity_distribution(gen)};  // Random initial velocity in the range [-1, 1]
        particle_velocities.push_back(velocity);

        particle_best_positions.push_back(position);
    }

    return make_tuple(particle_positions, particle_velocities, particle_best_positions);
}

// Run PSO simulation
vector<vector<vector<double>>> run_pso(int num_particles, int max_iterations, double c1, double c2, double w, mt19937 gen) {
    double timestep = 0.01;

    // Initialize the global best-known position and value
    ParticleState particles = initialize_particles(num_particles, gen);
    vector<vector<double>> particle_positions = get<0>(particles);
    vector<vector<double>> particle_velocities = get<1>(particles);
    vector<vector<double>> particle_best_positions = get<2>(particles);

    vector<double> global_best_position = particle_best_positions[0];
    double global_best_value = f(global_best_position);
    vector<vector<vector<double>>> particles_history;

    // Random distribution for random weighting of cognitive vs social vs inertial
    uniform_real_distribution<double> weighting_distribution(0.0, 1.0);

    // PSO optimization loop
    for (int iteration = 0; iteration < max_iterations; ++iteration) {
        for (int i = 0; i < num_particles; ++i) {
            vector<double>& position = particle_positions[i];
            vector<double>& velocity = particle_velocities[i];
            vector<double>& personal_best_position = particle_best_positions[i];

            // 1. Update velocity and position
            double r1 = weighting_distribution(gen);
            double r2 = weighting_distribution(gen);

            velocity[0] = w * velocity[0] + c1 * r1 * (personal_best_position[0] - position[0]) + c2 * r2 * (global_best_position[0] - position[0]);
            velocity[1] = w * velocity[1] + c1 * r1 * (personal_best_position[1] - position[1]) + c2 * r2 * (global_best_position[1] - position[1]);

            position[0] = position[0] + velocity[0]*timestep;
            position[1] = position[1] + velocity[1]*timestep;
            // Infinite space
            if (position[0] > WINDOW_WIDTH /RENDERER_SCALE)
            {
                position[0] = 0;
            } else if (position[0] < 0)
            {
                position[0] = WINDOW_WIDTH /RENDERER_SCALE;
            }

            if (position[1] > WINDOW_HEIGHT /RENDERER_SCALE)
            {
                position[1] = 0;
            } else if (position[1] < 0)
            {
                position[1] = WINDOW_HEIGHT /RENDERER_SCALE;
            }

            // 2. Evaluate the objective function at the new position
            double value = f(position);

            // 3. Update personal best if needed
            if (value < f(personal_best_position)) {
                personal_best_position = position;
            }

            // 4. Update global best if needed
            if (value < global_best_value) {
                global_best_position = position;
                global_best_value = value;
            }
        }

        particles_history.push_back(particle_positions);
    }

    cout << "Optimal Solution: x = " << global_best_position[0] << ", y = " << global_best_position[1] << ", f(x) = " << global_best_value << endl;

    return particles_history;
}

void draw_particles(SDL_Renderer* renderer, int time, vector<vector<vector<double>>> particles_history, int num_particles) {
    for (int i = max(0, time - 500); i < time; ++i) {
        // the higher the i, the more recent

        vector<vector<double>> particles_to_draw = particles_history[i];
        int color = i*255 / time;

        for (int particle_index =0; particle_index < num_particles; ++particle_index) {
            double x_to_draw = particles_to_draw[particle_index][0];
            double y_to_draw = particles_to_draw[particle_index][1];
            SDL_SetRenderDrawColor(renderer, color, color, color, SDL_ALPHA_OPAQUE);
            SDL_RenderDrawPoint(renderer, x_to_draw, y_to_draw);
        }
    }
}

int main() {
    // INTITIALIZATION =============================================================
    // Define the PSO parameters
    const int num_particles = 100;
    const int max_iterations = 1000;
    const double c1 = 1.5;  // Cognitive parameter
    const double c2 = 1.5;  // Social parameter
    const double w = 0.9;   // Inertia weight

    // Initialize the random seed
    mt19937 gen(314); // supposedly this seeds the rand num generator

    // RUNNING SIMULATION =============================================================
    vector<vector<vector<double>>> particles_history = run_pso(num_particles, max_iterations, c1, c2, w, gen);

    // DISPLAYING SIMULATION RESULTS ===================================================
    // Setup
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window;
    SDL_Renderer *renderer;
    window = SDL_CreateWindow("SDL Window",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH,
            WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetScale(renderer, RENDERER_SCALE, RENDERER_SCALE);

    // A basic main loop to prevent blocking
    bool is_running = true;
    SDL_Event event;
    for (int time = 0; time < max_iterations; ++time) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                is_running = false;
            }
        }
        if (is_running == false) {
            break;
        }

        // RENDERING ============================================================
        // Blank black canvas
        SDL_SetRenderDrawColor(renderer,0,0,0,SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        // draw particles
        draw_particles(renderer, time, particles_history, num_particles);

        // Render
        SDL_RenderPresent(renderer);
        SDL_Delay(50);
    }

    return 0;
}
