#include <mpi.h>

#include <iostream>

int main(int argc, char **argv) {
	// Инициализация MPI
	MPI_Init(&argc, &argv);

	// Получение общего числа процессов
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	// Получение ранга текущего процесса
	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	// Выполнение каких-то действий в каждом процессе
	int i = 0;
	for (; i < INT32_MAX - 1; i += 1)
		;

	// Вывод результата каждым процессом с использованием ранга
	std::cout << "Процесс " << world_rank << " результат: " << i
		  << std::endl;

	// Завершение MPI
	MPI_Finalize();

	return 0;
}