#include <iostream>
#include <vector>
#include <fstream>
#include <queue>
#include <cstring>

class T10 {
private:

    // the children nodes of this tree node
    T10 **_children;

public:

    //set this data as given value and set all children to nullptr
    explicit T10() {
        _children = new T10 *[10];
        for (int i = 0; i < 10; i++) {
            _children[i] = nullptr;
        }
    }

    //set children as given childs
    explicit T10(T10 **childs) {
        _children = childs;
    }

    //add a child at the first blank child node of this node, return false if no blank child node found
    bool operator+=(T10 *child) {
        for (int i = 0; i < 10; i++) {
            if (_children[i] == nullptr) {
                _children[i] = child;
                return true;
            }
        }

        return false;
    }

    //set given tree's data and children to this tree's _data and _children
    T10 *operator=(const T10 *tree) {
        this->_children = tree->_children;
        return this;
    }

    //get the child node with given index
    T10 *GetChild(int index) {
        return _children[index];
    }

    //replace the child node at index with given node and return the old one
    T10 *Replace(T10 *node, int index) {
        T10 *ret = _children[index];
        _children[index] = node;
        return ret;
    }

    ~T10() {
        delete _children;
    }
};

int pow(int a, int n) {
    int sum = 1;
    for (int i = 0; i < n; i++) {
        sum *= a;
    }
    return sum;
}

void readFromFile(std::istream &istream, int *count, std::vector<char> &emptyMap) {
    char completeTreeNodesCountBytes[sizeof(int)];
    for (char &countByte : completeTreeNodesCountBytes) {
        istream >> countByte;
    }
    int emptyCount = 0;
    int completeTreeNodesCount = *((int *) (completeTreeNodesCountBytes));
    for (int i = 0; i < completeTreeNodesCount; i += 8) {
        unsigned char temp = 0;
        istream >> temp;
        for (int c = 0; c < 8; c++) {
            if (c >= completeTreeNodesCount - i) {
                break;
            }
            unsigned char node = (temp & (char) pow(2, 7 - c));
            node >>= 7 - c;
            if (node == 1) {
                emptyCount++;
            }
            emptyMap.push_back((char) node);
        }
    }
    *count = completeTreeNodesCount - emptyCount;
}

void writeToFile(T10 *tree, const char *emptyNodesMap, int completeTreeNodesCount, std::ostream &ostream) {
    std::vector<unsigned char> emptyMap;
    int nodeCount = 0;
    std::queue<T10 *> queue;
    queue.push(tree);
    while (!queue.empty()) {
        T10 *node = queue.front();
        queue.pop();
        for (int i = 0; i < 10; i++) {
            if (node->GetChild(i) != nullptr) {
                queue.push(node->GetChild(i));
            }
        }
        nodeCount++;
    }
    for (int i = 0; i < sizeof(completeTreeNodesCount); i++) {
        ostream << *(((char *) (&completeTreeNodesCount)) + i);
    }
    for (int i = 0; i < completeTreeNodesCount;) {
        unsigned char temp = 0;
        int j = 0;
        for (j = 7; j >= 0; j--, i++) {
            char temp2 = emptyNodesMap[i];
            temp2 <<= j;
            temp |= temp2;
        }
        emptyMap.push_back(temp);
    }
    for (int i = 0; i < emptyMap.size(); i++) {
        ostream << emptyMap[i];
    }
}

//use this factory to create a T10 tree
class T10Factory {
private :
    //the total sum of tree nodes in the tree
    int _treeNodesCount;

    //the bitmap of empty nodes index in the whole tree, also use level traversal order
    char *_emptyNodesMap;


    T10Factory(char *emptyNodesMap, int treeNodesCount) :
            _emptyNodesMap(emptyNodesMap),
            _treeNodesCount(treeNodesCount) {}

    //use recursive function to create all tree nodes and connect them
    T10 *_BuildTreeRecursively(int currentNodeIndex, int currentLevelIndex) {
        if (_treeNodesCount == 0) {
            return nullptr;
        }
        if (currentNodeIndex < _treeNodesCount) {
            auto *node = new T10();
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
    static T10Factory *GetFactoryInstance(char *emptyNodesMap, int treeNodesCount) {
        return new T10Factory(emptyNodesMap, treeNodesCount);
    }

    //immediately create a tree using given arguments
    static T10 *BuildTreeImmediately(char *emptyNodesMap, int treeNodeCount) {
        auto factory = T10Factory(emptyNodesMap, treeNodeCount);
        auto *ret = factory.BuildTree();
        return ret;
    }

    //create the tree use the _BuildTreeRecursively recursive function
    T10 *BuildTree() {
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


void write(const char *path, int level) {
    int count = (1 - pow(10, level)) / (1 - 10);
    char *emptyNodes = new char[count];
    memset(emptyNodes, 0, count);
    for (int i = count - 10; i < count; i++) {
        emptyNodes[i] = 1;
    }
    auto root = T10Factory::BuildTreeImmediately(emptyNodes, count - 10);
    if (path != nullptr) {
        std::ofstream ofstream;
        ofstream.open(path);
        writeToFile(root, emptyNodes, count, ofstream);
        ofstream.close();
    }
    delete root;
    delete emptyNodes;
}

void read(char *read, char *write) {
    std::ifstream ifstream;
    ifstream.open(read);
    std::vector<char> emptyNodes;
    int count = 0;
    readFromFile(ifstream, &count, emptyNodes);
    ifstream.close();
    auto root = T10Factory::BuildTreeImmediately(emptyNodes.data(), count);
    if (write != nullptr) {
        std::ofstream ofstream;
        ofstream.open(write);
        writeToFile(root, emptyNodes.data(), (int) emptyNodes.size(), ofstream);
        ofstream.close();
    }
    delete root;
}

void printHelp() {
    std::cout << "usage:" << std::endl;
    std::cout << "-h print this message" << std::endl;
    std::cout << "-w create the tree in memory" << std::endl;
    std::cout << "-l <positive integer> level of this tree, if not given then set to 7, should only use with -w"
              << std::endl;
    std::cout << "-r <file name> read the level traverse data from file and create the tree in memory" << std::endl;
    std::cout << "-o <file name> write the created tree to a file using level traverse order" << std::endl;
    std::cout << "-c <file1 name> <file2 name> compare if two trees stored in given files are identical" << std::endl;
}

int main(int argc, const char **argv) {
    if (argc == 1) {
        printHelp();
        return 0;
    }
    bool createTree = false;
    bool readTree = false;
    bool compare = false;
    bool output = false;
    bool setLevel = false;
    int level = 7;
    char *outputFile = nullptr;
    char *readFile = nullptr;
    char *compareFile1 = nullptr;
    char *compareFile2 = nullptr;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h")) {
            printHelp();
            return 0;
        }
        if (!strcmp(argv[i], "-w")) {
            createTree = true;
        } else if (!strcmp(argv[i], "-l")) {
            if (setLevel != false) {
                printHelp();
                return -1;
            }
            setLevel = true;
            level = atoi(argv[++i]);
            if (level <= 0) {
                printHelp();
                return -1;
            }
        } else if (!strcmp(argv[i], "-r")) {
            if (readFile != nullptr || readTree != false) {
                printHelp();
                return -1;
            }
            readFile = (char *) argv[++i];
            readTree = true;
        } else if (!strcmp(argv[i], "-o")) {
            if (outputFile != nullptr || output != false) {
                printHelp();
                return -1;
            }
            outputFile = (char *) argv[++i];
            output = true;
        } else if (!strcmp(argv[i], "-c")) {
            if ((compareFile1 != nullptr && compareFile2 != nullptr) || compare != false) {
                printHelp();
                return -1;
            }
            if (i + 2 > argc) {
                printHelp();
                return -1;
            }
            compareFile1 = (char *) argv[++i];
            compareFile2 = (char *) argv[++i];
            compare = true;
        } else {
            printHelp();
            return -1;
        }
    }

    if (createTree && !readTree && !compare) {
        std::cout << "creating a " << level << "-level tree" << std::endl;
        write(outputFile, level);
        std::cout << "finished" << std::endl;
    } else if (readTree && !createTree && !compare && !setLevel) {
        std::cout << "reading a tree from file " << readFile << std::endl;
        read(readFile, outputFile);
        std::cout << "finished" << std::endl;
    } else if (!readTree && !output && !createTree && compare && !setLevel) {
        std::ifstream file1, file2;
        file1.open(compareFile1);
        file2.open(compareFile2);
        bool res = compareResults(file1, file2);
        std::cout << "result is: " << (res ? "identical" : "different") << std::endl;
        file1.close();
        file2.close();
    } else {
        printHelp();
        return -1;
    }
    return 0;
}