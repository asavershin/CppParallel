#include <mpi.h>

#include <chrono>
#include <iostream>
#include <vector>
#define N 2000

MPI_Status status;

double a[N][N], b[N][N], c[N][N];

void generateMatrix(int rows, int cols, double array[N][N]) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            array[i][j] = rand() % 10;
        }
    }
}

void multiplyMatrices(int rows1, int cols1, int rows2, int cols2) {
    if (cols1 != rows2) {
        std::cerr << "Невозможно умножить матрицы: неправильные размеры"
                  << std::endl;
        exit(1);
    }

    for (int i = 0; i < rows1; ++i)
        for (int j = 0; j < cols2; ++j) {
            for (int k = 0; k < cols1; ++k)
                c[i][j] += a[i][k] * b[k][j];
        }
}

main(int argc, char** argv) {
    int pid, np, source, i, element_per_process, index, j, k;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    std::vector<int> sizes = {10, 100, 1000, 2000};

    for (auto size : sizes) {
        // Главный процесс
        if (pid == 0) {
            generateMatrix(size, size, a);
            generateMatrix(size, size, b);

            auto start_time = std::chrono::high_resolution_clock::now();

            element_per_process = size / np;
            index = element_per_process;
            // Раздаём задачи
            if (np > 1) {
                for (i = 1; i < np - 1; ++i) {
                    MPI_Send(&index, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
                    MPI_Send(&element_per_process, 1, MPI_INT, i, 1,
                             MPI_COMM_WORLD);
                    MPI_Send(&a[index][0], element_per_process * size,
                             MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
                    MPI_Send(&b, size * size, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
                    index += element_per_process;
                }

                int elements_left = size - index;
                MPI_Send(&index, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
                MPI_Send(&elements_left, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
                MPI_Send(&a[index][0], elements_left * size, MPI_DOUBLE, i, 1,
                         MPI_COMM_WORLD);
                MPI_Send(&b, size * size, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
            }

            // Главный процесс не простаивает и тоже выполняет задачу
            multiplyMatrices(element_per_process, size, size, size);

            // Получаем результаты и записываем их в одну таблицу
            for (i = 1; i < np; i++) {
                MPI_Recv(&index, 1, MPI_INT, i, 2, MPI_COMM_WORLD, &status);
                MPI_Recv(&element_per_process, 1, MPI_INT, i, 2, MPI_COMM_WORLD,
                         &status);
                MPI_Recv(&c[index][0], element_per_process * size, MPI_DOUBLE,
                         i, 2, MPI_COMM_WORLD, &status);
            }

            auto end_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = end_time - start_time;

            std::cout << "Размер сетки: " << size << "x" << size
                      << " Время выполнения: " << elapsed.count() << " секунд"
                      << std::endl;
        }  // другие процессы
        else {
            source = 0;
            MPI_Recv(&index, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(&element_per_process, 1, MPI_INT, source, 1,
                     MPI_COMM_WORLD, &status);
            MPI_Recv(&a, element_per_process * size, MPI_DOUBLE, source, 1,
                     MPI_COMM_WORLD, &status);
            MPI_Recv(&b, size * size, MPI_DOUBLE, source, 1, MPI_COMM_WORLD,
                     &status);

            multiplyMatrices(element_per_process, size, size, size);

            MPI_Send(&index, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
            MPI_Send(&element_per_process, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
            MPI_Send(&c, element_per_process * size, MPI_DOUBLE, 0, 2,
                     MPI_COMM_WORLD);
        }
    }
    MPI_Finalize();
}
