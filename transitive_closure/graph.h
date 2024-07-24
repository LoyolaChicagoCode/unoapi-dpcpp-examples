#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <iostream>

int generate_simple_directed_graph(
    std::vector<std::vector<int>> & adj_matrix,
    int num_edge,
    int num_vertex
    );

void linearize(
    std::vector<std::vector<int>> & adj_matrix,
    std::vector<int> & linear_adj_matrix,
    int num_vertex
    );

void print_adj_matrix(
    std::vector<std::vector<int>> & adj_matrix,
    int num_vertex
    );

void print_linear_adj_matrix(
    std::vector<int> & linear_adj_matrix,
    int num_vertex
    );

#endif