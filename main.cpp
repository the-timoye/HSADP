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
    ifstream file(filename, ios::binary);

    if (!file){
        std::cerr << "Error opening file: " << filename << endl;
        exit(1);
    }

    file.seekg(0, ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, ios::beg);

    if (fileSize % sizeof(int) != 0) {
        cerr << "Invalid file format! " << endl;
        exit(1);
    };

    size_t numElements = fileSize/sizeof(int);
    vector<int> SA(numElements);

    file.read(reinterpret_cast<char*>(SA.data()), fileSize);

    file.close();
    return SA;
}

int main() {
    string S = "AATTCCCCCGACTGATTTAACCGGTTAAAGCCTTACC";
    string Q = "TTAA";
    // vector<int> SA = {11, 7, 0, 10, 3, 4, 6, 9, 2, 5, 8, 1};
    int l = 3;  // Pruning parameter

    vector<int> SA = readSuffixArrayBinary("output");

    cout << endl;
    

    // Call function
	clock_t start, finish;
	double  duration;
	start = clock();

    optional<vector<int>> result = createHierarchy(S, Q, l, SA);
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;

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