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
const float RENDERER_SCALE = 5.0;
const double WINDOW_CENTER_X = WINDOW_WIDTH/2 /RENDERER_SCALE;
const double WINDOW_CENTER_Y = WINDOW_HEIGHT/2 /RENDERER_SCALE;

// Convergence, Separation, Alignment, Cohesion

double f(vector<double> position, vector<double> f_center) {
    vector<double> x_shifted = {position[0] - f_center[0], position[1] - f_center[1]};
    return x_shifted[0]*x_shifted[0] + x_shifted[1]*x_shifted[1];
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

// Helper function for infinite space and wrap around positions
pair<vector<double>, vector<double>> infinite_space(vector<double> position, vector<double> velocity) {
    if (position[0] > WINDOW_WIDTH /RENDERER_SCALE)
    {
        position[0] = WINDOW_WIDTH /RENDERER_SCALE;
        velocity[0] = 0;
    } else if (position[0] < 0)
    {
        position[0] = 0;
        velocity[0] = 0;
    }
    if (position[1] > WINDOW_HEIGHT /RENDERER_SCALE)
    {
        position[1] = WINDOW_HEIGHT /RENDERER_SCALE;
        velocity[1] = 0;
    } else if (position[1] < 0)
    {
        position[1] = 0;
        velocity[1] = 0;
    }

    return make_pair(position, velocity);
}

double calculate_distance(const vector<double>& p1, const vector<double>& p2) {
    double distance = sqrt(pow(p1[0] - p2[0], 2) + pow(p1[1] - p2[1], 2));
    return distance;
}

// For now, neighbor function is just all neighbors within a distance
vector<vector<double>> get_social_neighbors(const vector<double>& reference, const vector<vector<double>>& positions, int neighborhood_distance) {
    vector<pair<double, vector<double>>> distances;

    for (const auto& pos : positions) {
        double distance = calculate_distance(pos, reference);
        distances.push_back({distance, pos});
    }

    // Sorting based on distance
    sort(distances.begin(), distances.end());

    vector<vector<double>> neighbors;
    for (const auto& dist_pos_pair : distances) {
        double distance = dist_pos_pair.first;
        if (distance <= neighborhood_distance && distance > 0) { // don't inlcude yourself
            neighbors.push_back(dist_pos_pair.second);
        }
    }

    return neighbors;
}

vector<double> get_best_position(vector<vector<double>>& positions, vector<double> f_center) {
    vector<double> best_position = positions[0];
    double current_best_value = f(best_position, f_center);

    for (const auto& position : positions) {
        double position_value = f(position, f_center);

        if (position_value < current_best_value) {
            // Update the current best position and value
            best_position = position;
            current_best_value = position_value;
        }
    }

    return best_position;
}

// Helper function to adjust particle positions and velocities to avoid collisions
vector<double> avoid_collisions(vector<double> position, vector<double> velocity, int i, vector<vector<double>>& particle_positions) {
    for (size_t j = 0; j < particle_positions.size(); ++j) {
        if (j != i) {
            vector<double> direction_ij = {particle_positions[j][0] - position[0],
                                            particle_positions[j][1] - position[1]};
            double norm = calculate_distance({0.0, 0.0}, direction_ij);
            direction_ij[0] /= norm;
            direction_ij[1] /= norm;

            velocity[0] -= (10/(norm*norm)) * direction_ij[0];
            velocity[1] -= (10/(norm*norm)) * direction_ij[1];
        }
    }
    return velocity;
}

vector<double> scatter(vector<double> position, vector<double> velocity, int i, vector<vector<double>>& particle_positions) {
    for (size_t j = 0; j < particle_positions.size(); ++j) {
        if (j != i) {
            vector<double> direction_ij = {particle_positions[j][0] - position[0],
                                            particle_positions[j][1] - position[1]};
            double norm = calculate_distance({0.0, 0.0}, direction_ij);
            direction_ij[0] /= norm;
            direction_ij[1] /= norm;

            velocity[0] -= (100) * direction_ij[0];
            velocity[1] -= (100) * direction_ij[1];
        }
    }
    return velocity;
}

// Run PSO simulation
pair<vector<vector<vector<double>>>, vector<vector<double>>> run_pso(int num_particles, int max_iterations, int neighborhood_distance, double c1, double c2, double w, mt19937 gen) {
    double timestep = 0.001;

    // Initialize the global best-known position and value
    ParticleState particles = initialize_particles(num_particles, gen);
    vector<vector<double>> particle_positions = get<0>(particles);
    vector<vector<double>> particle_velocities = get<1>(particles);
    vector<vector<double>> particle_best_positions = get<2>(particles);
    vector<vector<vector<double>>> particles_history;

    vector<double> f_center = {0.0, 0.0};
    vector<vector<double>> f_center_history;

    // Random distribution for random weighting of cognitive vs social vs inertial
    uniform_real_distribution<double> U(0.0, 1.0);

    // PSO optimization loop
    for (int iteration = 0; iteration < max_iterations; ++iteration) {
        if (iteration % 200 == 0) {
            f_center = {U(gen) *WINDOW_WIDTH/RENDERER_SCALE, U(gen) *WINDOW_HEIGHT/RENDERER_SCALE};
        }

        for (int i = 0; i < num_particles; ++i) {
            vector<double>& position = particle_positions[i];
            vector<double>& velocity = particle_velocities[i];
            vector<double>& personal_best_position = particle_best_positions[i];

            // 0. Find social influence group and social_best_position; update velocity
            vector<vector<double>> social_neighbors_positions = get_social_neighbors(position, particle_positions, neighborhood_distance);
            if (social_neighbors_positions.empty()) {
                // if no neighbors, don't just decrease velocity
                double r1 = U(gen);
                double r2 = U(gen);
                velocity[0] = velocity[0] + c1 * r1 * (personal_best_position[0] - position[0]);
                velocity[1] = velocity[1] + c1 * r1 * (personal_best_position[1] - position[1]);
            } else {
                vector<double> social_best_position = get_best_position(social_neighbors_positions, f_center);

                double r1 = U(gen);
                double r2 = U(gen);

                velocity[0] = w * velocity[0] + c1 * r1 * (personal_best_position[0] - position[0]) + c2 * r2 * (social_best_position[0] - position[0]);
                velocity[1] = w * velocity[1] + c1 * r1 * (personal_best_position[1] - position[1]) + c2 * r2 * (social_best_position[1] - position[1]);
            }
            
            // 0.5 Consider collisions
            if (iteration == 1000 || iteration == 500 || iteration == 1500) {
                velocity = scatter(position, velocity, i, particle_positions);
            } else {
                velocity = avoid_collisions(position, velocity, i, particle_positions);
            }

            // 1. Update position
            position[0] = position[0] + velocity[0]*timestep;
            position[1] = position[1] + velocity[1]*timestep;
            pair<vector<double>, vector<double>> infinite_correction = infinite_space(position, velocity);
            position = infinite_correction.first;
            velocity = infinite_correction.second;

            // 2. Evaluate the objective function at the new position
            double value = f(position, f_center);

            // 3. Update personal best if needed
            if (value < f(personal_best_position, f_center)) {
                personal_best_position = position;
            }
        }

        particles_history.push_back(particle_positions);
        f_center_history.push_back(f_center);
    }

    return make_pair(particles_history, f_center_history);
}

void draw_particles(SDL_Renderer* renderer, int time, vector<vector<vector<double>>> particles_history, int num_particles) {
    for (int i = max(0, time - 2000); i < time; ++i) {
        // the higher the i, the more recent

        vector<vector<double>> particles_to_draw = particles_history[i];
        int color = 255* (i - max(0, time-2000))/(time - max(0, time-2000));

        for (int particle_index = 0; particle_index < num_particles; ++particle_index) {
            double x_to_draw = particles_to_draw[particle_index][0];
            double y_to_draw = particles_to_draw[particle_index][1];
            if (particle_index % 3 == 0) {
                SDL_SetRenderDrawColor(renderer, color, color, 0, SDL_ALPHA_OPAQUE);
            } else if (particle_index % 3 == 1){
                SDL_SetRenderDrawColor(renderer, color, 0, 0, SDL_ALPHA_OPAQUE);
            } else {
                SDL_SetRenderDrawColor(renderer, color, 0, color, SDL_ALPHA_OPAQUE);
            }
            SDL_RenderDrawPoint(renderer, x_to_draw, y_to_draw);
        }
    }
}

void draw_f_center(SDL_Renderer* renderer, vector<double> f_center) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawPoint(renderer, f_center[0], f_center[1]);
    SDL_RenderDrawPoint(renderer, f_center[0], f_center[1]-1);
    SDL_RenderDrawPoint(renderer, f_center[0]+1, f_center[1]);
    SDL_RenderDrawPoint(renderer, f_center[0], f_center[1]+1);
    SDL_RenderDrawPoint(renderer, f_center[0]-1, f_center[1]);
}

int main() {
    // INTITIALIZATION =============================================================
    // Define the PSO parameters
    const int num_particles = 30;
    const int max_iterations = 2000;
    const int neighborhood_distance = 20;
    const double c1 = 1.5;  // Cognitive parameter
    const double c2 = 1.5;  // Social parameter
    const double w = 0.9;   // Inertia weight

    // Initialize the random seed
    mt19937 gen(314); // supposedly this seeds the rand num generator

    // RUNNING SIMULATION =============================================================
    pair<vector<vector<vector<double>>>, vector<vector<double>>> pso_history = run_pso(num_particles, max_iterations, neighborhood_distance, c1, c2, w, gen);
    vector<vector<vector<double>>> particles_history = pso_history.first;
    vector<vector<double>> f_center_history = pso_history.second;

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

        // draw
        draw_particles(renderer, time, particles_history, num_particles);
        draw_f_center(renderer, f_center_history[time]);

        // Render
        SDL_RenderPresent(renderer);
        SDL_Delay(1);
    }

    return 0;
}
