#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>

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
// Create output directory if it doesn't exist
// ─────────────────────────────────────────────────────────────
static void createOutputDir(const string& dirPath) {
    #ifdef _WIN32
        string cmd = "if not exist \"" + dirPath + "\" mkdir \"" + dirPath + "\"";
    #else
        string cmd = "mkdir -p \"" + dirPath + "\"";
    #endif
    system(cmd.c_str());
}

// ─────────────────────────────────────────────────────────────
// Get next available output folder number
// ─────────────────────────────────────────────────────────────
static int getNextOutputFolder() {
    int folderNum = 1;
    while (true) {
        string dirName = "output/" + to_string(folderNum);
        ifstream test(dirName + "/dummy.txt");
        if (!test.good()) break;
        folderNum++;
    }
    return folderNum;
}

// ─────────────────────────────────────────────────────────────
// main
// Usage (Mode 1 - with arguments):  ./parser <grammar_file> <input_file>
// Usage (Mode 2 - interactive):     ./parser
// ─────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    string grammarFile, inputFile;

    // ── Mode 1: Command-line arguments ─────────────────────
    if (argc == 3) {
        grammarFile = argv[1];
        inputFile   = argv[2];
    } 
    // ── Mode 2: Interactive menu ───────────────────────────
    else if (argc == 1) {
        cout << "========================================\n";
        cout << "  LL(1) Parser - Interactive Mode\n";
        cout << "========================================\n\n";

        cout << "Enter grammar file name (from input/ folder): ";
        cin >> grammarFile;
        grammarFile = "input/" + grammarFile;

        cout << "Enter input strings file name (from input/ folder): ";
        cin >> inputFile;
        inputFile = "input/" + inputFile;
    }
    else {
        cerr << "Usage: " << argv[0] << " [grammar_file input_file]\n";
        cerr << "If no arguments provided, interactive mode will be used.\n";
        return 1;
    }

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

    // ── Create output subfolder ─────────────────────────────
    int outputNum = getNextOutputFolder();
    string outputFolder = "output/" + to_string(outputNum);
    createOutputDir(outputFolder);

    // Display grammar transformation and FIRST/FOLLOW sets
    g.displayTransformationDOT(outputFolder);
    ff.displayFirstFollowDOT(outputFolder);

    // ── Step 5: Build Parsing Table ────────────────────────
    Parser parser;
    parser.buildTable(g, ff);
    parser.printTable(g);
    parser.displayTableDOT(g, outputFolder);

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

        // Export parsing trace to DOT file
        parser.displayParseLtraceDOT(outputFolder);

        // Export parse tree to DOT file
        if (root) {
            Tree::displayDOT(root, outputFolder, lineNum);
        }

        lineNum++;
    }

    cout << "\n========================================\n";
    cout << "All visualizations saved to: " << outputFolder << "/\n";
    cout << "========================================\n";

    return 0;
}
