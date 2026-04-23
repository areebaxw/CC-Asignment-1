#include "Tree.h"
#include <iostream>
#include <cstdio>

// TreeNode Implementation
TreeNode::TreeNode(const Symbol& sym) : symbol(sym), nodeId(-1) {}

TreeNode::~TreeNode() {
    for (TreeNode* child : children) {
        delete child;
    }
}

void TreeNode::addChild(TreeNode* child) {
    children.push_back(child);
}

bool TreeNode::isLeaf() const {
    return children.empty();
}

string TreeNode::toString(int indent) const {
    string result = string(indent, ' ') + symbol.toString() + "\n";
    for (TreeNode* child : children) {
        result += child->toString(indent + 2);
    }
    return result;
}

// ParseTree Implementation
ParseTree::ParseTree(const Symbol& rootSymbol) : nodeCounter(0) {
    root = new TreeNode(rootSymbol);
    root->nodeId = nodeCounter++;
}

ParseTree::~ParseTree() {
    delete root;
}

void ParseTree::assignNodeIdsRecursive(TreeNode* node) {
    if (!node) return;
    node->nodeId = nodeCounter++;
    for (TreeNode* child : node->children) {
        assignNodeIdsRecursive(child);
    }
}

void ParseTree::setRoot(TreeNode* newRoot) {
    if (!newRoot) return;
    delete root;
    root = newRoot;
    nodeCounter = 0;
    assignNodeIdsRecursive(root);
}

void ParseTree::addChild(TreeNode* parent, TreeNode* child) {
    if (parent && child) {
        child->nodeId = nodeCounter++;
        parent->addChild(child);
    }
}

void ParseTree::displayTree() const {
    cout << "\n=== Parse Tree ===" << endl;
    if (root) {
        cout << root->toString();
    }
}

string ParseTree::getNodeLabel(TreeNode* node) const {
    return node->symbol.toString();
}

void ParseTree::generateGraphvizRecursive(TreeNode* node, ofstream& file) const {
    if (!node) return;
    
    string nodeLabel = getNodeLabel(node);
    string nodeId = "node" + to_string(node->nodeId);
    
    // Create node
    file << "  " << nodeId << " [label=\"" << nodeLabel << "\"";
    if (node->isLeaf()) {
        file << ", shape=oval";
    } else {
        file << ", shape=box";
    }
    file << "];" << endl;
    
    // Create edges to children
    for (TreeNode* child : node->children) {
        string childId = "node" + to_string(child->nodeId);
        file << "  " << nodeId << " -> " << childId << ";" << endl;
    }
    
    // Recursively process children
    for (TreeNode* child : node->children) {
        generateGraphvizRecursive(child, file);
    }
}

void ParseTree::saveTreeAsImage(const string& filename) const {
    // Create temporary .dot file
    string dotFilename = filename.substr(0, filename.find_last_of(".")) + ".dot";
    
    ofstream file(dotFilename);
    if (!file.is_open()) {
        cerr << "Error: Cannot create graphviz file " << dotFilename << endl;
        return;
    }
    
    file << "digraph ParseTree {" << endl;
    file << "  rankdir=TB;" << endl;
    file << "  node [fontname=\"Arial\", fontsize=10];" << endl;
    
    if (root) {
        generateGraphvizRecursive(root, file);
    }
    
    file << "}" << endl;
    file.close();
    
    // Convert to PNG. Clean up temporary dot file regardless of conversion result.
    const bool converted = GraphvizConverter::convertToPNG(dotFilename, filename);
    if (!converted) {
        remove(dotFilename.c_str());
    }
}

int ParseTree::getNextNodeId() {
    return nodeCounter++;
}
