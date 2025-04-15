
#include <iostream>

#define MAX_NODES 20
#define MAX_EDGES 10

struct Edge;

struct Node {
    int value = 0;
    int edgeCount = 0;
    Edge* edges[MAX_EDGES];
};

struct Edge {
    std::string label;
    Node* to;
};

int nodeIndex = 0;

struct Hierarchy {
    Node nodePool[MAX_NODES];
};

void printHierarchy(Node* rootNode, Hierarchy* hier) {
    if (!rootNode) return;
    std::cout << "Root Node: " << rootNode->value << std::endl;

    for (int i=0; i<rootNode->edgeCount; ++i) {
        std::cout << "    ------- ";
        std::cout << rootNode->edges[i]->label;
        std::cout << "------- ";
        std::cout << rootNode->edges[i]->to->value << std::endl;
    }
};

Hierarchy* root = new Hierarchy;

Node* createNode(int value) {
    Node* node = &root->nodePool[nodeIndex++]; // assign a fixed size to the node
    if (node) {
        node->value = value;
    };

    return node;
};

Edge* createEdge(std::string label, Node* from, Node* to) {
    Edge* edge = new Edge;
    edge->label = label;
    from->edges[from->edgeCount] = edge;
    from->edgeCount++;    
    edge->to = to;
    return edge;
};


int main() {
    // need a loop to create all the possible nodes for that level
    Node* rootNode = createNode(1234567);
    Node* childNode1 = createNode(12);
    Node* childNode2 = createNode(3);
    Node* childNode3 = createNode(4);
    Node* childNode4 = createNode(56);
    Node* childNode5 = createNode(67);

    // for each node, there has to be an edge connecting it to its parent node
    Edge* rootEdge1 = createEdge("AATTA", rootNode, childNode1);
    Edge* rootEdge2 = createEdge("AAAC", rootNode, childNode2);
    Edge* rootEdge3 = createEdge("CAAAG", rootNode, childNode3);
    Edge* rootEdge4 = createEdge("GAAC", rootNode, childNode4);
    Edge* rootEdge5 = createEdge("GAGG", rootNode, childNode5);

    // once a match is found, all other edges are deleted apart from the edge that connects the match to its root
    free(rootEdge1);
    free(rootEdge2);
    free(rootEdge3);
    free(rootEdge5);

    // create nodes for the next level
    Node* gChildNode5 = createNode(6);
    Node* gChildNode6 = createNode(7);
    // the loop is repeated until a match is found (or otherwise)
    Edge* node4Edge1 = createEdge("GA", childNode4, gChildNode5);
    Edge* node4Edge2 = createEdge("AC", childNode4, gChildNode6);

    printHierarchy(childNode4, root);

    free(node4Edge1);
    free(node4Edge2);

    free(root);
    std::cout <<"Done! " << std::endl;

    return 0;
};