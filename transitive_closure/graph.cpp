// Inspired by: https://www.geeksforgeeks.org/how-to-create-a-random-graph-in-c/

// Why to linearize: https://community.khronos.org/t/matrix-multiplication-using-2d-std-vector/106457#:~:text=On%20a%20more,be%20known%20statically.

#include <stdio.h>
#include "graph.h"

// This function generates a directed graph that has no loops and no duplicate edges. The resulting graph
// is stored in adj_matrix.
int generate_simple_directed_graph(std::vector<std::vector<int>> & adj_matrix, int num_edge, int num_vertex)
{
    // Maximum number of edges = n x (n - 1)
    if (num_edge > num_vertex * (num_vertex - 1)) {
        std::cout << "Too many edges." << std::endl;
        return 1;
    }

    // Resizing adj_matrix (num_vertex by num_vertex).
    adj_matrix.resize(num_vertex, std::vector<int>(num_vertex, 0));

    int edge[num_edge][2];
    int i = 0;
    // Generate directed graph.
    while (i < num_edge) {
        // Generate the vertices connected by edge.
        edge[i][0] = rand() % num_vertex;
        edge[i][1] = rand() % num_vertex;

        // Discard loop.
        if (edge[i][0] == edge[i][1]) {
            continue;
        }
        // Discard duplicate edge.
        else {
            for (int j = 0; j < i; j++) {
                if ((
                    edge[i][0] == edge[j][0] &&
                    edge[i][1] == edge[j][1]
                    ))
                {
                    i--;
                }
            }
        }
        i++;
    }

    // Initialize adjacency matrix.
    int vertex_one, vertex_two;
    for (int j = 0; j < num_edge; j++) {
        // Set entries with connecting edges to 1.
        vertex_one = edge[j][0];
        vertex_two = edge[j][1];
        adj_matrix[vertex_one][vertex_two] = 1;
    }
    return 0;
}

// This function linearizes parameter adj_matrix and stores in linear_adj_matrix.
void linearize(std::vector<std::vector<int>> & adj_matrix, std::vector<int> & linear_adj_matrix, int num_vertex)
{
    linear_adj_matrix.resize(num_vertex * num_vertex);
    for (int i = 0; i < num_vertex; i++) {
        for (int j = 0; j < num_vertex; j++) {
            linear_adj_matrix[i * num_vertex + j] = adj_matrix[i][j];
        }
    }
}

void print_adj_matrix(std::vector<std::vector<int>> & adj_matrix, int num_vertex)
{
    for (int i = 0; i < num_vertex; i++) {
        for (int j = 0; j < num_vertex; j++) {
            std::cout << adj_matrix[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

void print_linear_adj_matrix(std::vector<int> & linear_adj_matrix, int num_vertex)
{
    for (int i = 0; i < num_vertex * num_vertex; i++) {
        std::cout << linear_adj_matrix[i] << " ";
        if ((i+1)%num_vertex==0) {
            std::cout << std::endl;
        }
    }
}