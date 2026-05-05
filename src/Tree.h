#ifndef TREE_H
#define TREE_H

#include "Token.h"
#include "GraphvizConverter.h"
#include <vector>
#include <fstream>

using namespace std;

// Parse tree node
class TreeNode {
public:
    Symbol symbol;
    vector<TreeNode*> children;
    int nodeId;
    
    TreeNode(const Symbol& sym);
    ~TreeNode();
    
    void addChild(TreeNode* child);
    bool isLeaf() const;
    
    string toString(int indent = 0, const string& prefix = "", bool isLast = true) const;
};

// Parse tree
class ParseTree {
private:
    TreeNode* root;
    int nodeCounter;
    
    void assignNodeIdsRecursive(TreeNode* node);
    void generateGraphvizRecursive(TreeNode* node, ofstream& file) const;
    string getNodeLabel(TreeNode* node) const;
    
public:
    ParseTree(const Symbol& rootSymbol);
    ~ParseTree();
    
    TreeNode* getRoot() const { return root; }
    void setRoot(TreeNode* newRoot);
    
    void addChild(TreeNode* parent, TreeNode* child);
    void displayTree() const;
    void saveTreeAsImage(const string& filename) const;
    
    int getNextNodeId();
};

#endif
