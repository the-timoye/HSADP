/* Hierarchical Suffix Array with Dynamic Pruning
* Uses a hash table as the hierarchy
* if a match is found, returns the tree branch
* If no match is found, returns the last hierarchy constructed
*/
#include <iostream>
#include <cstdint> // for the uints
#include <cstring>
#include <fstream>


#define MAX_BUCKETS 4096 // 2^12 buckets for hash indexing
#define MAX_MATCHES 64 // SA indicies per bucket
#define MAX_RESULTS 1024

#define __LOG std::cout
#define __ENDLOG std::endl

#define uchar unsigned char

struct Bucket {
    uint32_t count;
    uint32_t indicies[MAX_MATCHES];
};

struct Result {
    uint32_t count;
    uint32_t indicies[MAX_RESULTS];
};

uint32_t hash_function(const uchar* data, int length) {
    const uint32_t fnv_prime = 16777619u;
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; ++i) {
        hash ^= data[i];
        hash *= fnv_prime;
    }
    return hash % MAX_BUCKETS;
};

Result createHierarchy(uchar* S, uchar* Q, int l, uint32_t* SA, int n, int m, Result& results, uint32_t level= 0, uint32_t depth = 0, uint32_t* active_SA = nullptr, int activeSALength = 0) {
    __LOG << "Level: " << level << __ENDLOG;
    // TODO: make bucket size dynamic. It is expected to reduce per iteration;
    Bucket buckets[MAX_BUCKETS];
    const uint32_t* activeSA = active_SA ? active_SA : SA;
    int SASize = active_SA ? activeSALength : n;

    int new_l = (level > 0) ? ((m < l + l) ? (l + (m - l)) : (l + l)) : l;

    for (int i=0; i<SASize; ++i) {
        uint32_t idx = activeSA[i];
        if (idx + new_l > n) continue;

        uint32_t hash = hash_function(S+idx, new_l);

        // how can we set a fixed size for the maximum number of buckets that can be created?
        //    this will also measure the reduction in hierarchy size
        Bucket& bucket = buckets[hash];

        bucket.indicies[bucket.count++] = idx;
    }

    uint32_t target_hash = hash_function(Q, new_l);
    Bucket& matchBucket = buckets[target_hash]; // doesx

    bool partialFound = false;
    static uint32_t next_SA[MAX_MATCHES]; // needed?? TODO: Reduce the 
    int next_len = 0;

    for (uint32_t i = 0; i < matchBucket.count; ++i) {
        uint32_t position = matchBucket.indicies[i];
        if (memcmp(S + position, Q, new_l) == 0) {
            next_SA[next_len++] = position;
            partialFound = true;
            break;
        }
    }
    if (partialFound) {
        return createHierarchy(S, Q, new_l, SA, n, m, results, level + 1, depth + 1, next_SA, next_len);
    }

    for (uint32_t i=0; i<matchBucket.count; ++i) {
        uint32_t position = matchBucket.indicies[i];
        if (position + m <= (uint32_t)n && memcmp(S + position, Q, m) == 0) {
            if (results.count < MAX_RESULTS)
                results.indicies[results.count++] = position;
        }
    }
    return results;
}


int main() {
    std::ifstream fs("input.txt", std::ios::binary);
    std::ifstream safs("output", std::ios::binary);

    fs.seekg(0, std::ios::end);
    int n = fs.tellg();
    fs.seekg(0, std::ios::beg);

    uchar* S = new uchar[n];
    fs.read((char*)S, n);
    fs.close();

    safs.seekg(0, std::ios::end);
    int SA_len = safs.tellg() / sizeof(uint32_t);
    safs.seekg(0, std::ios::beg);

    uint32_t* SA = new uint32_t[SA_len];
    safs.read((char*)SA, SA_len * sizeof(uint32_t));
    safs.close();

    uchar Q[] = "ANAA";
    Result matches = {};
    createHierarchy(S, Q, 2, SA, n, sizeof(Q) -1, matches);

    if (matches.count > 0) {
        __LOG << "Matches: " << __ENDLOG;
        for (uint32_t i=0; i<matches.count; ++i) {
            __LOG << matches.indicies[i] << " ";
        }
        __LOG << __ENDLOG;
    } else {
        __LOG << "No match found:" << __ENDLOG;
    }

    return 0;
}