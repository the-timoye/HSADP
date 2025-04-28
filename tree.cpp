#include <fstream>
#include <iostream>
#include <cstring>

#define MAX_NODES 12
#define MAX_EDGES 10
#define MAX_INDEXES 50

struct Edge;

struct Node {
    int index;
    int value[MAX_INDEXES];
    int indexCount = 0;
    int edgeCount = 0;
    Edge* edges[MAX_EDGES];
};

struct Edge {
    std::string label;
    Node* to;
};

struct Hierarchy {
    Node nodePool[MAX_NODES];
    int nodeIndex = 0;
};




void printHierarchy(Node* rootNode) {
    if (!rootNode) return;
    std::cout << "Root Node: " ;
    for (int i=0; i<rootNode->indexCount; ++i) {
        std::cout << rootNode->value[i] << " ";
    }
    std::cout << std::endl;

    for (int i=0; i<rootNode->edgeCount; ++i) {
        std::cout << "    ------- ";
        std::cout << rootNode->edges[i]->label;
        std::cout << "; Value Count: " << rootNode->edges[i]->to->indexCount;

        std::cout << "------- ";
    

        std::cout << "Nodes Values: ";
        for (int j=0; j<rootNode->edges[i]->to->indexCount; ++j) {  
           std::cout << rootNode->edges[i]->to->value[j] << " ";
        }    
        std::cout << std::endl;
    }
};

Hierarchy* root = new Hierarchy;

Node* createNode(int value) {
    if (root->nodeIndex >= MAX_NODES) {
        std::cerr << "Error: Maximum number of nodes (" << MAX_NODES << ") exceeded!" << std::endl;
        std::exit(1); // or throw an exception
    }
    Node* node = &root->nodePool[root->nodeIndex++]; 
    node->index = root->nodeIndex;
    node->indexCount = 0;   // Important: reset these, in case re-used
    node->edgeCount = 0;
    node->value[node->indexCount] = value;
    node->indexCount++;
    return node;
}

void addIndex(Node* node, int value) {
    node->value[node->indexCount] = value;
    node->indexCount++;
};

Edge* createEdge(std::string label, Node* from, Node* to) {
    Edge* edge = new Edge;    
    edge->label = label;
    from->edges[from->edgeCount] = edge;
    from->edgeCount++;    
    edge->to = to;
    return edge;
};


Node* buildHierarchy(std::string S, int level, int SAsize, int *SA, size_t n, size_t m, int l) {
    std::cout << "------------------- HIERARCHY ------------------- Level " << level << std::endl;
    std::cout << "Size of SA: " << SAsize << std::endl;


    Node* rootNode = createNode(-1); // dummy root node    
    if (level > 0) {
        rootNode->indexCount = SAsize;
        std::cout << "ppp " << rootNode->indexCount*sizeof(int) << std::endl;
        memcpy(rootNode->value, SA, rootNode->indexCount*sizeof(int));
    }


    for (int i=0; i<SAsize; ++i) { // build the hierarchy
        if ((SA[i]+l) >= n) { continue;};

        bool found = false;
        for (int j=0; j<rootNode->edgeCount; j++) {
            if (S.substr(SA[i], l) == rootNode->edges[j]->label) {
                found = true;
                addIndex(rootNode->edges[j]->to, SA[i]);
            }
        }

        if (!found) {
            Node* node = createNode(SA[i]);
            createEdge(S.substr(SA[i], l), rootNode, node);
        }
    };    
    printHierarchy(rootNode); 
    return rootNode;
}

Edge* createHierarchy(std::string S, std::string Q, int *SA, int l, size_t n, size_t m, int SAsize, int level) {

    int newL = (level > 0) ? ((m < l + l) ? (l + (m - l)) : (l + l)) : l;

    Node* rootNode = buildHierarchy(S, level, SAsize, SA, n, m, newL);

    // PHASE 2: Search
    // search the hierarchy to compare edge labels with the value of Q.

    Edge* edgeFound = nullptr;

    for (int i=0; i<rootNode->edgeCount; i++) {
        if (edgeFound) {break;};
        if (rootNode->edges[i]->label == Q.substr(0, newL)) {
            edgeFound = rootNode->edges[i];
        } else {
            // root->nodePool[rootNode->edges[i]->to->index] = {};
            delete rootNode->edges[i];
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

    std::string S = "ATGCCTGATGC$";
    std::string Q = "TGC";
    int SA[] = {11, 7, 0, 10, 3, 4, 6, 9, 2, 5, 8, 1};

    int SASize = sizeof(SA)/sizeof(SA[0]);

    int n = S.length();
    int m = Q.length();
    int l = 2;
    int level = 0;

    Edge* result = createHierarchy(S, Q, SA, l, n, m, SASize, level);
    if (result) {
        std::cout << "------------------- RESULT -------------------" << result->to->indexCount << std::endl;
        printHierarchy(result->to);
    }

    delete root;

    std::cout <<"Done! " << std::endl;
    return 0;
};