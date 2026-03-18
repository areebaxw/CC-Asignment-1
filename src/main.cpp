#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "grammar.h"
#include "first_follow.h"
#include "parser.h"
#include "tree.h"
#include "error_handler.h"

using namespace std;

// ─────────────────────────────────────────────────────────────
// Tokenise a single line of input into a vector of strings
// ─────────────────────────────────────────────────────────────
static vector<string> tokeniseLine(const string& line) {
    vector<string> tokens;
    istringstream iss(line);
    string tok;
    while (iss >> tok) tokens.push_back(tok);
    return tokens;
}

// ─────────────────────────────────────────────────────────────
// main
// Usage:  ./parser <grammar_file> <input_file>
// ─────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0]
                  << " <grammar_file> <input_file>\n";
        return 1;
    }

    string grammarFile = argv[1];
    string inputFile   = argv[2];

    // ── Step 1: Load Grammar ───────────────────────────────
    Grammar g;
    if (!g.loadFromFile(grammarFile)) return 1;

    cout << "========================================\n";
    cout << "  LL(1) Parser\n";
    cout << "========================================\n";

    cout << "\n[1] Original Grammar:\n";
    g.print();

    // ── Step 2: Left Factoring ─────────────────────────────
    g.leftFactor();
    cout << "\n[2] After Left Factoring:\n";
    g.print();

    // ── Step 3: Left Recursion Removal ────────────────────
    g.removeLeftRecursion();
    cout << "\n[3] After Left Recursion Removal:\n";
    g.print();

    // ── Step 4: FIRST and FOLLOW ───────────────────────────
    FirstFollow ff;
    ff.computeFirst(g);
    ff.computeFollow(g);
    ff.printFirst();
    ff.printFollow();

    // ── Step 5: Build Parsing Table ────────────────────────
    Parser parser;
    parser.buildTable(g, ff);
    parser.printTable(g);

    if (!parser.isLL1) {
        cout << "\nWARNING: Grammar is not LL(1). "
                     "Parsing may be unreliable.\n";
    }

    // ── Step 6: Parse each input string ───────────────────
    ifstream inputStream(inputFile);
    if (!inputStream.is_open()) {
        cerr << "Cannot open input file: " << inputFile << "\n";
        return 1;
    }

    string line;
    int lineNum = 1;
    while (getline(inputStream, line)) {
        // Skip blank lines and comments
        string trimmed = line;
        while (!trimmed.empty() && (trimmed[0] == ' ' || trimmed[0] == '\t'))
            trimmed = trimmed.substr(1);
        if (trimmed.empty() || trimmed[0] == '#') { lineNum++; continue; }

        cout << "\n========================================\n";
        cout << "Parsing input " << lineNum << ": \"" << line << "\"\n";
        cout << "========================================\n";

        auto tokens = tokeniseLine(line);
        if (tokens.empty()) { lineNum++; continue; }

        ErrorHandler err;
        auto root = parser.parse(tokens, g, ff, err);

        // Show interactive parse tree menu for accepted strings
        if (root) {
            Tree::showMenu(root);
        }

        lineNum++;
    }

    return 0;
}
