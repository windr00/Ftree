#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <queue>
#include <cstring>

template<class C>
class T10 {
private:

    // the stored data value of this tree node
    C *_data;

    // the children nodes of this tree node
    T10 **_children;

public:

    //set this data as given value and set all children to nullptr
    explicit T10(C *data) : _data(data) {
        _children = new T10<C> *[10];
        for (int i = 0; i < 10; i++) {
            _children[i] = nullptr;
        }
    }

    //set this data as given value and set children as given childs
    explicit T10(C *data, T10<C> **childs) : _data(data) {
        _children = childs;
    }

    //add a child at the first blank child node of this node, return false if no blank child node found
    bool operator+=(T10<C> *child) {
        for (int i = 0; i < 10; i++) {
            if (_children[i] == nullptr) {
                _children[i] = child;
                return true;
            }
        }

        return false;
    }

    //set given tree's data and children to this tree's _data and _children
    T10<C> *operator=(const T10<C> *tree) {
        this->_data = tree->_data;
        this->_children = tree->_children;
        return this;
    }

    //get the reference of _data object
    C *GetData() {
        return _data;
    }

    //get the child node with given index
    T10<C> *GetChild(int index) {
        return _children[index];
    }

    //replace the child node at index with given node and return the old one
    T10<C> *Replace(T10<C> *node, int index) {
        T10 *ret = _children[index];
        _children[index] = node;
        return ret;
    }

    ~T10() {
        delete _children;
    }
};

//print the data of every node of the given tree using DFS
template<class C>
void print(T10<C> *tree) {
    if (tree == nullptr) {
        return;
    }
    std::cout << *tree->GetData() << std::endl;
    for (int i = 0; i < 10; i++) {
        print(tree->GetChild(i));
    }
}

template<class C>
void readFromFile(std::istream &istream, std::vector<C> &vector, std::vector<char> &emptyMap) {
    char countBytes[sizeof(int)];
    for (char &countByte : countBytes) {
        istream >> countByte;
    }
    for (int i = 0; i < *((int *) (countBytes)); i++) {
        char temp = 0;
        istream >> temp;
        for (int c = 0; c < 8; c++) {
            char node = (temp & (char) pow(2, c));
            emptyMap.push_back((node >> 7 - c));
        }
    }
    int size = sizeof(C);
    for (int i = 0; i < *((int *) (countBytes)); i++) {
        char temp[size] = {0};
        for (int j = 0; j < size; j++) {
            istream >> temp[j];
        }
        vector.push_back(*((long long *) temp));
    }
}

template<class C>
void writeToFile(T10<C> *tree, std::ostream &ostream) {
    std::vector<char> emptyMap;
    int nodeCount = 0;
    int shiftCount = 1;
    std::vector<C> levelTraversalData;
    std::queue<T10<C> *> queue;
    queue.push(tree);
    while (!queue.empty()) {
        T10<C> *node = queue.front();
        levelTraversalData.push_back(*node->GetData());
        queue.pop();
        for (int i = 0; i < 10; i++) {
            char flag = 1;
            if (node->GetChild(i) != nullptr) {
                queue.push(node->GetChild(i));
                flag = 0;
            }
            if (nodeCount % 8 == 0) {
                emptyMap.push_back(0);
            }
            emptyMap[nodeCount / 8] |= flag;
            emptyMap[nodeCount / 8] <<= shiftCount++;
            shiftCount = shiftCount == 8 ? 1 : shiftCount;
        }
        if (shiftCount <= 7) {
            emptyMap[nodeCount / 8] <<= 8 - shiftCount;
        }
    }
    for (int i = 0; i < sizeof(nodeCount); i++) {
        ostream << *(((char *) (&nodeCount)) + i);
    }
    for (int i = 0; i < emptyMap.size(); i++) {
        ostream << emptyMap[i];
    }
    for (int i = 0; i < nodeCount; i++) {
        long long data = levelTraversalData[i];
        for (int c = 0; c < sizeof(C); c++) {
            ostream << ((char *) (&data) + c);
        }
    }

}

//use this factory to create a T10 tree
template<class C>
class T10Factory {
private :
    //the total sum of tree nodes in the tree
    int _treeNodesCount;

    //data to be stored in the tree, this array is in level traversal order
    C *_allData;

    //the bitmap of empty nodes index in the whole tree, also use level traversal order
    char *_emptyNodesMap;


    T10Factory(C *allData, char *emptyNodesMap, int treeNodesCount) :
            _allData(allData),
            _emptyNodesMap(emptyNodesMap),
            _treeNodesCount(treeNodesCount) {}

    //use recursive function to create all tree nodes and connect them
    T10<C> *_BuildTreeRecursively(int currentNodeIndex, int currentLevelIndex) {
        if (_treeNodesCount == 0) {
            return nullptr;
        }
        if (currentNodeIndex < _treeNodesCount) {
            auto *node = new T10<C>(_allData + currentNodeIndex);
            int restNodesCount = _treeNodesCount - currentNodeIndex - 1;
            restNodesCount = restNodesCount > 10 ? 10 : restNodesCount;
            for (int i = 0; i < restNodesCount; i++) {
                int nextNodeIndex = (currentNodeIndex * 10) + i + 1;
                int nextLevelIndex = currentLevelIndex + 1;
                if (nextNodeIndex < _treeNodesCount && _emptyNodesMap[nextNodeIndex] == 0) {
                    node->Replace(_BuildTreeRecursively(nextNodeIndex, nextLevelIndex), i);
                }
            }
            return node;
        }
        return nullptr;
    }

public:

    //get an instance of factory, you can manually call BuildTree() later
    static T10Factory *GetFactoryInstance(C *allData, char *emptyNodesMap, int treeNodesCount) {
        return new T10Factory<C>(allData, emptyNodesMap, treeNodesCount);
    }

    //immediately create a tree using given arguments
    static T10<C> *BuildTreeImmediately(C *allData, char *emptyNodesMap, int treeNodeCount) {
        auto factory = T10Factory<C>(allData, emptyNodesMap, treeNodeCount);
        auto *ret = factory.BuildTree();
        return ret;
    }

    //create the tree use the _BuildTreeRecursively recursive function
    T10<C> *BuildTree() {
        return this->_BuildTreeRecursively(0, 0);
    }

};


bool compareResults(std::istream &istream1, std::istream &istream2) {
    while (istream1.eof() || istream2.eof()) {
        if ((istream1.eof() && !istream2.eof()) || (!istream1.eof() && istream2.eof())) {
            std::cout << "false";
            return false;
        }
        char a;
        char b;
        istream1 >> a;
        istream2 >> b;
        if (a != b) {
            std::cout << "false";
            return false;
        }
    }
    return true;
}

void printHelp() {
    std::cout << "usage: -hwroc <filename>" << std::endl;
    std::cout << "-h print this message" << std::endl;
    std::cout << "-w create the tree in memory" << std::endl;
    std::cout << "-r read the level traverse data from file and create the tree in memory" << std::endl;
    std::cout << "-o write the created tree to a file using level traverse order" << std::endl;
}

void test() {
    int count = (1 - pow(10, 7)) / (1 - 10);
    long long *data = new long long[count - 10];
    char *emptyNodes = new char[count];
    memset(emptyNodes, 0, count);
    for (int i = count - 10; i < count; i++) {
        emptyNodes[i] = 1;
    }
    for (int i = 0; i < count - 10; i++) {
        data[i] = i;
    }
    auto root = T10Factory<long long>::BuildTreeImmediately(data, emptyNodes, count - 10);
    std::ofstream ofstream;
    ofstream.open("/home/windr/Desktop/1.txt");
    writeToFile(root, ofstream);
    delete emptyNodes;
    delete data;
}

int main(int argc, const char **argv) {
//    if (argc == 1)
//    const char * op = argv[1];
//    const char * file = argv[2];
    test();
    return 0;
}