#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <cstring>
#include <locale>
#include <thread>
#include <conio.h>
#include <wchar.h>
#include <unordered_map>
#include <windows.h>
#include <ratio>
#include <chrono>

using namespace std;
using namespace std::chrono;
unordered_map <string, int> res;

HANDLE hSemaphore;
const char lpSemaphoreName[] = "MySemaphore";

void merge(unordered_map <string, int>& mapB) {
	for (pair<string, int> itB : mapB) {
		if (WaitForSingleObject(hSemaphore, 30000) == WAIT_FAILED)
			return;
		auto search = res.find(itB.first);
		if (search != res.end()) {
			res[itB.first] = res[itB.first] + itB.second;
		}
		else {
			res [itB.first] = itB.second;
		}
		if (hSemaphore != NULL)
			ReleaseSemaphore(hSemaphore, 1, NULL);
	}
}

void countWords(int threadID, int start, int end, vector<string>* words) {
	if (words->size() == 0)
		return;
	if (start < 0 || end > words->size() - 1 || end < start)
		return;
	if (end - start + 1 < 1)
		return;
	unordered_map <string, int> myMap;
	for (int i = start; i <= end; i++) {
		string str;			
			str = words->at(i);			
			myMap[str]++;
		}
	high_resolution_clock::time_point startMerge = high_resolution_clock::now();
	merge(myMap);
	high_resolution_clock::time_point finishMerge = high_resolution_clock::now();
	duration<double> mergeTiming = (finishMerge - startMerge);
	cout << mergeTiming.count() << endl;
	return;
}

bool MYiswalpha(char ch) {
	return (((int)ch<0 && (int)ch > -65) || ((int)ch > 64
		&& (int)ch < 91) || ((int)ch > 96 && (int)ch < 123));
}

char MYtowlower(char in) {
	if ((int)in >= 192 && (int)in <= 223)
		return char((int)in - 32);
	else
		return in;
}

string clearString(string& str) {
	string newStr;
	for (char& ch : str) {
		if (MYiswalpha(ch)) {
			ch = static_cast<char>(tolower(ch));
			newStr.push_back(ch);
		}
	}
	return newStr;
}
vector<string>* fileWordByWord(string fileName) {
	ifstream file(fileName);
	if (!file.is_open())
		return nullptr;
	string word;
	vector <string>* allWords = new vector <string>;
	allWords->resize(1);
	while (file >> word) {
		word = clearString(word);
		allWords->push_back(word);
	}
	file.close();
	return allWords;
}
void formatData(vector<string>* in) {
	for (string& str : *in) {
		str = clearString(str);
	}
}

int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "rus");
	cout << "Opening the file...";
	const int threads_amount = 4;
	hSemaphore = CreateSemaphore(NULL, 1, 1, lpSemaphoreName);
	if (hSemaphore == NULL) {
		cout << "Semaphore was not created!" << endl;
		return FALSE;
	}
	string fileName("./test.txt");
	vector<string> *allWords = fileWordByWord(fileName);
	if (allWords == nullptr) {
		cout << "No such file\n" << endl;
		delete allWords;
		return 1;
	}
	formatData(allWords);
	
	std::thread * threads[threads_amount];
	int size = allWords->size();
	int start = 0;
	int step = size / threads_amount;
	int mod = size % threads_amount;
	if (mod != 0) {
		step++;
	}
	int end = 0;
	high_resolution_clock::time_point startTime = high_resolution_clock::now();
	for (int i = 0; i < threads_amount; i++) {
		start = end;
		if (i == threads_amount - 1)
			end = size - 1;
		else
			end += step;
		if (start != 0)
			start += 1;
		threads[i] = new std::thread(&countWords, i, start, end, std::ref(allWords));
	}

	for (int i = 0; i < threads_amount; i++) {
		threads[i]->join();
	}
	//finish of formatting
	high_resolution_clock::time_point finishTime = high_resolution_clock::now();
	duration<double> timing = (finishTime - startTime);     //timing of formatting (in seconds)
	cout << endl;
	cout << "Время выполнения программы с использованием потоков составляет " << timing.count() << " сек." << endl;
	CloseHandle(hSemaphore);
	ofstream fout("C:/parallels/words2/statistics.txt", ios_base::app);
	if (!fout.is_open())
		cout << "Файл не может быть открыт!\n";
	fout << timing.count() << endl;
	fout.close();
	
	//for (auto it : res)
		//cout << " " << it.first << "-------> встречается: " << it.second << "раз." << endl;
	getchar();
}
