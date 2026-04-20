#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <mpi.h>

using namespace std;

vector<vector<int>> readMatrix(const string& filename, int& n) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    file >> n;
    vector<vector<int>> matrix(n, vector<int>(n));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            file >> matrix[i][j];
    file.close();
    return matrix;
}

void writeMatrix(const string& filename, const vector<vector<int>>& matrix, int n) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error writing to file: " << filename << endl;
        return;
    }
    file << n << '\n';
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            file << matrix[i][j];
            if (j != n-1) file << ' ';
        }
        file << '\n';
    }
    file.close();
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int n = 0;
    vector<vector<int>> A, B;

    if (rank == 0) {
        int nA, nB;
        A = readMatrix("matrix_a.txt", nA);
        B = readMatrix("matrix_b.txt", nB);
        if (nA != nB) {
            cerr << "Matrix sizes do not match!" << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        n = nA;
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    A.resize(n, vector<int>(n));
    B.resize(n, vector<int>(n));

    for (int i = 0; i < n; ++i) {
        MPI_Bcast(&A[i][0], n, MPI_INT, 0, MPI_COMM_WORLD);
    }
    for (int i = 0; i < n; ++i) {
        MPI_Bcast(&B[i][0], n, MPI_INT, 0, MPI_COMM_WORLD);
    }

    int rows_per_proc = n / size;
    int remainder = n % size;
    int offset = rank * rows_per_proc + min(rank, remainder);
    int local_rows = rows_per_proc + (rank < remainder ? 1 : 0);

    vector<vector<int>> C_local(local_rows, vector<int>(n, 0));
    long long local_volume = 0;

    MPI_Barrier(MPI_COMM_WORLD);
    auto start = chrono::high_resolution_clock::now();

    for (int i = 0; i < local_rows; ++i) {
        int global_i = offset + i;
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            for (int k = 0; k < n; ++k) {
                sum += A[global_i][k] * B[k][j];
                ++local_volume;
            }
            C_local[i][j] = sum;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    auto end = chrono::high_resolution_clock::now();
    double duration = chrono::duration<double>(end - start).count();

    vector<vector<int>> C_full;
    if (rank == 0) {
        C_full.resize(n, vector<int>(n));
        for (int i = 0; i < local_rows; ++i)
            C_full[offset + i] = C_local[i];
        for (int p = 1; p < size; ++p) {
            int p_offset = p * rows_per_proc + min(p, remainder);
            int p_rows = rows_per_proc + (p < remainder ? 1 : 0);
            for (int i = 0; i < p_rows; ++i) {
                MPI_Recv(&C_full[p_offset + i][0], n, MPI_INT, p, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
    } else {
        for (int i = 0; i < local_rows; ++i) {
            MPI_Send(&C_local[i][0], n, MPI_INT, 0, i, MPI_COMM_WORLD);
        }
    }

    long long total_volume = 0;
    MPI_Reduce(&local_volume, &total_volume, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        writeMatrix("result.txt", C_full, n);
        cout << "Execution time: " << duration << " s" << endl;
        cout << "Task volume: " << total_volume << endl;
    }

    MPI_Finalize();
    return 0;
}