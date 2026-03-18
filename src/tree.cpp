#include "tree.h"
#include <iostream>
#include <fstream>
#include <cstdlib>   // system()
#include <sstream>

using namespace std;

// ═════════════════════════════════════════════════════════════
// OPTION 1 — ASCII art (box-drawing style)
// ═════════════════════════════════════════════════════════════

void Tree::asciiHelper(const shared_ptr<TreeNode>& node,
                        const string& indent, bool isLast) {
    cout << indent;
    cout << (isLast ? "+-- " : "|-- ");
    cout << node->label << "\n";

    string newIndent = indent + (isLast ? "    " : "|   ");
    for (size_t i = 0; i < node->children.size(); i++) {
        asciiHelper(node->children[i], newIndent,
                    i + 1 == node->children.size());
    }
}

void Tree::displayASCII(const shared_ptr<TreeNode>& root) {
    if (!root) { cout << "(empty tree)\n"; return; }
    cout << "\n--- Parse Tree (ASCII Art) ---\n";
    // Print root label first, then its children
    cout << root->label << "\n";
    for (size_t i = 0; i < root->children.size(); i++) {
        asciiHelper(root->children[i], "",
                    i + 1 == root->children.size());
    }
    cout << "\n";
}

// ═════════════════════════════════════════════════════════════
// OPTION 2a — Preorder traversal
// Visit: root  →  children (left to right)
// ═════════════════════════════════════════════════════════════

void Tree::preorderHelper(const shared_ptr<TreeNode>& node) {
    cout << node->label << "  ";
    for (const auto& child : node->children)
        preorderHelper(child);
}

void Tree::displayPreorder(const shared_ptr<TreeNode>& root) {
    if (!root) { cout << "(empty tree)\n"; return; }
    cout << "\n--- Preorder Traversal ---\n";
    preorderHelper(root);
    cout << "\n\n";
}

// ═════════════════════════════════════════════════════════════
// OPTION 2b — Postorder traversal
// Visit: children (left to right)  →  root
// ═════════════════════════════════════════════════════════════

void Tree::postorderHelper(const shared_ptr<TreeNode>& node) {
    for (const auto& child : node->children)
        postorderHelper(child);
    cout << node->label << "  ";
}

void Tree::displayPostorder(const shared_ptr<TreeNode>& root) {
    if (!root) { cout << "(empty tree)\n"; return; }
    cout << "\n--- Postorder Traversal ---\n";
    postorderHelper(root);
    cout << "\n\n";
}

// ═════════════════════════════════════════════════════════════
// OPTION 3 — Indented text format
// Each level adds 4 spaces of indentation
// ═════════════════════════════════════════════════════════════

void Tree::indentedHelper(const shared_ptr<TreeNode>& node, int depth) {
    // Print depth * 4 spaces, then the label
    for (int i = 0; i < depth; i++) cout << "    ";
    cout << node->label << "\n";
    for (const auto& child : node->children)
        indentedHelper(child, depth + 1);
}

void Tree::displayIndented(const shared_ptr<TreeNode>& root) {
    if (!root) { cout << "(empty tree)\n"; return; }
    cout << "\n--- Indented Text Format ---\n";
    indentedHelper(root, 0);
    cout << "\n";
}

// ═════════════════════════════════════════════════════════════
// OPTION 4 — DOT format for Graphviz
// Writes output/tree_N.dot and runs: dot -Tpng -> output/tree_N.png
// ═════════════════════════════════════════════════════════════

void Tree::dotHelper(const shared_ptr<TreeNode>& node,
                     int parentId, int& counter, ofstream& out) {
    int myId = counter++;

    // Sanitise label for DOT (replace double-quotes)
    string lbl = node->label;
    for (char& c : lbl) {
        if (c == '"') c = '\'';
    }

    out << "    node" << myId << " [label=\"" << lbl << "\"];\n";
    if (parentId >= 0)
        out << "    node" << parentId << " -> node" << myId << ";\n";

    for (const auto& child : node->children)
        dotHelper(child, myId, counter, out);
}

void Tree::displayDOT(const shared_ptr<TreeNode>& root) {
    if (!root) { cout << "(empty tree)\n"; return; }

    // Auto-number files: tree1.dot, tree2.dot, ...
    static int fileNum = 0;
    fileNum++;

    // Make sure output/ folder exists
    system("if not exist output mkdir output");

    // Build file paths
    ostringstream dotPath, pngPath;
    dotPath << "output/tree" << fileNum << ".dot";
    pngPath << "output/tree" << fileNum << ".png";

    // Write the .dot file
    ofstream out(dotPath.str());
    if (!out.is_open()) {
        cout << "  ERROR: cannot create " << dotPath.str() << "\n";
        return;
    }

    out << "digraph ParseTree {\n";
    out << "    node [shape=ellipse, fontname=\"Monospace\"];\n";
    int counter = 0;
    dotHelper(root, -1, counter, out);
    out << "}\n";
    out.close();

    cout << "  DOT file written: " << dotPath.str() << "\n";

    // Try to run Graphviz dot to produce a PNG
    string cmd = "dot -Tpng " + dotPath.str() + " -o " + pngPath.str();
    int ret = system(cmd.c_str());

    if (ret == 0) {
        cout << "  PNG  file written: " << pngPath.str() << "\n";
    } else {
        cout << "  (Graphviz not found or failed.)\n";
        cout << "  Install Graphviz from https://graphviz.org/download/\n";
        cout << "  Then run manually:  " << cmd << "\n";
    }
    cout << "\n";
}

// ═════════════════════════════════════════════════════════════
// showMenu — interactive menu, loops until user exits
// ═════════════════════════════════════════════════════════════

void Tree::showMenu(const shared_ptr<TreeNode>& root) {
    int choice = 0;
    do {
        cout << "\n+-------------------------------+\n";
        cout << "|     Parse Tree Display Menu   |\n";
        cout << "+-------------------------------+\n";
        cout << "|  1. ASCII Art (box-drawing)   |\n";
        cout << "|  2. Preorder Traversal        |\n";
        cout << "|  3. Postorder Traversal       |\n";
        cout << "|  4. Indented Text Format      |\n";
        cout << "|  5. DOT Format (Graphviz)     |\n";
        cout << "|  0. Continue to next input    |\n";
        cout << "+-------------------------------+\n";
        cout << "  Enter choice: ";
        cin >> choice;

        switch (choice) {
            case 1: displayASCII(root);     break;
            case 2: displayPreorder(root);  break;
            case 3: displayPostorder(root); break;
            case 4: displayIndented(root);  break;
            case 5: displayDOT(root);       break;
            case 0: break;
            default: cout << "  Invalid choice. Try again.\n"; break;
        }
    } while (choice != 0);
}
