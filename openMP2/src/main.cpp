#include <omp.h>

#include <iostream>
#include <vector>

std::vector<int> generateRandomNumbers(int size) {
    std::vector<int> numbers(size);
    for (int i = 0; i < size; ++i) {
        numbers[i] = rand() % 10;
    }
    return numbers;
}

int main() {
    std::vector<int> sizes = {10,      1000,     10000,     100000,
                              1000000, 10000000, 100000000, 1000000000};

    for (int size : sizes) {
        std::vector<int> numbers = generateRandomNumbers(size);

        double start_time = omp_get_wtime();

        // Используем OpenMP для распараллеливания подсчета суммы массива
        int sum = 0;
#pragma omp parallel for redeuction(+ : sum)
        for (int i = 0; i < size; ++i) {
            sum += numbers[i];
        }

        double end_time = omp_get_wtime();

        std::cout << "Размер массива: " << size << ", Сумма: " << sum
                  << ", Время выполнения: " << (end_time - start_time)
                  << " секунд" << std::endl;
    }

    return 0;
}
