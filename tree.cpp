#include <fstream>
#include <iostream>
#include <cstring>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdlib>

size_t totalMemoryUsed = 0;
size_t nodeCount = 0;
size_t edgeCount = 0;
size_t valueArrayBytes = 0;
size_t edgeArrayBytes = 0;
size_t edgeStructBytes = 0;


struct Edge;

struct Node {
    int index;
    uint32_t* values;
    int indexCount = 0;
    int edgeCount = 0;
    int edgeCapacity = 0;
    Edge** edges = nullptr;
};

struct Edge {
    uint64_t label;
    uint8_t labelLength;
    Node* to;
};

int nodeIndex = 0;

// ----------------------------------------------
// Helper: FNV-1a hash for arbitrary prefix length
// ----------------------------------------------
uint64_t hashPrefix(const char* S, int start, int l, size_t n) {
    const uint64_t FNV_prime = 1099511628211u;
    const uint64_t FNV_offset_basis = 14695981039346656037u;
    uint64_t hash = FNV_offset_basis;

    for (int i = 0; i < l; ++i) {
        if (start + i >= n) break;
        hash ^= static_cast<uint8_t>(S[start + i]);
        hash *= FNV_prime;
    }

    return hash;
}

void writeToCSV(std::string filename, int level, int l, float duration, int result, int edges=1, int totalMemory=0 ) {
    std::ofstream file_out;
    file_out.open(filename, std::ios::app);
    if (file_out.is_open()) {
        file_out << l << ", " << level << ", " << duration << ", " << filename << ", " << result <<", " << edges <<", " << totalMemory<< std::endl;
        file_out.close();
    } else {
        std::cerr << "Error opening file: " << filename << std::endl;
    }
}

Node* createNode(uint32_t value, int n) {
    Node* node = new Node(); 
    node->index = nodeIndex++;
    node->indexCount = 0;
    node->edgeCount = 0;
    node->values = new uint32_t[1];
    node->values[node->indexCount++] = value;
    node->edgeCapacity = 1;
    node->edges = new Edge*[node->edgeCapacity];
    return node;
}

void addIndex(Node* node, uint32_t value, int n) {
    uint32_t* newValues = new uint32_t[node->indexCount + 1];
    std::memcpy(newValues, node->values, node->indexCount * sizeof(uint32_t));
    delete[] node->values;
    node->values = newValues;
    node->values[node->indexCount++] = value;
};

void addEdge(Node* from, Edge* edge) {
    if (from->edgeCount >= from->edgeCapacity) {
        int newCap = from->edgeCapacity + 1;
        Edge** newEdges = new Edge*[newCap];
        std::memcpy(newEdges, from->edges, from->edgeCount * sizeof(Edge*));
        delete[] from->edges;
        from->edges = newEdges;
        from->edgeCapacity = newCap;
        edgeArrayBytes += sizeof(Edge*) * (newCap - from->edgeCapacity);
    }
    from->edges[from->edgeCount++] = edge;
}

Edge* createEdge(uint64_t packedPrefix, int labelLength, Node* from, Node* to) {
    Edge* edge = new Edge();    
    edge->label = packedPrefix;
    edge->labelLength = labelLength;   
    edge->to = to;
    addEdge(from, edge);
    return edge;
}

Node* buildHierarchy(const char* S, const char* Q, int level, int SAsize, uint32_t* SA, size_t n, size_t m, int l) {
    std::cout << "------------------- HIERARCHY ------------------- Level " << level << std::endl;

    Node* rootNode = createNode(-1, n);
    if (level > 0) rootNode->indexCount = SAsize;
    char edgefound = 0;

    uint64_t queryPrefix = hashPrefix(Q, 0, l, m);

    for (int i = 0; i < SAsize; ++i) {
        if ((SA[i] + l) >= n) continue;

        uint64_t packedPrefix = hashPrefix(S, SA[i], l, n);
        bool found = false;
        for (int j = 0; j < rootNode->edgeCount; j++) {
            if (rootNode->edges[j]->labelLength == l &&
                rootNode->edges[j]->label == packedPrefix) {
                found = true;
                addIndex(rootNode->edges[j]->to, SA[i], n);
                valueArrayBytes += sizeof(uint32_t);
                break;
            }
            if (rootNode->edges[j]->label == queryPrefix) {
                edgefound = 1;
            }
        }
        if (edgefound == 1 && !found) {
            break;
            return rootNode;
        }
        if (!found) {
            Node* node = createNode(SA[i], n);
            valueArrayBytes += sizeof(uint32_t);
            createEdge(packedPrefix, l, rootNode, node);
            edgeStructBytes += sizeof(Edge*);
        }
    }
    return rootNode;
}

int createHierarchy(const char* S, const char* Q, uint32_t* SA, int l, size_t n, size_t m, int SAsize, int level) {
    int newL = (level > 0) ? ((m < l + l) ? (l + (m - l)) : (l + l)) : l;

    clock_t build_start = clock();
    Node* rootNode = buildHierarchy(S, Q, level, SAsize, SA, n, m, newL);
    clock_t build_finish = clock();
    double duration = (double)(build_finish - build_start) / CLOCKS_PER_SEC;

    int newIndexCount = 0;
    uint32_t* nextValues = nullptr;

    if (rootNode->edgeCount > 0) {
        uint64_t queryPrefix = hashPrefix(Q, 0, newL, m);
        for (int i = 0; i < rootNode->edgeCount; i++) {
            if (rootNode->edges[i]->label == queryPrefix) {
                newIndexCount = rootNode->edges[i]->to->indexCount;
                nextValues = new uint32_t[newIndexCount];
                std::memcpy(nextValues, rootNode->edges[i]->to->values, newIndexCount * sizeof(uint32_t));
            }
            delete[] rootNode->edges[i]->to->values;
            delete[] rootNode->edges[i]->to->edges;
            delete rootNode->edges[i]->to;
            delete rootNode->edges[i];
        }
    }

    delete[] rootNode->edges;
    delete[] rootNode->values;
    delete rootNode;

    writeToCSV("ch21D.csv", level, newL, duration, newIndexCount, rootNode->edgeCount, totalMemoryUsed + valueArrayBytes + edgeArrayBytes + edgeStructBytes);

    if ((m > newL) && (nextValues)) {
        return createHierarchy(S, Q, nextValues, newL, n, m, newIndexCount, level + 1);
    }
    std::cout << "Index count: " << newIndexCount << std::endl;
    
    delete[] nextValues;
    return 0;
}

int main() {
    FILE* f = fopen("datasets/ch21.fa", "rb");
    fseek(f, 0, SEEK_END);
    size_t n = ftell(f);
    rewind(f);
    char* S = new char[n + 1];
    fread(S, 1, n, f);
    fclose(f);

    FILE* fQ = fopen("Q.txt", "rb");
    fseek(fQ, 0, SEEK_END);
    size_t m = ftell(fQ);
    rewind(fQ);
    char* Q = new char[m + 1];
    fread(Q, 1, m, fQ);
    fclose(fQ);

    std::ifstream safs("suffixArrays/ch21", std::ios::binary);
    safs.seekg(0, std::ios::end);
    int SA_len = safs.tellg() / sizeof(uint32_t);
    safs.seekg(0, std::ios::beg);
    uint32_t* SA = new uint32_t[SA_len];
    safs.read((char*)SA, SA_len * sizeof(uint32_t));
    safs.close();

    int l = 4;
    int level = 0;

    clock_t start = clock();
    createHierarchy(S, Q, SA, l, n, m, SA_len, level);
    clock_t finish = clock();

    double duration = (double)(finish - start) / CLOCKS_PER_SEC;
    writeToCSV("ch21D.csv", 0, l, duration, 0, totalMemoryUsed + valueArrayBytes + edgeArrayBytes + edgeStructBytes);

    std::cout << "------------------- RESULT ------------------- " << duration << " seconds" << std::endl;
    std::cout << "------------------- MEMORY ------------------- " << totalMemoryUsed + valueArrayBytes + edgeArrayBytes + edgeStructBytes << " bytes" << std::endl;

    delete[] S;
    delete[] Q;
    delete[] SA;

    std::cout << "Done!" << std::endl;
    return 0;
}
