#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <windows.h> 
#include <mutex>
#include <vector>
#include <conio.h>
#include <sstream>
#define N 1000
#define MAX_THREADS 3
#define PATH L"C:\\Users\\miros\\source\\repos\\lab_6_OC01\\lab_6_OC01\\result.txt"
HANDLE hFile = CreateFile(PATH, GENERIC_READ | GENERIC_WRITE, 0, nullptr,
	OPEN_EXISTING,
	FILE_ATTRIBUTE_NORMAL, nullptr); // відкриваємо файл


using namespace std;

HANDLE ghMutex;
double* threadTime;
int p;
double relProg = 0;
vector<int> vec;
double result = 0;
double sum = 0;


struct info  //структура, яку ми використовуємо для кожного з потоків
{
	int pochtk;
	int kinec;
	int index;
	HANDLE ghMutex;
};

//написати функцію цикл для рандомного заповнення масиву 
//і запису його у файл
void write_mass(const char* filename) {

	FILE* f;
	f = fopen(filename, "w");
	int arr[N];
	srand(time(NULL));
	for (int i = 0; i < N; i++) {
		arr[i] = rand();
		//arr[i] = rand()%100;   //числа в межах від 0 до 99
		fprintf(f, " %d", arr[i]);
	}
	fclose(f);
}
//-------------------------------------------------------------------------------------------------

//функція для відкриття файлу, зчитування масиву, 
//та обрахунку середнього арифметичного елементів масиву
double get_average_mass(const char* filename) {

	double res, sum = 0;

	FILE* f;
	f = fopen(filename, "r");
	int mass[N];

	if (!f)
	{
		cout << "Error. Cannot open file.";
		return false;
	}

	for (int i = 0; i < N; i++) {
		fscanf(f, " %d", mass + i);
	}

	for (int j = 0; j < N; j++)
	{
		sum += mass[j];
	}

	res = sum / N;
	cout << " Thread ID = " << this_thread::get_id() << endl;
	printf("Average array: %f \n", res);

	fclose(f);
	return res;
}

DWORD WINAPI MULT(LPVOID infoin) {///////////////////////////////////////

	info* infoin2 = (info*)infoin;  //створюємо вказівник на структуру info
	chrono::steady_clock::time_point clock_begin = chrono::steady_clock::now();  //починаємо відлік часу
	int apochtk = infoin2->pochtk;  //доступаємося до першого елемента структури і записуємо його у змінну
	int akinec = infoin2->kinec;   //доступаємося до останнього елемента структури і записуємо його у змінну
	//int sum = 0;
	for (int i = apochtk; i < akinec; i++)  //цикл по елементах структури
	{
		Sleep((rand() % 50 + 50));   //чекаємо
		sum += vec[i];   // знаходимо суму усіх елементів масиву////////////////////////////////////////
		WaitForSingleObject(ghMutex, INFINITE);   //чекаємо поки усі елементи допрацюють
		relProg++;  //рахуємо елементи
		ReleaseMutex(ghMutex);  //звільнення м'ютекса
	}

	chrono::steady_clock::time_point clock_end = chrono::steady_clock::now();  // закінчуємо відлік часу
	chrono::steady_clock::duration time_span = clock_end - clock_begin;   // рахуємо різницю, тобто скільки часу на це пішло
	threadTime[infoin2->index] = (double(time_span.count()) * chrono::steady_clock::period::num / chrono::steady_clock::period::den);  //час виконання потоку
	ReleaseMutex(ghMutex);   // звільнення м'ютекса

	ExitThread(0);//виходимо з потоку
}


int to_get_res()
{
	HANDLE thread[1000];   //масив потоків 
	//write_mass("mass.txt"); //заповнення файлу рандомними числами
	p = 0;
	ifstream fin("mass.txt");  //відкриваємо файл
	int i = 0;
	if (fin.is_open())  // перевірка чи файл відкрився
	{
		while (!fin.eof())  //поки файл не пустий 
		{
			int tmp;
			fin >> tmp;
			vec.push_back(tmp); // зчитуємо дані з файлу і записуємо(вставляємо) їх в масив vec
		}
	}
	fin.close(); // закриваємо файл


	ghMutex = CreateMutex(NULL, FALSE, NULL); //створюємо м'ютекс (атрибут безпеки, прапорець початкового власника, ім'я об'єкта)
	if (ghMutex == NULL) //якщо м'ютекс дорівнює нуль, тобто не створився
	{
		printf("CreateMutex error: %d\n", GetLastError()); //виводимо помилку
		return 1; //завершуємо програму
	}

	int krok = vec.size() / MAX_THREADS;  //ділимо кількість елементів масиву на кількість потоків і обчислюємо крок
	int etc = vec.size() % MAX_THREADS;  // остача від ділення
	int p = 0;
	int k = 0;

	threadTime = new double[MAX_THREADS];
	for (int i = 0; i < MAX_THREADS; i++)  // цикл по кількості потоків
	{
		int nextKrok = i < etc ? (krok + 1) : krok;  // обчислюємо наступний крок(якщо 1 менше за остачу, то він дорівнює крок плюс 1, а якщо ні, то - наступний крок дорівнює кроку)
		k += nextKrok;
		info* param = new info(); //створюємо вказівник на структуру
		param->index = i;  // в index записуємо значення і
		param->pochtk = p;  //в pochtk вписуємо значення p
		param->kinec = k;  //в kinec вписуємо значення k
		param->ghMutex = ghMutex;  //  в ghMutex присвоюємо значення ghMutex


		thread[i] = CreateThread(NULL, 0, MULT, (LPVOID)param, 0, 0); // створюємо потік

		p += nextKrok;  // значення p збільшуємо на nextKrok
	}
	while (relProg < vec.size())  //поки кількість елементів у функції менше за кількість елементів в масиві (відмальовуємо шкалу прогресу)
	{
		cout << relProg / vec.size() * 100 << "% \n"; //виводимо кількість процентів на кожній ітерації
		Sleep(1000);  //чекаємо 1000 мілісекунд
	}
	cout << "100% ";
	WaitForMultipleObjects(MAX_THREADS, thread, TRUE, INFINITE); //чекаємо поки усі потоки завершать своє виконання

	result = sum / N; //знаходимо середнє арифметичне

	for (DWORD i = 0; i < MAX_THREADS; ++i) //цикл по усіх відкритих потоках
	{
		CloseHandle(thread[i]); //закриваємо потік
	}

	CloseHandle(ghMutex); //закриваємо м'ютекс

	std::string tmp = std::to_string(result);
	char const* num_char = tmp.c_str();
	WriteFile(hFile, (LPVOID)num_char, sizeof(result), 0, NULL); //записуємо результат у файл
	printf("\nAverage array: %f \n", result); //виводимо результат

}

struct FileMapping {
	HANDLE hFile; //хендл файлу
	HANDLE hMapping; //хендл відображення 
	size_t fsize; // розмір файлу
	unsigned char* dataPtr; //вказівник на ділянку пам'яті з відображенням
};

int main() {

	if (hFile == INVALID_HANDLE_VALUE) {
		std::cerr << "fileMappingCreate - CreateFile failed, fname = "
			<< PATH << std::endl;
		return 0;
	}

	to_get_res();

	DWORD dwFileSize = GetFileSize(hFile, nullptr); // отримуємо розмір файлу
	if (dwFileSize == INVALID_FILE_SIZE) {
		std::cerr << "fileMappingCreate - GetFileSize failed, fname = "
			<< PATH << std::endl;
		CloseHandle(hFile);
		return 0;
	}
	HANDLE hMapping = CreateFileMapping(hFile, nullptr, PAGE_READONLY,
		0, 0, L"MyMapping"); // створюємо відображення
	if (hMapping == nullptr) {
		std::cerr << "fileMappingCreate - CreateFileMapping failed, fname = "
			<< PATH << std::endl;
		CloseHandle(hFile);
		return 0;
	}
	unsigned char* dataPtr = (unsigned char*)MapViewOfFile(hMapping,
		FILE_MAP_READ,
		0,
		0,
		dwFileSize); // отримуємо вказівник на ділянку пам'яті з відображенням
	if (dataPtr == nullptr) {
		std::cerr << "fileMappingCreate - MapViewOfFile failed, fname = "
			<< PATH << std::endl;
		CloseHandle(hMapping);
		CloseHandle(hFile);
		return 0;
	}
	FileMapping* mapping = (FileMapping*)malloc(sizeof(FileMapping)); // заповнюємо структуру 
	//і повертаєм вказівник на неї в якості результату
	if (mapping == nullptr) {
		std::cerr << "fileMappingCreate - malloc failed, fname = "
			<< PATH << std::endl;
		UnmapViewOfFile(dataPtr);
		CloseHandle(hMapping);
		CloseHandle(hFile);
		return 0;
	}

	mapping->hFile = hFile;
	mapping->hMapping = hMapping;
	mapping->dataPtr = dataPtr;
	mapping->fsize = (size_t)dwFileSize;

	_getch();

	//закриваємо відображення
	UnmapViewOfFile(mapping->dataPtr);
	CloseHandle(mapping->hMapping);
	CloseHandle(mapping->hFile);
	free(mapping);

	return 0;

}