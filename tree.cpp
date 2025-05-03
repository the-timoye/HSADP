#include <fstream>
#include <iostream>
#include <cstring>
#include <stdint.h>


#include <fcntl.h>      // open
#include <unistd.h>     // read, close
#include <sys/stat.h>   // fstat
#include <sys/types.h>  // fstat
#include <cstdlib>


struct Edge;

struct Node {
    int index;
    int* values;
    int indexCount = 0;
    int edgeCount = 0;
    int edgeCapacity = 0;
    Edge** edges = nullptr;
};

struct Edge {
    uint32_t label;
    uint8_t labelLength;
    Node* to;
};

struct Hierarchy {
    int nodeIndex = 0;
};

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
void printHierarchy(Node* rootNode, Hierarchy* hier) {
    if (!rootNode) return;
    
    std::cout << "Edge Count: ";
    std::cout << rootNode->edgeCount;
    std::cout << " Global Node Count: ";
    std::cout << hier->nodeIndex;
    std::cout << "; \nRoot Node: ";


    for (int i = 0; i < rootNode->indexCount; ++i) {
        std::cout << rootNode->values[i] << " ";
    }
    std::cout << std::endl;

    for (int i = 0; i < rootNode->edgeCount; ++i) {
        std::cout << "    ------- ";
        // std::cout.write(rootNode->edges[i]->label, rootNode->edges[i]->labelLength); // FIXED
        std::cout << "; Values Count: " << rootNode->edges[i]->to->indexCount;
        std::cout << " ------- ";

        std::cout << "Nodes Valuess: ";
        for (int j = 0; j < rootNode->edges[i]->to->indexCount; ++j) {
            std::cout << rootNode->edges[i]->to->values[j] << " ";
        }
        std::cout << std::endl;
    }
}
Node* createNode(int values, Hierarchy* hier, int n) {
    if (hier->nodeIndex >= n/2) {
        std::cerr << "Error: Maximum number of nodes (" << n/2 << ") exceeded!" << std::endl;
    }
    Node* node = new Node; 
    node->index = hier->nodeIndex++;
    node->indexCount = 0;   // Important: reset these, in case re-used
    node->edgeCount = 0;
    node->values = new int[n];
    node->values[node->indexCount] = values;
    node->indexCount++;
    node->edgeCapacity = 4;
    node->edges = new Edge*[node->edgeCapacity];
    return node;
}

void addIndex(Node* node, int values, int n) {
    if (node->indexCount >= (n/2)) {
        std::cout << "Maximum indexes exceeded!" << std::endl;
        node->indexCount = (n);
    }
    node->values[node->indexCount] = values;
    node->indexCount++;
};

void addEdge(Node* from, Edge* edge) {
    if (from->edgeCount >= from->edgeCapacity) {
        int newCap = from->edgeCapacity*2;
        Edge** newEdges = new Edge*[newCap];
        std::memcpy(newEdges, from->edges, from->edgeCount*sizeof(Edge*));
        delete[] from->edges;
        from->edges = newEdges;
        from->edgeCapacity = newCap;
    }
    from->edges[from->edgeCount++] = edge;
}

Edge* createEdge(int startIndex, int labelLength,  Node* from, Node* to) {
    Edge* edge = new Edge();    
    edge->label = startIndex;
    edge->labelLength = labelLength;   
    edge->to = to;
    addEdge(from, edge);
    return edge;
};


Node* buildHierarchy(const char* S, int level, int SAsize, int *SA, size_t n, size_t m, int l, Hierarchy* hier) {
    std::cout << "------------------- HIERARCHY ------------------- Level " << level << std::endl;
    std::cout << "Size of SA: " << SAsize << std::endl;


    Node* rootNode = createNode(-1, hier, n); // dummy root node   
    if (level > 0) {
        rootNode->indexCount = SAsize;
        memcpy(rootNode->values, SA, rootNode->indexCount*sizeof(int));
        delete[] SA;
    }


    for (int i=0; i<SAsize; ++i) { // build the hierarchy
        if ((SA[i]+l) >= n) { continue;};

        bool found = false;
        for (int j=0; j<rootNode->edgeCount; j++) {
            if (rootNode->edges[j]->labelLength == l &&
                strncmp(S+rootNode->edges[j]->label, S+SA[i], l) == 0) {
                found = true;
                addIndex(rootNode->edges[j]->to, SA[i], n);
            }
        }

        if (!found) {
            Node* node = createNode(SA[i], hier, n);
            createEdge(SA[i], l, rootNode, node);
        }
    };    
    return rootNode;
}

Edge* createHierarchy(const char* S, const char* Q, int *SA, int l, size_t n, size_t m, int SAsize, int level, Hierarchy* hier) {

    int newL = (level > 0) ? ((m < l + l) ? (l + (m - l)) : (l + l)) : l;

    Node* rootNode = buildHierarchy(S, level, SAsize, SA, n, m, newL, hier);

    Edge* edgeFound = nullptr;

    std::cout << "------------------- SEARCHING ------------------- Level " << level << std::endl;

    if (rootNode->edgeCount > 0) {
        for (int i=0; i<rootNode->edgeCount; i++) {
            // if (edgeFound) {
            //     delete[] rootNode->values;
            //     delete[] rootNode->edges;
            //     delete rootNode;
            //     break;
            // };
            if (rootNode->edges[i]->labelLength == newL &&
                strncmp(S+rootNode->edges[i]->label, Q, newL) == 0) {
                edgeFound = rootNode->edges[i];
            }else {
                delete[] rootNode->edges[i]->to->values;
                delete[] rootNode->edges[i]->to->edges;
                delete rootNode->edges[i]->to;
                delete rootNode->edges[i];
            }
        }
    }

    if ((m > newL) && (edgeFound)) {
        std::cout << "recurse " <<std::endl;
        int* nextSA = edgeFound->to->values;
        int SASize = edgeFound->to->indexCount;
        delete[] rootNode->values;
        delete[] rootNode->edges;
        delete rootNode;

        return createHierarchy(S, Q, nextSA, newL, n, m, SASize, level+1, hier);
    }    

    delete[] rootNode->edges;
    delete[] rootNode->values;
    delete rootNode;

    return edgeFound;
};

int main() {

    Hierarchy* hier = new Hierarchy;

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

    // char S[] = "BANANA$";
    // char Q[] = "ANA";
    // int SA[] = {6, 5, 3, 1, 0, 4, 2};

    

    // int n = 7;
    // int m = 3;
    int SASize;
    int* SA = loadSuffixArray("rel", SASize);
    std::cout << "------------------- S Size -------------------" << n << std::endl;

    int l = 3;
    int level = 0;

    clock_t start, finish;
	double  duration;
	start = clock();

    Edge* result = createHierarchy(S, Q, SA, l, n, m, SASize, level, hier);

	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;

    if (result) {
        std::cout << "------------------- RESULT ------------------- " << duration << " seconds" << std::endl;
        printHierarchy(result->to, hier);         
    }
    delete result->to;
    delete hier;
    delete[] S;
    delete[] Q;
    delete[] SA;

    std::cout <<"Done! " << std::endl;
    return 0;
};