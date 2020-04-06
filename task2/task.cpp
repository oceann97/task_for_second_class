#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_max.h>
#include <cilk/reducer_min.h>
#include <cilk/reducer_vector.h>
#include <chrono>
#include <stdlib.h>
#include <iostream>
#include <vector>

using namespace std::chrono;

/// Сункци§ ReducerMaxTest() определ§ет максимальный элемент массива,
/// переданного ей в качестве аргумента, и его позицию
/// mass_pointer - указатель исходный массив целых чисел
/// size - количество элементов в массиве
void ReducerMaxTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_max_index<long, int>> maximum;
	cilk_for(long i = 0; i < size; ++i)
	{
		maximum->calc_max(i, mass_pointer[i]);
	}
	printf("Maximal element = %d has index = %d\n\n",
		maximum->get_reference(), maximum->get_index_reference());
}

void ReducerMinTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_min_index<long, int>> minimum;
	cilk_for(long i = 0; i < size; ++i)
	{
		minimum->calc_min(i, mass_pointer[i]);
	}
	printf("Minimal element = %d has index = %d\n\n",
		minimum->get_reference(), minimum->get_index_reference());
}

/// Сункци§ ParallelSort() сортирует массив в пор§дке возрастани§
/// begin - указатель на первый элемент исходного массива
/// end - указатель на последний элемент исходного массива
duration<double> ParallelSort(int *begin, int *end)
{
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	if (begin != end)
	{
		--end;
		int *middle = std::partition(begin, end, std::bind2nd(std::less<int>(), *end));
		std::swap(*end, *middle);
		cilk_spawn ParallelSort(begin, middle);
		ParallelSort(++middle, ++end);
		cilk_sync;
	}
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration<double> duration = (t2 - t1);
	return duration;
}


void CompareForAndCilk_For(size_t sz)
{
	std::vector<int> my_vect;
	cilk::reducer<cilk::op_vector<int>>red_vec;
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	for(int i = 0; i < sz; i++)
		my_vect.push_back(rand() % 20000 + 1);
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	cilk_for (int i = 0; i < sz; i++)
		red_vec->push_back(rand() % 20000 + 1);
	high_resolution_clock::time_point t3 = high_resolution_clock::now();
	duration<double> duration_for = (t2 - t1);
	duration<double> duration_cilk = (t3 - t2);
	std::cout << "Size is: " << sz << std::endl;
	std::cout << "Duration 'for' is: " << duration_for.count() << " seconds" << std::endl;
	std::cout << "Duration 'cilk_for' is: " << duration_cilk.count() << " seconds" << std::endl;
}

int main()
{
	srand((unsigned)time(0));

	// устанавливаем количество работающих потоков = 4
	__cilkrts_set_param("nworkers", "4");

	long i;
	const long mass_size = 1000000;
	std::cout << "Massive size:" << mass_size << std::endl;
	int *mass_begin, *mass_end;
	int *mass = new int[mass_size];

	for (i = 0; i < mass_size; ++i)
	{
		mass[i] = (rand() % 25000) + 1;
	}

	mass_begin = mass;
	mass_end = mass_begin + mass_size;
	ReducerMaxTest(mass, mass_size);
	ReducerMinTest(mass, mass_size);
	duration<double> duration = ParallelSort(mass_begin, mass_end);
	std::cout << "Duration is: " << duration.count() << " seconds" << std::endl << std::endl;
	ReducerMaxTest(mass, mass_size);
	ReducerMinTest(mass, mass_size);
	delete[]mass;
	
	int sizes[] = { 10, 50, 100, 500, 1000, 10000, 100000, 1000000 };
	for (int i = 0; i < 8; i++)
		CompareForAndCilk_For(sizes[i]);
	//CompareForAndCilk_For(100);
	system("pause");
	return 0;
}