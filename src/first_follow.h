#pragma once
#include "grammar.h"
#include <map>
#include <set>
#include <string>

using namespace std;

class FirstFollow {
public:
    // Computed results
    map<string, set<string>> first;
    map<string, set<string>> follow;

    // Compute FIRST sets for every non-terminal in the grammar
    void computeFirst(const Grammar& g);

    // Compute FOLLOW sets (must call computeFirst first)
    void computeFollow(const Grammar& g);

    // Compute FIRST of an arbitrary sequence of symbols
    // (used during table construction)
    set<string> firstOfSequence(
        const vector<string>& seq,
        const Grammar& g) const;

    // Pretty-print both sets
    void printFirst()  const;
    void printFollow() const;
};
