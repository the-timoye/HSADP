#include <iostream>
#include <fstream>
#include <unordered_map>
#include <optional>
#include <vector>
#include <cstring> // used by memcpy

#define __LOG std::cout
#define __ENDLOG std::endl
#define vector std::vector
#define string std::string
#define ios std::ios
#define ifstream std::ifstream
#define nullopt std::nullopt
#define unordered_map std::unordered_map


/* TODOs:
* Change the strings to char/uchar. Unordered map may not work for the uchar, vector combination
* - Obtain their lengths/size of the values
* - Obtain the substring from S
* - 
* Change the SA type to binary
* Read S, Q, and SA as binary files
*/

unordered_map<string, vector<int>> createHierarchy(string S, string Q, vector<int> SA, int l, int level) {
    __LOG << "Level: " << level << __ENDLOG;
    unordered_map<string, vector<int>> hier;

    int n = S.length();
    int m = Q.length();
    // vector<int> _SA = valid_indicies ? valid_indicies : suffixArray;
    int new_l = (level > 0) ? ((m < (l+l)) ? l + (m-l) : (l+l)) : l;

    for (int i=0; i<(SA.size()); i++) {
        if (i + new_l > n) continue;
        string present_suffix = S.substr(SA[i], new_l);
        if (present_suffix.length() < new_l) continue;
        hier[present_suffix].push_back(SA[i]);
    }

    vector<int> _SA = {};
    for (auto it: hier) {
        if (it.first == Q) { 
            __LOG << "---------------------------- Result ----------------------------" << __ENDLOG;
            __LOG << "Match Found: ";
            hier.clear();
            hier[it.first] = it.second;
            return hier;
        };
        if (it.first == Q.substr(0, new_l)){
            _SA = it.second;
            __LOG << "Recursion" << __ENDLOG;
            break;
        };
    }
    if (_SA.empty()) {

        __LOG << "---------------------------- Result ----------------------------" << __ENDLOG;
        __LOG << "No Match Found for " << Q << __ENDLOG;
        __LOG << "Last Hierarchy Found: " << __ENDLOG;
        return hier;
    };
    return createHierarchy(S, Q, _SA, new_l, level+1);
};

int main() {
    string S = "ATGCCTGATGC$";
    string Q = "ATAT";
    int l = 2;

    vector<int> SA = {11, 7, 0, 10, 3, 4, 6, 9, 2, 5, 8, 1};
    // Call function
	clock_t start, finish;
	double  duration;
	start = clock();

    unordered_map<string, vector<int>> result = createHierarchy(S, Q, SA, l, 0);
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;

    for (auto it : result) {
        __LOG << it.first << ": ";
        for (int i=0; i<it.second.size(); i++) {
            __LOG << it.second[i] << " ";
        };
        __LOG << "; " << __ENDLOG;
    };

    __LOG << "---------------------------- Time Taken ----------------------------" << __ENDLOG;
    __LOG << duration << __ENDLOG;

    return 0;
}