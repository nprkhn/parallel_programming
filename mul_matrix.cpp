#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <string>
#include <sstream>
#include <omp.h>

using namespace std;

vector<vector<int>> readMatrix(const string& filename, int& n) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error with open the file!" << filename << endl;
        exit(1);
    }
    file >> n;
    vector<vector<int>> matrix(n, vector<int>(n));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            file >> matrix[i][j];
        }
    }
    file.close();
    return matrix;
}

void writeMatrix(const string& filename, const vector<vector<int>>& matrix, int n) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error with write in the file! " << filename << endl;
        exit(1);
    }
    file << n << '\n';
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            file << matrix[i][j];
            if (j != n - 1) file << ' ';
        }
        file << '\n';
    }
    file.close();
}

vector<vector<int>> multiplyMatrices(const vector<vector<int>>& A, const vector<vector<int>>& B, int n, long long& taskVolume, int num_threads) {
    vector<vector<int>> C(n, vector<int>(n, 0));
    taskVolume = 0;

    omp_set_num_threads(num_threads);

    int actual_threads = 1;
    #pragma omp parallel
    {
        #pragma omp single
        actual_threads = omp_get_num_threads();
    }
    std::cout << "Num threads: " << actual_threads << std::endl;

    #pragma omp parallel for
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            for (int k = 0; k < n; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }

    taskVolume = (long long)n * n * (2 * n - 1);
    return C;
}

int main(int argc, char* argv[]) {
    int num_threads = 1;

    if (argc > 1) num_threads = stoi(argv[1]);

    int n;
    vector<vector<int>> A = readMatrix("matrix_a.txt", n);

    int m;
    vector<vector<int>> B = readMatrix("matrix_b.txt", m);
    if (n != m) {
        cerr << "Error: matr's sizes not exist!" << endl;
        return 1;
    }

    long long taskVolume = 0;

    auto start = chrono::high_resolution_clock::now();
    vector<vector<int>> C = multiplyMatrices(A, B, n, taskVolume, num_threads);
    auto end = chrono::high_resolution_clock::now();

    double duration = chrono::duration<double>(end - start).count();

    writeMatrix("result.txt", C, n);

    cout << "Execution time: " << duration << " s" << endl;
    cout << "Task volume: " << taskVolume << endl;

    return 0;
}