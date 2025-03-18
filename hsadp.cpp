#include <iostream>
#include <vector>
#include <unordered_map>
#include <optional>

using namespace std;

// Function to create a hierarchical suffix tree and search for Q
optional<vector<int>> createHierarchy(const string& S, const string& Q, int l, 
                                      const vector<int>& SA, 
                                      const optional<pair<string, vector<int>>>& valid_indices = nullopt, 
                                      int level = 0) {
    cout << "Level: " << level << endl;
    
    unordered_map<string, vector<int>> hier;
    const vector<int>& suffix_array = valid_indices ? valid_indices->second : SA;

    int new_l = (level > 0) ? ((Q.length() < l + l) ? (l + (Q.length() - l)) : (l + l)) : l;

    // Build hierarchy tree
    for (int index : suffix_array) {
        if (index + new_l > S.length()) continue;  // Avoid out-of-bounds access
        string present_suffix = S.substr(index, new_l);
        hier[present_suffix].push_back(index);
    }

    // Optional: Print tree structure
    cout << "Tree Structure: { ";
    for (const auto& [key, values] : hier) {
        cout << "\"" << key << "\": [ ";
        for (int v : values) cout << v << " ";
        cout << "] ";
    }
    cout << "}" << endl;

    // Find valid indices for the next recursion
    optional<pair<string, vector<int>>> next_valid_indices = nullopt;
    for (const auto& [key, values] : hier) {
        // if (key != Q) {hier.erase(key);} // delete key to reduce hierarchy size and reduce space
        if (key == Q) return values;  // Found exact match
        if (key == Q.substr(0, new_l)) next_valid_indices = {key, values};
    }

    // No matching indices found
    if (!next_valid_indices) {
        cout << "Last tree: ";
           // Optional: Print tree structure
        cout << "Tree Structure: { ";
        for (const auto& [key, values] : hier) {
            cout << "\"" << key << "\": [ ";
            for (int v : values) cout << v << " ";
            cout << "] ";
        }
        cout << "}" << endl;
        cout << "No matching strings for this Query.\n";
        return nullopt;
    }

    return createHierarchy(S, Q, new_l, SA, next_valid_indices, level + 1);
}


