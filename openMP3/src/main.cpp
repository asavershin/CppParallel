#include <omp.h>

#include <cmath>
#include <iostream>
#include <vector>

// Определим функцию, для которой будем вычислять производную
double function(double x, double y) {
    return x * (sin(x) + cos(y));
}

// Функция для вычисления производной по переменной x на сетке
void computeDerivativeX(const std::vector<std::vector<double>>& A,
                        std::vector<std::vector<double>>& B, double dx) {
    int rows = A.size();
    int cols = A[0].size();

// Вычисление производной по переменной x
#pragma omp parallel for collapse(2)
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

int main() {
    std::vector<int> sizes = {10, 100, 1000, 10000};

    for (auto size : sizes) {
        int rows = size;
        int cols = size;
        double dx = 0.01;

        std::vector<std::vector<double>> A(rows, std::vector<double>(cols));
        std::vector<std::vector<double>> B(rows, std::vector<double>(cols));

        // Заполнение массива значениями функции
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                A[i][j] = function(i * dx, j * dx);
            }
        }

        // Засекаем время начала выполнения алгоритма
        double start_time = omp_get_wtime();

        computeDerivativeX(A, B, dx);

        // Засекаем время окончания выполнения алгоритма
        double end_time = omp_get_wtime();

        std::cout << "Размер сетки: " << rows << "x" << cols
                  << " Время выполнения: " << (end_time - start_time)
                  << " секунд" << std::endl;
    }

    return 0;
}
