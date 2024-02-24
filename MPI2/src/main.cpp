#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <chrono>
#include <iostream>
#include <vector>

// Генерация массива чисел
void generateArray(int* array, int size) {
    for (int i = 0; i < size; ++i) {
        array[i] = rand() % 10;
    }
}

// Вычисление суммы массива чисел
int sumArray(int* array, int size) {
    int sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += array[i];
    }
    return sum;
}

int main(int argc, char* argv[]) {
    // np -- количество процессов
    // pid -- номер процесса
    int pid, np, elements_per_process, n_elements_recieved;
    std::vector<int> sizes = {10,      1000,     10000,     100000,
                              1000000, 10000000, 100000000, 1000000000};
    // Статус отправки/получения
    MPI_Status status;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    int* a = nullptr;
    for (auto size : sizes) {
        // Главный процесс
        if (pid == 0) {
            int* a = new int[size];
            int index, i;
            elements_per_process = size / np;

            generateArray(a, size);

            // Замер времени начала выполнения
            auto start_time = std::chrono::high_resolution_clock::now();
            //Раздача задач
            if (np > 1) {
                for (i = 1; i < np - 1; i++) {
                    index = i * elements_per_process;

                    MPI_Send(&elements_per_process, 1, MPI_INT, i, 0,
                             MPI_COMM_WORLD);
                    MPI_Send(&a[index], elements_per_process, MPI_INT, i, 0,
                             MPI_COMM_WORLD);
                }

                index = i * elements_per_process;
                int elements_left = size - index;

                MPI_Send(&elements_left, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                MPI_Send(&a[index], elements_left, MPI_INT, i, 0,
                         MPI_COMM_WORLD);
            }

            int sum = sumArray(a, elements_per_process);

            // Собираем все суммы
            int tmp;
            for (i = 1; i < np; i++) {
                MPI_Recv(&tmp, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,
                         &status);
                sum += tmp;
            }

            auto end_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = end_time - start_time;

            std::cout << "Размер массива: " << size << ", Сумма: " << sum
                      << ", Время выполнения: " << (elapsed.count())
                      << " секунд" << std::endl;
        }
        //Работа других процессов
        else {
            MPI_Recv(&n_elements_recieved, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,
                     &status);

            a = new int[n_elements_recieved];
            MPI_Recv(a, n_elements_recieved, MPI_INT, 0, 0, MPI_COMM_WORLD,
                     &status);

            int partial_sum = sumArray(a, n_elements_recieved);

            MPI_Send(&partial_sum, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
        delete[] a;
    }
    MPI_Finalize();

    return 0;
}
