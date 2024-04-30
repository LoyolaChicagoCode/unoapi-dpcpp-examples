#include <sycl/sycl.hpp>
#include <iostream>
#include <limits>
#include <chrono>
#include <fstream>

#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <stdio.h>
#include <math.h>

int populate_matrix(std::string & filename, sycl::buffer<int, 2> & C_buf)
{
    const sycl::host_accessor C(C_buf);
    std::ifstream input(filename);
    
    if (!input.is_open()) {
        std::cout << "The file could not be opened.";
        return 1;
    }
    
    int rows, columns;
    input >> rows >> columns;
    
    if (rows != columns) { return 1; }
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            input >> C[i][j];
        }
    }
    
    input.close();
    return 0;
    
}

int verify_matrix_squared(std::string & filename, int & n)
{
    int rows, columns;
    std::ifstream input(filename);
    
    if (!input.is_open()) {
        std::cout << "The file could not be opened.";
        return 1;
    }
    
    input >> rows >> columns;
    n = rows;
    
    if (rows != columns) { return 1; }
    
    input.close();
    return 0;
}

int main()
{
    
    std::string filename{"/home/u211094/indep-study/unoapi-dpcpp-examples/transitive_closure/data.txt"}; // file to read matrix from
    
    int n;
    
    // n stores dimension of matrix in 1 direction
    if (verify_matrix_squared(filename, n) == 1) { return 1; }
    
    sycl::buffer<int, 2> C_buf(sycl::range(n, n));
    sycl::buffer<int, 2> A_buf(sycl::range(n, n));
    
    if (populate_matrix(filename, C_buf) == 1) { return 1; }
    sycl::queue q(sycl::default_selector_v);
    
    
    // wardell procedure
    // step 1: copy adjacency matrix C into matrix A
    // for i := 1 to n do 
    //   for j := 1 to n do
    //     A[i, j] := C[i, j]
    //
    q.submit([&](auto &h) {
        const sycl::accessor C(C_buf, h, sycl::read_only);
        const sycl::accessor A(A_buf, h, sycl::write_only);
        
        h.parallel_for(sycl::range(n, n), [=](auto index) {
            A[index] = C[index];
            
        });
    });
    
    
    // wardell procedure
    // step 2: compute the transitive closure of C as A
    //
    // for k := 1 to n do
    //   for i := 1 to n do
    //     for j := 1 to n do
    //       if A[i, j] = false then
    //         A[i, j] := A[i, k] and A[k, j]
    //
    for (int k = 0; k < n; k++) {
        
        q.submit([&](auto &h) {
            const sycl::accessor A(A_buf, h, sycl::write_only);

            h.parallel_for(sycl::range(n, n), [=](auto index) {
                A[index] = !A[index] ? ( A[index[0]][k] * A[k][index[1]] ) : A[index];
                
            });
        });
    }
    
    
    // print matrix C and transitive closure A
    //
    const sycl::host_accessor C{ C_buf };
    const sycl::host_accessor A{ A_buf };
    
    std::cout << "Matrix C: " << std::endl;
    for (int i = 0; i < n; i ++) {
        for (int j = 0; j < n; j++) {
            std::cout << C[i][j] << " ";
        }
        std::cout << std::endl;
    }
    
    std::cout << "Transitive Closure A of C: " << std::endl;
    for (int i = 0; i < n; i ++) {
        for (int j = 0; j < n; j++) {
            std::cout << A[i][j] << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}