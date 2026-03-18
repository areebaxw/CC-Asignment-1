#include "first_follow.h"
#include <iostream>
#include <iomanip>

using namespace std;

// ─────────────────────────────────────────────────────────────
// computeFirst
// Iterative fixpoint: keep going until no new symbol is added.
// ─────────────────────────────────────────────────────────────
void FirstFollow::computeFirst(const Grammar& g) {
    // Initialise empty sets for every non-terminal
    for (const auto& nt : g.ntOrder) {
        first[nt]; // creates empty set if not present
    }

    bool changed = true;
    while (changed) {
        changed = false;

        for (const auto& nt : g.ntOrder) {
            auto& F = first[nt];
            const auto& alts = g.productions.at(nt);

            for (const auto& alt : alts) {
                // Production: nt -> alt[0] alt[1] ... alt[k]

                if (alt.empty()) continue;

                // Special case: epsilon production
                if (alt.size() == 1 && alt[0] == "epsilon") {
                    if (F.insert("epsilon").second) changed = true;
                    continue;
                }

                // Walk through each symbol in the alternative
                bool allCanBeEps = true;
                for (const auto& sym : alt) {
                    if (sym == "epsilon") {
                        // treat as epsilon symbol
                        if (F.insert("epsilon").second) changed = true;
                        break;
                    }

                    if (!g.isNonTerminal(sym)) {
                        // sym is a terminal: add it, then stop
                        if (F.insert(sym).second) changed = true;
                        allCanBeEps = false;
                        break;
                    } else {
                        // sym is a non-terminal: add FIRST(sym) - {epsilon}
                        for (const auto& s : first[sym]) {
                            if (s != "epsilon") {
                                if (F.insert(s).second) changed = true;
                            }
                        }
                        // if sym cannot derive epsilon, stop
                        if (first[sym].find("epsilon") == first[sym].end()) {
                            allCanBeEps = false;
                            break;
                        }
                    }
                }
                // If every symbol in the alt can derive epsilon, add epsilon
                if (allCanBeEps) {
                    if (F.insert("epsilon").second) changed = true;
                }
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────
// firstOfSequence  – FIRST of a sequence of symbols (alpha)
// ─────────────────────────────────────────────────────────────
set<string> FirstFollow::firstOfSequence(
        const vector<string>& seq,
        const Grammar& g) const {
    set<string> result;

    if (seq.empty()) { result.insert("epsilon"); return result; }

    bool allEps = true;
    for (const auto& sym : seq) {
        if (sym == "epsilon") {
            result.insert("epsilon");
            break;
        }
        if (!g.isNonTerminal(sym)) {
            result.insert(sym);
            allEps = false;
            break;
        }
        // sym is NT
        auto it = first.find(sym);
        if (it != first.end()) {
            for (const auto& s : it->second) {
                if (s != "epsilon") result.insert(s);
            }
            if (it->second.find("epsilon") == it->second.end()) {
                allEps = false;
                break;
            }
        } else {
            allEps = false;
            break;
        }
    }
    if (allEps) result.insert("epsilon");
    return result;
}

// ─────────────────────────────────────────────────────────────
// computeFollow
// ─────────────────────────────────────────────────────────────
void FirstFollow::computeFollow(const Grammar& g) {
    // Initialise empty sets
    for (const auto& nt : g.ntOrder) {
        follow[nt];
    }

    // Start symbol gets $
    follow[g.startSymbol].insert("$");

    bool changed = true;
    while (changed) {
        changed = false;

        for (const auto& nt : g.ntOrder) {
            const auto& alts = g.productions.at(nt);

            for (const auto& alt : alts) {
                // For every symbol B in the alt that is a non-terminal:
                for (size_t i = 0; i < alt.size(); i++) {
                    const string& B = alt[i];
                    if (!g.isNonTerminal(B)) continue;

                    // beta = everything after B in this production
                    vector<string> beta(alt.begin() + i + 1,
                                                   alt.end());

                    // Add FIRST(beta) - {epsilon} to FOLLOW(B)
                    auto fb = firstOfSequence(beta, g);
                    for (const auto& s : fb) {
                        if (s != "epsilon") {
                            if (follow[B].insert(s).second) changed = true;
                        }
                    }

                    // If epsilon in FIRST(beta), add FOLLOW(nt) to FOLLOW(B)
                    if (fb.count("epsilon")) {
                        for (const auto& s : follow[nt]) {
                            if (follow[B].insert(s).second) changed = true;
                        }
                    }
                }
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────
// printFirst
// ─────────────────────────────────────────────────────────────
void FirstFollow::printFirst() const {
    cout << "\n--- FIRST Sets ---\n";
    for (const auto& entry : first) {
        cout << setw(16) << left << entry.first << " : { ";
        for (const auto& s : entry.second) cout << s << " ";
        cout << "}\n";
    }
}

// ─────────────────────────────────────────────────────────────
// printFollow
// ─────────────────────────────────────────────────────────────
void FirstFollow::printFollow() const {
    cout << "\n--- FOLLOW Sets ---\n";
    for (const auto& entry : follow) {
        cout << setw(16) << left << entry.first << " : { ";
        for (const auto& s : entry.second) cout << s << " ";
        cout << "}\n";
    }
}
