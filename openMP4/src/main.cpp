#include <omp.h>

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>

std::vector<std::vector<int>> generateRandomMatrix(int rows, int cols) {
    std::vector<std::vector<int>> matrix(rows, std::vector<int>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = rand() % 9 + 1;
        }
    }
    return matrix;
}

std::vector<std::vector<int>> multiplyMatrices(
    const std::vector<std::vector<int>>& matrix1,
    const std::vector<std::vector<int>>& matrix2) {
    int rows1 = matrix1.size();
    int cols1 = matrix1[0].size();
    int rows2 = matrix2.size();
    int cols2 = matrix2[0].size();

    // Проверяем возможность умножения матриц
    if (cols1 != rows2) {
        std::cerr << "Невозможно умножить матрицы: неправильные размеры"
                  << std::endl;
        exit(1);
    }

    std::vector<std::vector<int>> result(rows1, std::vector<int>(cols2));

    // Используем директиву OpenMP для распараллеливания вычислений
#pragma omp parallel for collapse(2)
    for (int i = 0; i < rows1; ++i) {
        for (int j = 0; j < cols2; ++j) {
            int sum = 0;
            for (int k = 0; k < cols1; ++k) {
                sum += matrix1[i][k] * matrix2[k][j];
            }
            result[i][j] = sum;
        }
    }

    return result;
}

int main() {
    std::vector<std::pair<int, int>> matrix_sizes = {
        {10, 10}, {100, 100}, {1000, 1000}, {2000, 2000}};

    for (const auto& size : matrix_sizes) {
        int rows1 = size.first;
        int cols1 = size.second;
        int rows2 = cols1;
        int cols2 = size.first;

        std::vector<std::vector<int>> matrix1 =
            generateRandomMatrix(rows1, cols1);
        std::vector<std::vector<int>> matrix2 =
            generateRandomMatrix(rows2, cols2);

        // Засекаем время начала выполнения алгоритма
        double start_time = omp_get_wtime();

        std::vector<std::vector<int>> result =
            multiplyMatrices(matrix1, matrix2);

        // Засекаем время окончания выполнения алгоритма
        double end_time = omp_get_wtime();

        std::cout << "Размеры матриц: " << rows1 << "x" << cols1 << " и "
                  << rows2 << "x" << cols2
                  << ", время выполнения: " << (end_time - start_time)
                  << " секунд" << std::endl;
    }

    return 0;
}
