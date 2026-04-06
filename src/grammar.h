#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>

using namespace std;

// Grammar stores productions as:
//   productions["Expr"] = { {"Term","ExprPrime"}, {"id"}, ... }
// ntOrder keeps the original ordering of non-terminals (important for
// indirect left-recursion removal).

class Grammar {
public:
    // Map: NonTerminal -> list of alternatives (each alternative = list of symbols)
    map<string, vector<vector<string>>> productions;

    // Ordered list of non-terminals (insertion order)
    vector<string> ntOrder;

    string startSymbol;

    // Load grammar from file
    bool loadFromFile(const string& filename);

    // Step 1: Left Factoring
    void leftFactor();

    // Step 2: Left Recursion Removal (handles both direct and indirect)
    void removeLeftRecursion();

    // Utility: check if a symbol is a non-terminal
    bool isNonTerminal(const string& sym) const;

    // Print the grammar nicely
    void print() const;

    // Display transformed grammar as DOT format
    void displayTransformationDOT(const string& outputFolder = "") const;

private:
    // Helper: find longest common prefix among a set of productions
    vector<string> longestCommonPrefix(
        const vector<vector<string>>& alts) const;

    // Helper: generate a unique new non-terminal name like "ExprPrime"
    string freshName(const string& base);

    // Helper: direct left recursion removal for one non-terminal
    void removeDirect(const string& nt);
};
