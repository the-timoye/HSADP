#include <string>
#include <iostream>
#include <time.h>
#include <vector>
#include <unordered_map>
#include <optional>
#include <fstream>


using namespace std;

optional<vector<int>> createHierarchy(const string& S, const string& Q, int l, 
    const vector<int>& SA, 
    const optional<pair<string, vector<int>>>& valid_indices = nullopt, 
    int level = 0);

vector<int> readSuffixArrayBinary(const string& filename) {
    vector<int> SA;
    ifstream file(filename, ios::binary);

    if (!file) {
        cerr << "Error opening file: " << filename << endl;
        exit(1);
    }

    int32_t index;  // 4-byte integer
    while (file.read(reinterpret_cast<char*>(&index), sizeof(int32_t))) {
        SA.push_back(index);
    }

    file.close();
    return SA;
}

int main() {
    string S = "ATGCCTGATGC";
    string Q = "CCTA";
    // vector<int> SA = {11, 7, 0, 10, 3, 4, 6, 9, 2, 5, 8, 1};
    int l = 2;  // Pruning parameter

    vector<int> SA = readSuffixArrayBinary("output");

    cout << endl;
    

    // Call function
	clock_t start, finish;
	double  duration;
	start = clock();

    optional<vector<int>> result = createHierarchy(S, Q, l, SA);
	finish = clock();
	duration = (double)(finish - start) / 1000;

	std::cout << "Time: " << duration << std::endl;

    // Print final result
    if (result) {
        cout << "\nMatching String Found at indices: ";
        for (int index : *result) cout << index << " ";
        cout << endl;
    } else {
        cout << "\nNo exact match found." << endl;
    }

    return 0;
}