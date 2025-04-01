#include <iostream>
#include <vector>
#include <unordered_map>
#include <optional>

#define _LOG std::cout
#define _ENDLOG std::endl
#define _vector std::vector
#define _optional std::optional
#define _unorderedMap std::unordered_map
#define _pair std::pair
#define _STR std::string
#define _NULLOPT std::nullopt

// Function to create a hierarchical suffix tree and search for Q
_optional<_vector<int>> createHierarchy(const _STR& S, const _STR& Q, int l, 
                                      const _vector<int>& SA, 
                                      const _optional<_pair<_STR, _vector<int>>>& valid_indices = _NULLOPT, 
                                      int level = 0) {
    _LOG << "Level: " << level << _ENDLOG;
    
    _unorderedMap<_STR, _vector<int>> hier;
    const _vector<int>& suffix_array = valid_indices ? valid_indices->second : SA;

    int new_l = (level > 0) ? ((Q.length() < l + l) ? (l + (Q.length() - l)) : (l + l)) : l;

    // Build hierarchy tree
    for (int index : suffix_array) {
        _LOG << index << _ENDLOG;

        if ((index + new_l) > S.length()) continue;  // Avoid out-of-bounds access
        _LOG << index << _ENDLOG;
        _STR present_suffix = S.substr(index, new_l);
        hier[present_suffix].push_back(index);
    }

    // // Optional: Print tree structure
    _LOG << "Tree Structure: { ";
    for (const auto& [key, values] : hier) {
        _LOG << "\"" << key << "\": [ ";
        for (int v : values) _LOG << v << " ";
        _LOG << "] ";
    }
    _LOG << "}" << _ENDLOG;

    // Find valid indices for the next recursion
    _optional<_pair<_STR, _vector<int>>> next_valid_indices = _NULLOPT;
    for (const auto& [key, values] : hier) {
        // if (key != Q) {hier.erase(key);} // delete key to reduce hierarchy size and reduce space
        if (key == Q) return values;  // Found exact match
        if (key == Q.substr(0, new_l)) next_valid_indices = {key, values};
    }

    // No matching indices found
    if (!next_valid_indices) {
        _LOG << "Last tree: ";
           // Optional: Print tree structure
        _LOG << "Tree Structure: { ";
        for (const auto& [key, values] : hier) {
            _LOG << "\"" << key << "\": [ ";
            for (int v : values) _LOG << v << " ";
            _LOG << "] ";
        }
        _LOG << "}" << _ENDLOG;
        _LOG << "No matching _STRs for this Query.\n";
        return _NULLOPT;
    }

    return createHierarchy(S, Q, new_l, SA, next_valid_indices, level + 1);
}


// -- TODO: --
/*
    -- read SA from binary file
    -- the SA must already have been created, with the output in binary file format
    -- how do we access the string
*/
