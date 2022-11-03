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
	FILE_ATTRIBUTE_NORMAL, nullptr); // ��������� ����


using namespace std;

HANDLE ghMutex;
double* threadTime;
int p;
double relProg = 0;
vector<int> vec;
double result = 0;
double sum = 0;


struct info  //���������, ��� �� ������������� ��� ������� � ������
{
	int pochtk;
	int kinec;
	int index;
	HANDLE ghMutex;
};

//�������� ������� ���� ��� ���������� ���������� ������ 
//� ������ ���� � ����
void write_mass(const char* filename) {

	FILE* f;
	f = fopen(filename, "w");
	int arr[N];
	srand(time(NULL));
	for (int i = 0; i < N; i++) {
		arr[i] = rand();
		//arr[i] = rand()%100;   //����� � ����� �� 0 �� 99
		fprintf(f, " %d", arr[i]);
	}
	fclose(f);
}
//-------------------------------------------------------------------------------------------------

//������� ��� �������� �����, ���������� ������, 
//�� ��������� ���������� ������������� �������� ������
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

	info* infoin2 = (info*)infoin;  //��������� �������� �� ��������� info
	chrono::steady_clock::time_point clock_begin = chrono::steady_clock::now();  //�������� ���� ����
	int apochtk = infoin2->pochtk;  //����������� �� ������� �������� ��������� � �������� ���� � �����
	int akinec = infoin2->kinec;   //����������� �� ���������� �������� ��������� � �������� ���� � �����
	//int sum = 0;
	for (int i = apochtk; i < akinec; i++)  //���� �� ��������� ���������
	{
		Sleep((rand() % 50 + 50));   //������
		sum += vec[i];   // ��������� ���� ��� �������� ������////////////////////////////////////////
		WaitForSingleObject(ghMutex, INFINITE);   //������ ���� �� �������� ����������
		relProg++;  //������ ��������
		ReleaseMutex(ghMutex);  //��������� �'������
	}

	chrono::steady_clock::time_point clock_end = chrono::steady_clock::now();  // �������� ���� ����
	chrono::steady_clock::duration time_span = clock_end - clock_begin;   // ������ ������, ����� ������ ���� �� �� ����
	threadTime[infoin2->index] = (double(time_span.count()) * chrono::steady_clock::period::num / chrono::steady_clock::period::den);  //��� ��������� ������
	ReleaseMutex(ghMutex);   // ��������� �'������

	ExitThread(0);//�������� � ������
}


int to_get_res()
{
	HANDLE thread[1000];   //����� ������ 
	//write_mass("mass.txt"); //���������� ����� ���������� �������
	p = 0;
	ifstream fin("mass.txt");  //��������� ����
	int i = 0;
	if (fin.is_open())  // �������� �� ���� ��������
	{
		while (!fin.eof())  //���� ���� �� ������ 
		{
			int tmp;
			fin >> tmp;
			vec.push_back(tmp); // ������� ��� � ����� � ��������(����������) �� � ����� vec
		}
	}
	fin.close(); // ��������� ����


	ghMutex = CreateMutex(NULL, FALSE, NULL); //��������� �'����� (������� �������, ��������� ����������� ��������, ��'� ��'����)
	if (ghMutex == NULL) //���� �'����� ������� ����, ����� �� ���������
	{
		printf("CreateMutex error: %d\n", GetLastError()); //�������� �������
		return 1; //��������� ��������
	}

	int krok = vec.size() / MAX_THREADS;  //����� ������� �������� ������ �� ������� ������ � ���������� ����
	int etc = vec.size() % MAX_THREADS;  // ������ �� ������
	int p = 0;
	int k = 0;

	threadTime = new double[MAX_THREADS];
	for (int i = 0; i < MAX_THREADS; i++)  // ���� �� ������� ������
	{
		int nextKrok = i < etc ? (krok + 1) : krok;  // ���������� ��������� ����(���� 1 ����� �� ������, �� �� ������� ���� ���� 1, � ���� �, �� - ��������� ���� ������� �����)
		k += nextKrok;
		info* param = new info(); //��������� �������� �� ���������
		param->index = i;  // � index �������� �������� �
		param->pochtk = p;  //� pochtk ������� �������� p
		param->kinec = k;  //� kinec ������� �������� k
		param->ghMutex = ghMutex;  //  � ghMutex ���������� �������� ghMutex


		thread[i] = CreateThread(NULL, 0, MULT, (LPVOID)param, 0, 0); // ��������� ����

		p += nextKrok;  // �������� p �������� �� nextKrok
	}
	while (relProg < vec.size())  //���� ������� �������� � ������� ����� �� ������� �������� � ����� (����������� ����� ��������)
	{
		cout << relProg / vec.size() * 100 << "% \n"; //�������� ������� �������� �� ����� ��������
		Sleep(1000);  //������ 1000 ��������
	}
	cout << "100% ";
	WaitForMultipleObjects(MAX_THREADS, thread, TRUE, INFINITE); //������ ���� �� ������ ��������� ��� ���������

	result = sum / N; //��������� ������ �����������

	for (DWORD i = 0; i < MAX_THREADS; ++i) //���� �� ��� �������� �������
	{
		CloseHandle(thread[i]); //��������� ����
	}

	CloseHandle(ghMutex); //��������� �'�����

	std::string tmp = std::to_string(result);
	char const* num_char = tmp.c_str();
	WriteFile(hFile, (LPVOID)num_char, sizeof(result), 0, NULL); //�������� ��������� � ����
	printf("\nAverage array: %f \n", result); //�������� ���������

}

struct FileMapping {
	HANDLE hFile; //����� �����
	HANDLE hMapping; //����� ����������� 
	size_t fsize; // ����� �����
	unsigned char* dataPtr; //�������� �� ������ ���'�� � ������������
};

int main() {

	if (hFile == INVALID_HANDLE_VALUE) {
		std::cerr << "fileMappingCreate - CreateFile failed, fname = "
			<< PATH << std::endl;
		return 0;
	}

	to_get_res();

	DWORD dwFileSize = GetFileSize(hFile, nullptr); // �������� ����� �����
	if (dwFileSize == INVALID_FILE_SIZE) {
		std::cerr << "fileMappingCreate - GetFileSize failed, fname = "
			<< PATH << std::endl;
		CloseHandle(hFile);
		return 0;
	}
	HANDLE hMapping = CreateFileMapping(hFile, nullptr, PAGE_READONLY,
		0, 0, L"MyMapping"); // ��������� �����������
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
		dwFileSize); // �������� �������� �� ������ ���'�� � ������������
	if (dataPtr == nullptr) {
		std::cerr << "fileMappingCreate - MapViewOfFile failed, fname = "
			<< PATH << std::endl;
		CloseHandle(hMapping);
		CloseHandle(hFile);
		return 0;
	}
	FileMapping* mapping = (FileMapping*)malloc(sizeof(FileMapping)); // ���������� ��������� 
	//� �������� �������� �� �� � ����� ����������
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

	//��������� �����������
	UnmapViewOfFile(mapping->dataPtr);
	CloseHandle(mapping->hMapping);
	CloseHandle(mapping->hFile);
	free(mapping);

	return 0;

}