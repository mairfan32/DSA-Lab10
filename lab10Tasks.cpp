
#include <iostream>
#include <vector>
#include <queue>
#include <stack>
using namespace std;


// task 1: freight convoy partitioning 

struct Container {
    int id, weight;
    Container* next;
    Container(int i, int w) : id(i), weight(w), next(nullptr) {}
};

struct Truck {
    int remaining;          // capacity left
    vector<int> ids;        // container ids loaded
};

// Try to place the container 'cur' (and all after it) onto the trucks.
bool loadContainers(Container* cur, vector<Truck>& trucks, int remainingWeight, int remainingCapacity) {
    if (cur == nullptr) return true;                    // every container placed

    // PRUNE 2: not enough total capacity left for the remaining containers
    if (remainingWeight > remainingCapacity) return false;

    for (int i = 0; i < (int)trucks.size(); i++) {
        // PRUNE 1: this truck can't fit the container
        if (trucks[i].remaining < cur->weight) continue;

        // Choose: put container on truck i
        trucks[i].remaining -= cur->weight;
        trucks[i].ids.push_back(cur->id);

        if (loadContainers(cur->next, trucks, remainingWeight - cur->weight,
                           remainingCapacity - cur->weight))
            return true;

        // Undo (backtrack)
        trucks[i].remaining += cur->weight;
        trucks[i].ids.pop_back();
    }
    return false;
}

void task1() {
    cout << "TASK 1: Freight Convoy\n";
    int W = 10;                                          // capacity of each truck
    int numTrucks = 3;

    // Build linked list: (id, weight)
    int data[][2] = { {1, 5}, {2, 4}, {3, 6}, {4, 3}, {5, 7}, {6, 5} };
    Container* head = nullptr; Container* tail = nullptr;
    int totalWeight = 0;
    for (auto& d : data) {
        Container* n = new Container(d[0], d[1]);
        totalWeight += d[1];
        if (!head) head = tail = n; else { tail->next = n; tail = n; }
    }

    // Queue of trucks
    queue<Truck> truckQueue;
    for (int i = 0; i < numTrucks; i++) truckQueue.push({W, {}});

    // Move trucks from queue into a vector so we can modify them while backtracking
    vector<Truck> trucks;
    while (!truckQueue.empty()) { trucks.push_back(truckQueue.front()); truckQueue.pop(); }

    if (loadContainers(head, trucks, totalWeight, W * numTrucks)) {
        for (int i = 0; i < (int)trucks.size(); i++) {
            cout << "Truck " << i + 1 << ": ";
            for (int id : trucks[i].ids) cout << "C" << id << " ";
            cout << "\n";
        }
    } else {
        cout << "Insufficient truck capacity.\n";
    }
    cout << endl;
}


// task 2: Undo/Redo Macro Synthesizer 

struct Op { char type; int value; };

int applyOp(int s, Op op) {
    if (op.type == '+') return s + op.value;
    if (op.type == '-') return s - op.value;
    if (op.type == '*') return s * op.value;
    return s / op.value;                                 // '/'
}

void printStack(stack<Op> st) {                          // copy so the original is untouched
    vector<Op> v;
    while (!st.empty()) { v.push_back(st.top()); st.pop(); }
    for (int i = (int)v.size() - 1; i >= 0; i--) cout << v[i].type << v[i].value << " ";
    cout << "\n";
}

void findSequences(int state, int target, int K, int Mmax,
                   vector<Op>& ops, stack<Op>& opStack, int& count) {
    if (state == target && !opStack.empty()) {           // valid sequence found
        printStack(opStack);
        count++;
    }
    if ((int)opStack.size() == K) return;                // budget used up

    for (Op op : ops) {
        int next = applyOp(state, op);
        if (next < 0 || next > Mmax) continue;           // prune invalid values

        opStack.push(op);                                // choose
        findSequences(next, target, K, Mmax, ops, opStack, count);
        opStack.pop();                                   // undo
    }
}

void task2() {
    cout << "TASK 2: Macro Synthesizer\n";
    int Sstart = 2, Starget = 9, K = 4, Mmax = 100;
    vector<Op> ops = { {'+', 3}, {'*', 2}, {'-', 5}, {'/', 2} };

    stack<Op> opStack;
    int count = 0;
    cout << "Start=" << Sstart << ", Target=" << Starget << ", K=" << K << "\n";
    findSequences(Sstart, Starget, K, Mmax, ops, opStack, count);
    if (count == 0) cout << "No valid sequence found.\n";
    cout << endl;
}


// task 3: cloud budget path finder (BST)

struct Node {
    int serviceID, cost;
    Node *left, *right;
    Node(int id, int c) : serviceID(id), cost(c), left(nullptr), right(nullptr) {}
};

Node* insert(Node* root, int id, int cost) {             // normal BST insert (by cost)
    if (!root) return new Node(id, cost);
    if (cost < root->cost) root->left = insert(root->left, id, cost);
    else                   root->right = insert(root->right, id, cost);
    return root;
}

// Part A: downward paths that sum to B
void printPath(vector<Node*>& path, int start) {
    for (int i = start; i < (int)path.size(); i++)
        cout << "S" << path[i]->serviceID << "($" << path[i]->cost << ") ";
    cout << "\n";
}

void findPaths(Node* node, int B, vector<Node*>& path) {
    if (!node) return;
    path.push_back(node);                                // go down

    // Check every path that ENDS at this node (starting from any ancestor)
    int sum = 0;
    for (int i = (int)path.size() - 1; i >= 0; i--) {
        sum += path[i]->cost;
        if (sum == B) printPath(path, i);
    }

    findPaths(node->left, B, path);
    findPaths(node->right, B, path);
    path.pop_back();                                     // backtrack
}

// Part B: subsets of the whole BST that sum to B 
void inorder(Node* node, vector<Node*>& sorted) {        // BST inorder = sorted by cost
    if (!node) return;
    inorder(node->left, sorted);
    sorted.push_back(node);
    inorder(node->right, sorted);
}

void findSubsets(vector<Node*>& sorted, int index, int B, int sum, vector<Node*>& chosen) {
    if (sum == B && !chosen.empty()) {                   // valid subset
        for (Node* n : chosen) cout << "S" << n->serviceID << "($" << n->cost << ") ";
        cout << "\n";
    }
    for (int i = index; i < (int)sorted.size(); i++) {
        // PRUNE: costs are in increasing order, so if this one overshoots B,
        // every node after it (the "right side") overshoots too.
        if (sum + sorted[i]->cost > B) break;

        chosen.push_back(sorted[i]);                     // choose
        findSubsets(sorted, i + 1, B, sum + sorted[i]->cost, chosen);
        chosen.pop_back();                               // undo
    }
}

void task3() {
    cout << "TASK 3: Cloud Budget\n";
    Node* root = nullptr;
    int costs[][2] = { {1, 50}, {2, 30}, {3, 70}, {4, 20}, {5, 40}, {6, 60}, {7, 80} };
    for (auto& c : costs) root = insert(root, c[0], c[1]);

    int B = 120;
    cout << "Target budget B = " << B << "\n\n";

    cout << "Part A - downward paths:\n";
    vector<Node*> path;
    findPaths(root, B, path);

    cout << "\nPart B - subsets of the BST:\n";
    vector<Node*> sorted, chosen;
    inorder(root, sorted);
    findSubsets(sorted, 0, B, 0, chosen);
    cout << endl;
}

int main() {
    task1();
    task2();
    task3();
    return 0;
}