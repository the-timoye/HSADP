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
    uint32_t* values;
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

Node* createNode(uint32_t values, Hierarchy* hier, int n) {
    Node* node = new Node(); 
    node->index = hier->nodeIndex++;
    node->indexCount = 0;   // Important: reset these, in case re-used
    node->edgeCount = 0;
    node->values = new uint32_t[1];
    node->values[node->indexCount] = values;
    node->indexCount++;
    node->edgeCapacity = 1;
    node->edges = new Edge*[node->edgeCapacity];
    return node;
}

void addIndex(Node* node, uint32_t values, int n) {
    uint32_t *newValues = new uint32_t[node->indexCount+1];
    std::memcpy(newValues, node->values, (node->indexCount)*sizeof(uint32_t));
    delete[] node->values;
    node->values = newValues;
    node->values[node->indexCount] = values;
    node->indexCount++;
};

void addEdge(Node* from, Edge* edge) {
    if (from->edgeCount >= from->edgeCapacity) {
        int newCap = from->edgeCapacity+1;
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


Node* buildHierarchy(const char* S, int level, int SAsize, uint32_t *SA, size_t n, size_t m, int l, Hierarchy* hier) {
    std::cout << "------------------- HIERARCHY ------------------- Level " << level << std::endl;


    Node* rootNode = createNode(-1, hier, n); // dummy root node   
    if (level > 0) {
        rootNode->indexCount = SAsize;
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

int createHierarchy(const char* S, const char* Q, uint32_t *SA, int l, size_t n, size_t m, int SAsize, int level, Hierarchy* hier) {

    int newL = (level > 0) ? ((m < l + l) ? (l + (m - l)) : (l + l)) : l;

    Node* rootNode = buildHierarchy(S, level, SAsize, SA, n, m, newL, hier);

    int newIndexCount = 0;
    uint32_t* nextValues = nullptr;

    if (rootNode->edgeCount > 0) {

        for (int i=0; i<rootNode->edgeCount; i++) {
            if (rootNode->edges[i]->labelLength == newL &&
                strncmp(S+rootNode->edges[i]->label, Q, newL) == 0) {
                    newIndexCount = rootNode->edges[i]->to->indexCount;
                    nextValues = new uint32_t[newIndexCount];
                    std::memcpy(nextValues, rootNode->edges[i]->to->values, (newIndexCount)*sizeof(uint32_t));
                delete[] rootNode->edges[i]->to->values;
                delete[] rootNode->edges[i]->to->edges;
                delete rootNode->edges[i]->to;
                delete rootNode->edges[i]; 
            }
        }
    }

    if ((m > newL) && (nextValues)) {
        std::cout << "recurse " << sizeof(nextValues)/sizeof(uint32_t) <<std::endl;

        delete[] rootNode->edges;
        delete[] rootNode->values;
        delete rootNode;
        

        return createHierarchy(S, Q, nextValues, newL, n, m, newIndexCount, level+1, hier);
    }    
    delete[] rootNode->edges;
    delete[] rootNode->values;
    delete rootNode;
    for (int i=0; i<newIndexCount; i++) {
        std::cout << nextValues[i] << std::endl;
    }
    return 0;
};

int main() {

    Hierarchy* hier = new Hierarchy;

    // // Reading S from file
    FILE* f = fopen("input.txt", "rb");
    fseek(f, 0, SEEK_END);
    size_t n = ftell(f);
    rewind(f);
    char* S = new char[n + 1];  // +1 for safety null if needed
    fread(S, 1, n, f);
    fclose(f);

    //Reading Q from file
    FILE* fQ = fopen("Q.txt", "rb");
    fseek(fQ, 0, SEEK_END);
    size_t m = ftell(fQ);
    rewind(fQ);
    char* Q = new char[m + 1];  // +1 for safety null if needed
    fread(Q, 1, m, fQ);
    fclose(fQ);

    std::ifstream safs("rell", std::ios::binary);
    safs.seekg(0, std::ios::end);
    int SA_len = safs.tellg() / sizeof(uint32_t);
    safs.seekg(0, std::ios::beg);
    uint32_t* SA = new uint32_t[SA_len];
    safs.read((char*)SA, SA_len * sizeof(uint32_t));
    safs.close();

    std::cout << "------------------- S Size -------------------" << n << std::endl;

    int l = 6;
    int level = 0;

    clock_t start, finish;
	double  duration;
	start = clock();
    
    createHierarchy(S, Q, SA, l, n, m, SA_len, level, hier);

	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
    
    std::cout << "------------------- RESULT ------------------- " << duration << " seconds" << std::endl;

    // delete result->to;  

    // separate build from search  
    delete hier;
    delete[] S;
    delete[] Q;
    delete[] SA;

    std::cout <<"Done! " << std::endl;
    return 0;
};