#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <cstdlib> // For random number generation
#include <ctime>   // For seeding the random number generator
#include <algorithm>
#include <cmath>
#include <random>
#include <chrono>
#include <iterator>

using namespace std;

const int WINDOW_HEIGHT = 600;
const int WINDOW_WIDTH = 800;
const float RENDERER_SCALE = 5.0;
const double WINDOW_CENTER_X = WINDOW_WIDTH/2 /RENDERER_SCALE;
const double WINDOW_CENTER_Y = WINDOW_HEIGHT/2 /RENDERER_SCALE;

// Helper function for choosing a rxn using a discrete distribution
// using the values in `possible_rxns` and corresponding relative probabilities given by `rxn_propensities`
int choose_a_rxn(vector<int> possible_rxns, vector<double> rxn_propensities, double total_propensity, mt19937 gen) {

    // Validate sizes and initialize distribution
    if (possible_rxns.size() != rxn_propensities.size() || possible_rxns.empty()) {
        cerr << "Invalid input sizes or empty vectors." << endl;
        return 1; // Return an error code
    }

    // Normalize propensities to probabilities
    vector<double> rxn_probabilities;
    transform(rxn_propensities.begin(), rxn_propensities.end(), back_inserter(rxn_probabilities),
                   [total_propensity](double prop) { return prop / total_propensity; });

    // Create a discrete distribution based on rxn_probabilities
    discrete_distribution<> rxns_distribution(rxn_probabilities.begin(), rxn_probabilities.end());

    // Generate a random index
    int random_rxn_index = rxns_distribution(gen);

    return random_rxn_index;
}

// Helper function to generate a random integer in the specified range [min, max]
int get_random_int(int min, int max, mt19937 gen) {
    uniform_int_distribution<int> random_int_distribution(min, max);
    int random_int = random_int_distribution(gen);
    return random_int;
}

// nx and ny are cells on the x and y sides of the grid
pair<vector<vector<int>>, vector<int>> get_grid(int nx, int ny, mt19937 gen) {
    vector<vector<int>> adjs;
    for (int i = 0; i < nx; ++i) {
        for (int j = 0; j < ny; ++j) {
            vector<int> adj;
            // adjs is a list of the 6 neighbors for each cell
            for (auto [x, y] : vector<pair<int, int>>{{i - 1, j - 1}, {i, j - 1}, {i - 1, j},
                                                               {i + 1, j}, {i, j + 1}, {i + 1, j + 1}}) {
                // index of neighbor if neighbor exists
                // None if no neighbors bc fictitious cells give 0 for D_bar, don'time affect anything
                adj.push_back((x >= 0 && x < nx && y >= 0 && y < ny) ? (x * ny + y) : -1);
            }
            adjs.push_back(adj);
        }
    }

    vector<int> initial_compartments; // initial_compartments is all the compartments for all cells: N D Z
    for (int i = 0; i < nx * ny; ++i) {
        int N = get_random_int(19, 20, gen);
        int Z = 20;
        int D = 0;
        initial_compartments.push_back(N);
        initial_compartments.push_back(D);
        initial_compartments.push_back(Z);
    }

    return {adjs, initial_compartments};
}


double f(double x) {
    return (x * x) / (0.01 + (x * x));
}

// N: (cell_i*3)
// D: (cell_i*3)+1
// Z: (cell_i*3)+2
double rxn0_propensity(int cell_i, vector<int> compartments, vector<vector<int>> adjs) {
    // zero condition: if N+1 > Z
    double Z = compartments[(cell_i * 3) + 2];
    double N = compartments[cell_i * 3];
    
    if (N + 1 > Z) {
        return 0.0;
    } else {
        double D_bar = 0.0;
        for (int neighbor_i : adjs[cell_i]) {
            if (neighbor_i != -1) {
                D_bar += compartments[(neighbor_i * 3) + 1];
            }
        }
        return Z * f(D_bar / Z);
    }
}

double rxn1_propensity(int cell_i, vector<int> compartments, vector<vector<int>> adjs) {
    // zero condition: if N-1 < 0
    double N = compartments[cell_i * 3];
    
    if (N - 1 < 0) {
        return 0.0;
    } else {
        return N;
    }
}

double rxn2_propensity(int cell_i, vector<int> compartments, vector<vector<int>> adjs) {
    // zero condition: if D+1 > Z
    double Z = compartments[(cell_i * 3) + 2];
    double D = compartments[(cell_i * 3) + 1];
    
    if (D + 1 > Z) {
        return 0.0;
    } else {
        double N = compartments[cell_i * 3];
        return Z * (1 - f(N / Z));
    }
}

double rxn3_propensity(int cell_i, vector<int> compartments, vector<vector<int>> adjs) {
    // zero condition: if D-1 < 0
    double D = compartments[(cell_i * 3) + 1];
    
    if (D - 1 < 0) {
        return 0.0;
    } else {
        return D;
    }
}

pair<vector<double>, vector<vector<int>>> ssa_delta_notch(vector<int> initial_compartments,
                                                vector<vector<int>> adjs,
                                                double time_end,
                                                mt19937 gen) {
    vector<int> compartments = initial_compartments; // initialize compartments
    vector<double> times = {0.0}; // initialize times
    vector<vector<int>> compartment_solutions = {compartments};

    int num_cells = static_cast<int>(adjs.size());
    int num_rxns_per_cell = 4;
    vector<double> rxn_propensities(num_cells*num_rxns_per_cell); // preset size of rxn_propensities

    cout << "running ssa simulation..." << endl;
    double time = 0.0; // initialize time
    while (time < time_end) {
        // 1. compute sorted propensities
        // for each cell, get all of that cell's rxns and propensities
        for (int cell_i = 0; cell_i < num_cells; ++cell_i) {
            // add propensities to the conglomerated array of propensities
            rxn_propensities[(cell_i*num_rxns_per_cell) + 0] = rxn0_propensity(cell_i, compartments, adjs); // Rxn 0
            rxn_propensities[(cell_i*num_rxns_per_cell) + 1] = rxn1_propensity(cell_i, compartments, adjs); // Rxn 1
            rxn_propensities[(cell_i*num_rxns_per_cell) + 2] = rxn2_propensity(cell_i, compartments, adjs); // Rxn 2
            rxn_propensities[(cell_i*num_rxns_per_cell) + 3] = rxn3_propensity(cell_i, compartments, adjs); // Rxn 3
        }
        double total_propensity = accumulate(rxn_propensities.begin(), rxn_propensities.end(), 0);

        if (total_propensity != 0.0) { // can'time divide by 0
            // 2. sample tau and advance time
            uniform_real_distribution<double> distribution(0.0, 1.0);
            double u1 = distribution(gen);
            double tau = -log(u1) / total_propensity;
            time = min(time + tau, time_end); // don'time go over time_end

            // 3. draw rxn
            vector<int> possible_rxns(num_cells * num_rxns_per_cell);
            iota(possible_rxns.begin(), possible_rxns.end(), 0);
            int i = choose_a_rxn(possible_rxns, rxn_propensities, total_propensity, gen);

            // 4. apply rxn
            int rxn_type = i % 4;
            int cell_selected = i / 4;
            // N: (cell_i*3)
            // D: (cell_i*3)+1
            // Z: (cell_i*3)+2
            if (rxn_type == 0) {
                compartments[(cell_selected * 3)] = compartments[(cell_selected * 3)] + 1;
            } else if (rxn_type == 1) {
                compartments[(cell_selected * 3)] = compartments[(cell_selected * 3)] - 1;
            } else if (rxn_type == 2) {
                compartments[(cell_selected * 3) + 1] = compartments[(cell_selected * 3) + 1] + 1;
            } else if (rxn_type == 3) {
                compartments[(cell_selected * 3) + 1] = compartments[(cell_selected * 3) + 1] - 1;
            }

            compartment_solutions.push_back(compartments);
            times.push_back(time);
        } else { // total_propensities_by_cell == 0, no more rxns
            time = time_end;
            compartment_solutions.push_back(compartments);
            times.push_back(time);
            cout << "rxns ended" << endl;
        }
    }

    return {times, compartment_solutions};
}


void draw_hexagon(SDL_Renderer* renderer, double center_x, double center_y, double radius, int nx, int ny, uint8_t cell_color) {
    double angle = 30 * M_PI / 180;
    double window_x_shift = WINDOW_CENTER_X - (radius*(nx+1)/2);
    double window_y_shift = WINDOW_CENTER_Y - (radius*(ny+1)/2);

    // Calculate the vertices of the hexagon
    vector<double> vertices_x(6);
    vector<double> vertices_y(6);
    for (int i = 0; i < 6; ++i) {
        double x = center_x + radius * cos(angle + (2*M_PI*i/6));
        double y = center_y + radius * sin(angle + (2*M_PI*i/6));

        vertices_x[i] = x +window_x_shift;
        vertices_y[i] = y +window_y_shift;
    }
    vector<SDL_Vertex> vertices;
    for (int i = 0; i < 6; ++i) {
        if (i == 0) {
            vertices.push_back({ { static_cast<float>(vertices_x[5]),static_cast<float>(vertices_y[5]) }, { cell_color, cell_color, cell_color, 255 }, { 0, 0 } });
        } else {
            vertices.push_back({ { static_cast<float>(vertices_x[i-1]),static_cast<float>(vertices_y[i-1]) }, { cell_color, cell_color, cell_color, 255 }, { 0, 0 } });
        }
        vertices.push_back({ { static_cast<float>(vertices_x[i]),static_cast<float>(vertices_y[i]) }, { cell_color, cell_color, cell_color, 255 }, { 0, 0 } });
        vertices.push_back({ { static_cast<float>(center_x +window_x_shift), static_cast<float>(center_y +window_y_shift) }, { cell_color, cell_color, cell_color, 255 }, { 0, 0 } });
    }
    
    // Draw the hexagon
    SDL_RenderGeometry(renderer, nullptr, vertices.data(), vertices.size(), nullptr, 0);
}

void draw_hexagonal_grid(SDL_Renderer* renderer, int nx, int ny, vector<int> compartments) {
    double radius = 10;

    int cell_index = 0;
    for (int i = 0; i < nx; ++i) {
        for (int j = 0; j < ny; ++j) {
            double x = (2 * radius * i) - (j * radius);
            double y = 1.5 * j * radius;

            double cell_color = 255.0*(1.0 - static_cast<double>(compartments[cell_index]) / compartments[cell_index+2]);
            draw_hexagon(renderer, x, y, radius, nx, ny, static_cast<uint8_t>(cell_color));

            cell_index += 3;
        }
    }
}

int main() {
    // INTITIALIZATION =============================================================
    int nx = 8; // replace with your desired values
    int ny = 8; // replace with your desired values
    double time_end = 10.0; // replace with your desired time_end
    mt19937 gen(314); // supposedly this seeds the rand num generator

    // get grid of adjs and initial_compartments
    auto grid_result = get_grid(nx, ny, gen);
    vector<vector<int>> adjs = grid_result.first;
    vector<int> initial_compartments = grid_result.second;

    // RUNNING SIMULATION =============================================================
    pair<vector<double>, vector<vector<int>>> ssa_result = ssa_delta_notch(initial_compartments, adjs, time_end, gen);

    vector<double> ssa_times = ssa_result.first;
    vector<vector<int>> ssa_compartment_sols = ssa_result.second;

    // DISPLAYING SIMULATION RESULTS ===================================================
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
    // Using nested loops to iterate through the vector of vectors
    for (size_t t = 0; t < ssa_compartment_sols.size()+1; ++t) {
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

        // Render hexagons
        draw_hexagonal_grid(renderer, nx, ny, ssa_compartment_sols[t]);
        
        // Render
        SDL_RenderPresent(renderer);
        SDL_Delay((ssa_times[t+1] - ssa_times[t]) / 10);
    }
    return 0;
}
