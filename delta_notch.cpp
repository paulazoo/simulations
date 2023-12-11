#include <iostream>
#include <vector>

using namespace std;

// nx and ny are cells on the x and y sides of the grid
pair<vector<vector<int>>, vector<int>> get_grid_a(int nx, int ny) {
    vector<vector<int>> adjs;
    for (int i = 0; i < nx; ++i) {
        for (int j = 0; j < ny; ++j) {
            vector<int> adj;
            // adjs is a list of the 6 neighbors for each cell
            vector<pair<int, int>> pair_vector = {{i - 1, j - 1}, {i, j - 1}, {i - 1, j},
                                                               {i + 1, j}, {i, j + 1}, {i + 1, j + 1}};
            for (int p = 0; p < pair_vector.size(); ++p) {
                
                pair<int, int> xy_pair = pair_vector[p];
                int x = xy_pair.first;
                int y = xy_pair.second;
                // index of neighbor if neighbor exists
                // -1 if no neighbors
                adj.push_back((x >= 0 && x < nx && y >= 0 && y < ny) ? (x * ny + y) : -1);
            }
            adjs.push_back(adj);
        }
    }

    vector<int> count; // count is all the compartments for all cells: N D Z
    for (int i = 0; i < nx * ny; ++i) {
        int Z = 100;
        if (i == 0) { // 1st cell
            int N = Z;
            int D = Z;
            count.push_back(N);
            count.push_back(D);
            count.push_back(Z);
        } else if (i == 1) { // 2nd cell
            int N = Z - 1;
            int D = N;
            count.push_back(N);
            count.push_back(D);
            count.push_back(Z);
        }
    }

    return make_pair(adjs, count);
}

int main() {
    int nx = 3; // replace with your desired values
    int ny = 3; // replace with your desired values

    return 0;
}
