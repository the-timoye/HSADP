/*
* An implementation of HSADP handling binary strings and binary SAs
* Optimizations:
    - hashes of size_t instead of strings. That is, 8bytes >> 32bytes
*/

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <optional>
#include <vector>
#include <cstring> // used by memcpy

#define __LOG std::cout
#define __LOGERR std::cerr
#define __ENDLOG std::endl
#define vector std::vector
#define pair std::pair
#define string std::string
#define ios std::ios
#define ifstream std::ifstream
#define optional std::optional
#define nullopt std::nullopt
#define unordered_map std::unordered_map
#define min std::min

#define uchar unsigned char

// reads the string as binary
uchar* readS(const string& filename, size_t& size) {
    ifstream file(filename, ios::binary | ios::ate);
    if (!file) {
        __LOGERR << "Error opening file" << filename << __ENDLOG;
        exit(1);
    };

    size = file.tellg(); // find where in the stream the cursor is at
    uchar* buffer = new uchar[size];
    file.seekg(0, ios::beg);
    file.read(reinterpret_cast<char*>(buffer), size);
    file.close();
    return buffer;
};

u_int32_t* readSA(const string& filename, size_t& size) {
    ifstream file(filename, ios::binary | ios::ate);
    if (!file) {
        __LOGERR << "Error opening file" << filename << __ENDLOG;
        exit(1);
    };

    size = file.tellg() / sizeof(u_int32_t);
    u_int32_t* buffer = new u_int32_t[size];
    file.seekg(0, ios::beg);
    file.read(reinterpret_cast<char*>(buffer), size * sizeof(u_int32_t));
    file.close();
    return buffer;
};

void printHierarchy(const unordered_map<size_t, vector<u_int32_t>>& hier, const uchar* S, int substring_length) {
    __LOG << "Tree Structure: { ";
    for (const auto& [hash, values] : hier) {
        if (!values.empty()) {
            string reconstructed_substring(reinterpret_cast<const char*>(S + values[0]), substring_length);
            __LOG << "\"" << reconstructed_substring << "\": [ ";
            for (u_int32_t v : values) __LOG << v << " ";
            __LOG << "] ";
        }
    }
    __LOG << "}" << __ENDLOG;
}

/*
creates the hierarchy that aids the querying of a string
Params:
 * S - the string to be searched
 * Q - the query string/parameter
 * l - the pruning parameter
 * suffixArray - defived from the SAIS algorithm
 * n - the length of the string S
 * m - the length/size of the query string Q
 * validIndicies - the SA to be searched (useful upon pruning)
 * level - tree level
 Returns: <vector> 
*/
optional<vector<u_int32_t>> createHierarchy(
    const uchar* S
    , const uchar* Q
    , int l
    , const u_int32_t* suffixArray
    , size_t n
    , size_t m
    , size_t SA_size
    , const optional<pair<const uchar*, vector<u_int32_t>>> valid_indicies = nullopt
    , int level = 0
) {
    __LOG << "Level: " << level << __ENDLOG;

    unordered_map<size_t, vector<u_int32_t>> hier; // LARGE: find an alternative for size_t. it costs 8bytes!!
    const u_int32_t* SA = valid_indicies ? valid_indicies->second.data() : suffixArray;
    size_t SA_count = valid_indicies ? valid_indicies->second.size() : SA_size;
    int new_l = (level > 0) ? ((m < (l+l)) ? l + (m-l) : (l+l)) : l;

    for (size_t i=0; i < SA_count; i++) {
        u_int32_t index = SA[i];
        if (index + new_l > n) continue; // ensures that there are no overheads. the number of characters in the index does not go beyond the size of the strings
        size_t hash = 0;

        memcpy(&hash, S+index, min(sizeof(size_t), (size_t)new_l));
        hier[hash].push_back(index); // LARGE: costs 56bytes!! too large
        __LOG << "Size of H: " << sizeof(hash) << __ENDLOG;

    };

    printHierarchy(hier, S, new_l);


    optional<pair<const uchar*, vector<u_int32_t>>> nextValidIndicies = nullopt;
    size_t qHash = 0;
    memcpy(&qHash, Q, min(sizeof(size_t), (size_t)new_l));

    auto it = hier.find(qHash);
    __LOG << "IT value: " << it->second[0] << __ENDLOG;
    if (it != hier.end()) {
        if (memcmp((S + it->second[0]), Q, m) == 0) return it->second;    
        nextValidIndicies = {S+it->second[0], it->second};
    };

    if(!nextValidIndicies) {
        __LOG << "No matching string found!" << __ENDLOG;
        return nullopt;
    }
    return createHierarchy(S, Q, new_l, SA, n, m, SA_size, nextValidIndicies, level+1);
};

int main () {
    size_t n, SA_size;
    uchar* S = readS("input.txt", n);
    u_int32_t* SA = readSA("output", SA_size);

    uchar Q[] = "NAN";
    size_t m = sizeof(Q)-1;
    int l = 3;

    // Call function
	clock_t start, finish;
	double  duration;
	start = clock();
    optional<vector<u_int32_t>> result = createHierarchy(S, Q, l, SA, n, m, SA_size);
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;

	std::cout << "Time: " << duration << std::endl;
    if (result) {
        for (u_int32_t index : *result) __LOG << index << __ENDLOG;
    } else {__LOG << "No Exact Match Found." << __ENDLOG;};

    delete[] S;
    delete[] SA;

    return 0;
}