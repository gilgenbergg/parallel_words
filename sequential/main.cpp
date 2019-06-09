#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <assert.h>
#include <cstring>
#include <locale>
#include <wchar.h>
#include <unordered_map>
#include <windows.h>
#include <ratio>
#include <chrono>

using namespace std;
using namespace std::chrono;

unordered_map<string, int> countWords(vector<string>* words) {
	unordered_map<string, int> res;
	if (words->size() == 0)
		return res;
	else {
		for (int i = 0; i < words->size(); i++) {
			string str;
			str = words ->at(i);
			res[str]++;
		}
		return res;
	}
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
	while (file >> word) {
		word = clearString(word);
		allWords->push_back(word);
	}
	file.close();
	return allWords;
}

vector<string>* formatData(vector<string>* in) {
	for (string& str : *in) {
		str = clearString(str);
	}
	return in;
}


int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "rus");
	cout << "Opening the file...";
	string fileName("./test.txt");
	vector<string> *allWords = fileWordByWord(fileName);
	if (allWords == nullptr) {
		cout << "No such file\n" << endl;
		delete allWords;
		return 1;
	}
	allWords = fileWordByWord(fileName); //reading the file
	allWords = formatData(allWords);
	high_resolution_clock::time_point start = high_resolution_clock::now();
	unordered_map<string, int> res = countWords(allWords);
	high_resolution_clock::time_point finish = high_resolution_clock::now();
	duration<double> timing = duration_cast<duration<double>>(finish - start);
	cout << endl;
	cout << "Время выполнения последовательной программы составляет "<< timing.count() << " сек."<< endl;
	ofstream fout("C:/parallels/words1/statistics.txt", ios_base::app);
	if (!fout.is_open()) 
		cout << "Файл не может быть открыт!\n";
	fout << timing.count() << endl;
	fout.close();
	//for (auto it : res)
		//cout <<" " << it.first << "-------> встречается: " << it.second << "раз." << endl;
	system("pause");
}