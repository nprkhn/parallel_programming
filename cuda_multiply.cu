#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <cuda_runtime.h>

using namespace std;

template<int BLOCK_SIZE>
__global__ void matmul_kernel(const int* A, const int* B, int* C, int n) {
    __shared__ int As[BLOCK_SIZE][BLOCK_SIZE];
    __shared__ int Bs[BLOCK_SIZE][BLOCK_SIZE];

    int bx = blockIdx.x, by = blockIdx.y;
    int tx = threadIdx.x, ty = threadIdx.y;

    int row = by * BLOCK_SIZE + ty;
    int col = bx * BLOCK_SIZE + tx;

    int sum = 0;
    for (int k = 0; k < (n + BLOCK_SIZE - 1) / BLOCK_SIZE; ++k) {
        if (row < n && k * BLOCK_SIZE + tx < n)
            As[ty][tx] = A[row * n + k * BLOCK_SIZE + tx];
        else
            As[ty][tx] = 0;

        if (col < n && k * BLOCK_SIZE + ty < n)
            Bs[ty][tx] = B[(k * BLOCK_SIZE + ty) * n + col];
        else
            Bs[ty][tx] = 0;

        __syncthreads();

        for (int i = 0; i < BLOCK_SIZE; ++i) {
            sum += As[ty][i] * Bs[i][tx];
        }
        __syncthreads();
    }

    if (row < n && col < n) {
        C[row * n + col] = sum;
    }
}

vector<int> readMatrix(const string& filename, int& n) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        exit(1);
    }
    file >> n;
    vector<int> matrix(n * n);
    for (int i = 0; i < n * n; ++i) {
        file >> matrix[i];
    }
    file.close();
    return matrix;
}

void writeMatrix(const string& filename, const vector<int>& matrix, int n) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error writing file: " << filename << endl;
        exit(1);
    }
    file << n << '\n';
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            file << matrix[i * n + j];
            if (j != n - 1) file << ' ';
        }
        file << '\n';
    }
    file.close();
}

int main(int argc, char* argv[]) {
    int block_size = 16;
    if (argc > 1) block_size = atoi(argv[1]);
    if (block_size != 16 && block_size != 32) {
        cerr << "Block size must be 16 or 32" << endl;
        return 1;
    }

    int n;
    vector<int> A = readMatrix("matrix_a.txt", n);
    int m;
    vector<int> B = readMatrix("matrix_b.txt", m);
    if (n != m) {
        cerr << "Matrix size mismatch" << endl;
        return 1;
    }

    size_t bytes = n * n * sizeof(int);
    int *d_A, *d_B, *d_C;
    cudaMalloc(&d_A, bytes);
    cudaMalloc(&d_B, bytes);
    cudaMalloc(&d_C, bytes);

    cudaMemcpy(d_A, A.data(), bytes, cudaMemcpyHostToDevice);
    cudaMemcpy(d_B, B.data(), bytes, cudaMemcpyHostToDevice);

    dim3 threads(block_size, block_size);
    dim3 blocks((n + block_size - 1) / block_size, (n + block_size - 1) / block_size);

    auto start = chrono::high_resolution_clock::now();

    if (block_size == 16) {
        matmul_kernel<16><<<blocks, threads>>>(d_A, d_B, d_C, n);
    } else {
        matmul_kernel<32><<<blocks, threads>>>(d_A, d_B, d_C, n);
    }
    cudaDeviceSynchronize();

    auto end = chrono::high_resolution_clock::now();
    double duration = chrono::duration<double>(end - start).count();

    vector<int> C(n * n);
    cudaMemcpy(C.data(), d_C, bytes, cudaMemcpyDeviceToHost);

    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);

    writeMatrix("result.txt", C, n);

    long long taskVolume = (long long)n * n * (2LL * n - 1);
    cout << "Execution time: " << duration << " s" << endl;
    cout << "Task volume: " << taskVolume << endl;

    return 0;
}