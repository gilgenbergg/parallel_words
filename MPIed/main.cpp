#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <cstring>
#include <locale>
#include <unistd.h>
#include <wchar.h>
#include <unordered_map>
#include <ratio>
#include <chrono>
#include <openmpi/mpi.h>
#include <algorithm>
#include <jsoncpp/json/json.h>

using namespace Json;
using namespace std;
using namespace std::chrono;

vector<string> allWords;
MPI_Status status;

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

vector<string> fileWordByWord(string fileName) {
    vector <string> allWords;
    ifstream file(fileName);
    if (!file.is_open())
        return allWords;
    string word;
    allWords.resize(1);
    while (file >> word) {
        word = clearString(word);
        allWords.push_back(word);
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
    MPI_Init(&argc, &argv);
    setlocale(LC_ALL, "rus");
    int procs_amount;
    int rank;
    int *words_amount = NULL; //amount of words for each proc
    MPI_Comm_size(MPI_COMM_WORLD, &procs_amount);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int size = 0;
    int mod = 0;
    int step = 0;
    high_resolution_clock::time_point startTime;
    string fileName("./test.txt");
    allWords = fileWordByWord(fileName);
    if (allWords.empty()) {
        cout << "No such file\n" << endl;
        return 1;
    }
    formatData(&allWords);
    MPI_Barrier(MPI_COMM_WORLD);
    size = allWords.size();
    startTime = high_resolution_clock::now();

    words_amount = new int[size]; //number of words

    step = size / procs_amount;
    mod = size % procs_amount;
    int start = 0;
    int end = 0;
    if (mod != 0)
        step++;
    for (int i = 0; i < procs_amount; i++) {
        start = rank*step;
        end = min((rank+1)*step, size);
        words_amount[i] = step;
    }
    MPI_Barrier(MPI_COMM_WORLD);

    map<string, int> resMap;
    map<string, int> procMap;

    string cur_word = "";
    int cur_count = 0;
    int oldNum, newNum = 0;
    //map of each proc
    for (int i=start; i<end; i++) {
        if (start == 0)
            start = start + 1;
        cur_word = allWords[i]; // take word
        if (procMap.find(cur_word) != procMap.end()) { //if word is already added to map, update its number of occurrence
            oldNum = procMap.find(cur_word)->second; //get old count
            newNum = oldNum + 1;
            procMap.find(cur_word)->second = newNum; //update the count
        }
        else { // word is not found in the map, add it
            procMap.insert(make_pair(cur_word, 1));
        }
    }

    Json::Value jsonedMap;
    for (auto&& it : procMap) {
        jsonedMap[it.first] = it.second;
    }
    Json::StyledWriter writer;
    const string output = writer.write(jsonedMap);

    int str_portion = output.length();
    if (rank != 0) {
        MPI_Send(&str_portion, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(output.c_str(), str_portion, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    }
    if (rank == 0) {
        map<string, int> resMap;
        string cur_word = "";
        int cur_count = 0;
        int oldAmount, newAmount = 0;

        Json::CharReaderBuilder readerBuilder;

        for (int i = 1; i < procs_amount; i++) {
            MPI_Recv(&str_portion, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
            char *buf = new char[str_portion];
            MPI_Recv(buf, str_portion, MPI_CHAR, i, 0, MPI_COMM_WORLD, &status);

            Json::Value parsed;
            auto *b = readerBuilder.newCharReader();
            std::string err;
            b->parse(buf, buf + str_portion, &parsed, &err);
            Json::Value::const_iterator pit;
            for (pit = parsed.begin(); pit != parsed.end(); ++pit) {
                cur_word = pit.key().asString();
                if (resMap.find(cur_word) !=
                    resMap.end()) { //if word is already added to map, update its number of occurrences
                    oldAmount = resMap.find(cur_word)->second; //get amount
                    newAmount = oldAmount + std::stoi((*pit).asString());
                    resMap.find(cur_word)->second = newAmount; //update the amount
                } else { // word is not found in the map, add it
                    resMap.insert(make_pair(cur_word, cur_count));
                }
            }
            delete[] buf;
        }

        high_resolution_clock::time_point finishTime = high_resolution_clock::now();
        duration<double> timing = (finishTime - startTime);
        cout << endl;
        auto it = resMap.begin(); //create iterator to move along the map
        while (it != resMap.end()) { //while its not the end of the map
        cout << it->first << " :: " << it->second << endl; //write key :: value
        it++;
        }
        cout << "Time of program execution with MPI is " << timing.count << " sec." << endl;
        cout<<resMap.size()<<endl;
        cout<<allWords.size()<<endl;
    }
    delete[] words_amount;
    MPI_Finalize();
}
