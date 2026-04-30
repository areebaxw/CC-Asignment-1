#pragma once
#include <string>
#include <vector>
#include <memory>
#include <fstream>

using namespace std;

// Each node in the parse tree:
//   - label  : the grammar symbol (NT or terminal)
//   - children: sub-trees (empty for leaves)
struct TreeNode {
    string label;
    vector<shared_ptr<TreeNode>> children;

    explicit TreeNode(const string& lbl) : label(lbl) {}
};

class Tree {
public:
    // Option 1: ASCII art box-drawing tree
    static void displayASCII(const shared_ptr<TreeNode>& root);

    // Option 2a: Preorder traversal  (root, then children left->right)
    static void displayPreorder(const shared_ptr<TreeNode>& root);

    // Option 2b: Postorder traversal (children left->right, then root)
    static void displayPostorder(const shared_ptr<TreeNode>& root);

    // Option 3: Indented text format (depth-based spaces)
    static void displayIndented(const shared_ptr<TreeNode>& root);

    // Option 4: DOT format for Graphviz
    static void displayDOT(const shared_ptr<TreeNode>& root, const string& outputFolder, int inputNum);

    // Menu: ask user which format they want and display accordingly
    static void showMenu(const shared_ptr<TreeNode>& root);

private:
    static void asciiHelper(const shared_ptr<TreeNode>& node,
                            const string& indent, bool isLast);
    static void preorderHelper(const shared_ptr<TreeNode>& node);
    static void postorderHelper(const shared_ptr<TreeNode>& node);
    static void indentedHelper(const shared_ptr<TreeNode>& node, int depth);
    static void dotHelper(const shared_ptr<TreeNode>& node, int parentId,
                          int& counter, ofstream& out);
};
