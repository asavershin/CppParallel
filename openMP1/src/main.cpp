#include <omp.h>

#include <iostream>

int main() {
		// Указываем OpenMP, что следующий участок кода нужно выполнить
		// параллельно #pragma omp parallel num_threads(2) { Получаем номер
		// текущего потока
		int thread_num = omp_get_thread_num();

		// Выполняем какие-то действия
		int i = 0;
		for (; i < INT32_MAX - 1; i += 1)
				;
#pragma omp critical
		{ std::cout << "Поток " << thread_num << " результат: " << i << std::endl; }
		// }

		std::cout << "Hello";
		return 0;
}
