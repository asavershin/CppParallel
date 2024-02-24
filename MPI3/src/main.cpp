#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <chrono>
#include <cmath>
#include <iostream>
#include <vector>

#define N 10000  // Выдаю сразу память под все тесты

double A[N][N], B[N][N];

// Определим функцию, для которой будем вычислять производную
double function(double x, double y) {
    return x * (sin(x) + cos(y));
}

double dx = 0.01;

void generateMatrix(int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            A[i][j] = function(i * dx, j * dx);
        }
    }
}

// Функция для вычисления производной по переменной x на сетке
void computeDerivativeX(int rows, int cols) {
    // Вычисление производной по переменной x
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (j == 0 || j == cols - 1) {
                // Простые конечные разности на краях сетки
                B[i][j] = (A[i][j + 1] - A[i][j]) / dx;
            } else {
                // Центральная разность внутри сетки
                B[i][j] = (A[i][j + 1] - A[i][j - 1]) / (2 * dx);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    // np -- количество процессов
    // pid -- номер процесса
    int pid, np, elements_per_process, n_elements_recieved, index;
    std::vector<int> sizes = {10, 100, 1000, 10000};
    // Статус отправки/получения
    MPI_Status status;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    for (auto size : sizes) {
        // Главный процесс
        if (pid == 0) {
            generateMatrix(size, size);
            int i;
            elements_per_process = size / np;

            // Замер времени начала выполнения
            auto start_time = std::chrono::high_resolution_clock::now();
            //Раздача задач
            if (np > 1) {
                for (i = 1; i < np - 1; i++) {
                    index = i * elements_per_process;

                    MPI_Send(&elements_per_process, 1, MPI_INT, i, 0,
                             MPI_COMM_WORLD);
                    MPI_Send(&A[index][0], elements_per_process * size,
                             MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
                    MPI_Send(&index, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                }

                index = i * elements_per_process;
                int elements_left = size - index;
                MPI_Send(&elements_left, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                MPI_Send(&A[index][0], elements_left * size, MPI_DOUBLE, i, 0,
                         MPI_COMM_WORLD);
                MPI_Send(&index, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            }

            // Считаем самую первую часть производной
            computeDerivativeX(elements_per_process, size);

            // Принимаем производные от других процессов и добавляем их к общему
            // массиву производных
            for (int i = 1; i < np; i++) {
                MPI_Recv(&index, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
                MPI_Recv(&n_elements_recieved, 1, MPI_INT, i, 1, MPI_COMM_WORLD,
                         &status);
                MPI_Recv(&B[index][0], n_elements_recieved * size, MPI_DOUBLE,
                         i, 1, MPI_COMM_WORLD, &status);
            }

            auto end_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = end_time - start_time;

            std::cout << "Размер матрицы: " << size << "x" << size
                      << " Время выполнения: " << (elapsed.count()) << " секунд"
                      << std::endl;

        }
        //Работа других процессов
        else {
            MPI_Recv(&n_elements_recieved, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,
                     &status);

            MPI_Recv(&A, n_elements_recieved * size, MPI_DOUBLE, 0, 0,
                     MPI_COMM_WORLD, &status);
            MPI_Recv(&index, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

            // Вычисление производной для полученной части массива
            computeDerivativeX(n_elements_recieved, size);

            // Отправка вычисленных производных обратно в главный процесс

            MPI_Send(&index, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
            MPI_Send(&n_elements_recieved, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
            MPI_Send(&B, n_elements_recieved * size, MPI_DOUBLE, 0, 1,
                     MPI_COMM_WORLD);
        }
    }
    MPI_Finalize();

    return 0;
}
