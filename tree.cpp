#include <fstream>
#include <iostream>
#include <cstring>

#include <fcntl.h>      // open
#include <unistd.h>     // read, close
#include <sys/stat.h>   // fstat
#include <sys/types.h>  // fstat
#include <cstdlib>

#define MAX_NODES 2000
#define MAX_EDGES 2000
#define MAX_INDEXES 2000

struct Edge;

struct Node {
    int index;
    int value[MAX_INDEXES];
    int indexCount = 0;
    int edgeCount = 0;
    Edge* edges[MAX_EDGES];
};

struct Edge {
    const char* label;
    int labelLength;
    Node* to;
};

struct Hierarchy {
    Node nodePool[MAX_NODES];
    int nodeIndex = 0;
};

void printHierarchy(Node* rootNode) {
    if (!rootNode) return;
    std::cout << "Root Node: ";
    for (int i = 0; i < rootNode->indexCount; ++i) {
        std::cout << rootNode->value[i] << " ";
    }
    std::cout << std::endl;

    for (int i = 0; i < rootNode->edgeCount; ++i) {
        std::cout << "    ------- ";
        std::cout.write(rootNode->edges[i]->label, rootNode->edges[i]->labelLength); // FIXED
        std::cout << "; Value Count: " << rootNode->edges[i]->to->indexCount;
        std::cout << " ------- ";

        std::cout << "Nodes Values: ";
        for (int j = 0; j < rootNode->edges[i]->to->indexCount; ++j) {
            std::cout << rootNode->edges[i]->to->value[j] << " ";
        }
        std::cout << std::endl;
    }
}

int* loadSuffixArray(const char* filename, int& length) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) return nullptr;

    struct stat st;
    if (fstat(fd, &st) != 0) {
        close(fd);
        return nullptr;
    }

    length = st.st_size / sizeof(int);
    int* SA = (int*)malloc(st.st_size);
    if (!SA) {
        close(fd);
        return nullptr;
    }

    ssize_t bytesRead = read(fd, SA, st.st_size);
    close(fd);

    if (bytesRead != st.st_size) {
        free(SA);
        return nullptr;
    }

    return SA;
}

Hierarchy* root = new Hierarchy;

Node* createNode(int value) {
    if (root->nodeIndex >= MAX_NODES) {
        std::cerr << "Error: Maximum number of nodes (" << MAX_NODES << ") exceeded!" << std::endl;
        std::exit(1); // or throw an exception
    }
    Node* node = &root->nodePool[root->nodeIndex]; 
    node->index = root->nodeIndex;
    node->indexCount = 0;   // Important: reset these, in case re-used
    node->edgeCount = 0;
    node->value[node->indexCount] = value;
    node->indexCount++;
    root->nodeIndex++;
    return node;
}

void addIndex(Node* node, int value) {
    if (node->indexCount >= MAX_INDEXES) {
        std::cerr << "Error: Maximum indexes exceeded!" << std::endl;
        std::exit(1);
    }
    node->value[node->indexCount] = value;
    node->indexCount++;
};

Edge* createEdge(const char* label, int labelLength,  Node* from, Node* to) {
    Edge* edge = new Edge;    
    edge->label = label;
    edge->labelLength = labelLength;
    from->edges[from->edgeCount] = edge;
    from->edgeCount++;    
    edge->to = to;
    return edge;
};


Node* buildHierarchy(const char* S, int level, int SAsize, int *SA, size_t n, size_t m, int l) {
    std::cout << "------------------- HIERARCHY ------------------- Level " << level << std::endl;
    std::cout << "Size of SA: " << SAsize << std::endl;


    Node* rootNode = createNode(-1); // dummy root node   
    if (level > 0) {
        rootNode->indexCount = SAsize;
        memcpy(rootNode->value, SA, rootNode->indexCount*sizeof(int));
    }


    for (int i=0; i<SAsize; ++i) { // build the hierarchy
        if ((SA[i]+l) >= n) { continue;};

        bool found = false;
        for (int j=0; j<rootNode->edgeCount; j++) {
            if (rootNode->edges[j]->labelLength == l &&
                strncmp(rootNode->edges[j]->label, S+SA[i], l) == 0) {
                found = true;
                addIndex(rootNode->edges[j]->to, SA[i]);
            }
        }

        if (!found) {
            Node* node = createNode(SA[i]);
            createEdge(S+SA[i], l, rootNode, node);
        }
    };    
    printHierarchy(rootNode); 
    return rootNode;
}

Edge* createHierarchy(const char* S, const char* Q, int *SA, int l, size_t n, size_t m, int SAsize, int level) {

    int newL = (level > 0) ? ((m < l + l) ? (l + (m - l)) : (l + l)) : l;

    Node* rootNode = buildHierarchy(S, level, SAsize, SA, n, m, newL);

    // PHASE 2: Search
    // search the hierarchy to compare edge labels with the value of Q.

    Edge* edgeFound = nullptr;

    if (rootNode->edgeCount > 0) {
        for (int i=0; i<rootNode->edgeCount; i++) {
            if (edgeFound) {break;};
            if (rootNode->edges[i]->labelLength == newL &&
                strncmp(rootNode->edges[i]->label, Q, newL) == 0) {
                edgeFound = rootNode->edges[i];
            } else {
                // root->nodePool[rootNode->edges[i]->to->index] = {};
                delete rootNode->edges[i];
                rootNode->edges[i] = nullptr;
            }
        }
    }

    if ((m > newL) && (edgeFound)) {
        std::cout << "recurse " <<std::endl;
        int* nextSA = edgeFound->to->value;
        int SASize = edgeFound->to->indexCount;
        return createHierarchy(S, Q, nextSA, newL, n, m, SASize, level+1);
    }
    
    return edgeFound;
};

int main() {

    // Reading S from file
    FILE* f = fopen("input.txt", "rb");
    fseek(f, 0, SEEK_END);
    size_t n = ftell(f);
    rewind(f);
    char* S = new char[n + 1];  // +1 for safety null if needed
    fread(S, 1, n, f);
    fclose(f);

    // Reading Q from file
    FILE* fQ = fopen("Q.txt", "rb");
    fseek(fQ, 0, SEEK_END);
    size_t m = ftell(fQ);
    rewind(fQ);
    char* Q = new char[m + 1];  // +1 for safety null if needed
    fread(Q, 1, m, fQ);
    fclose(fQ);

    int SASize;
    int* SA = loadSuffixArray("rel", SASize);
    std::cout << "------------------- SA Size -------------------" << SASize<< std::endl;

    int l = 2;
    int level = 0;

    Edge* result = createHierarchy(S, Q, SA, l, n, m, SASize, level);
    if (result) {
        std::cout << "------------------- RESULT -------------------" << result->to->indexCount << std::endl;
        printHierarchy(result->to);
    }

    delete root;
    delete[] S;
    delete[] Q;

    std::cout <<"Done! " << std::endl;
    return 0;
};