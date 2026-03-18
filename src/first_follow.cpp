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
    for (size_t i = 0; i < g.ntOrder.size(); i++) {
        const string& nt = g.ntOrder[i];
        first[nt]; // creates empty set if not present
    }

    bool changed = true;
    while (changed) {
        changed = false;

        for (size_t i = 0; i < g.ntOrder.size(); i++) {
            const string& nt = g.ntOrder[i];
            auto& F = first[nt];
            const auto& alts = g.productions.at(nt);

            for (size_t altIdx = 0; altIdx < alts.size(); altIdx++) {
                const vector<string>& alt = alts[altIdx];
                // Production: nt -> alt[0] alt[1] ... alt[k]

                if (alt.empty()) continue;

                // Special case: epsilon production
                if (alt.size() == 1 && alt[0] == "epsilon") {
                    if (F.insert("epsilon").second) changed = true;
                    continue;
                }

                // Walk through each symbol in the alternative
                bool allCanBeEps = true;
                for (size_t si = 0; si < alt.size(); si++) {
                    const string& sym = alt[si];
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
                        const set<string>& firstSym = first[sym];
                        for (set<string>::const_iterator sit = firstSym.begin(); sit != firstSym.end(); ++sit) {
                            const string& s = *sit;
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
    for (size_t si = 0; si < seq.size(); si++) {
        const string& sym = seq[si];
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
        map<string, set<string>>::const_iterator it = first.find(sym);
        if (it != first.end()) {
            const set<string>& symFirst = it->second;
            for (set<string>::const_iterator sit = symFirst.begin(); sit != symFirst.end(); ++sit) {
                const string& s = *sit;
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
    for (size_t i = 0; i < g.ntOrder.size(); i++) {
        const string& nt = g.ntOrder[i];
        follow[nt];
    }

    // Start symbol gets $
    follow[g.startSymbol].insert("$");

    bool changed = true;
    while (changed) {
        changed = false;

        for (size_t ni = 0; ni < g.ntOrder.size(); ni++) {
            const string& nt = g.ntOrder[ni];
            const vector<vector<string>>& alts = g.productions.at(nt);

            for (size_t ai = 0; ai < alts.size(); ai++) {
                const vector<string>& alt = alts[ai];
                // For every symbol B in the alt that is a non-terminal:
                for (size_t i = 0; i < alt.size(); i++) {
                    const string& B = alt[i];
                    if (!g.isNonTerminal(B)) continue;

                    // beta = everything after B in this production
                    vector<string> beta(alt.begin() + i + 1,
                                                   alt.end());

                    // Add FIRST(beta) - {epsilon} to FOLLOW(B)
                    set<string> fb = firstOfSequence(beta, g);
                    for (set<string>::const_iterator sit = fb.begin(); sit != fb.end(); ++sit) {
                        const string& s = *sit;
                        if (s != "epsilon") {
                            if (follow[B].insert(s).second) changed = true;
                        }
                    }

                    // If epsilon in FIRST(beta), add FOLLOW(nt) to FOLLOW(B)
                    if (fb.count("epsilon")) {
                        const set<string>& followNt = follow[nt];
                        for (set<string>::const_iterator sit = followNt.begin(); sit != followNt.end(); ++sit) {
                            const string& s = *sit;
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
    for (map<string, set<string>>::const_iterator it = first.begin(); it != first.end(); ++it) {
        cout << setw(16) << left << it->first << " : { ";
        for (set<string>::const_iterator sit = it->second.begin(); sit != it->second.end(); ++sit) {
            cout << *sit << " ";
        }
        cout << "}\n";
    }
}

// ─────────────────────────────────────────────────────────────
// printFollow
// ─────────────────────────────────────────────────────────────
void FirstFollow::printFollow() const {
    cout << "\n--- FOLLOW Sets ---\n";
    for (map<string, set<string>>::const_iterator it = follow.begin(); it != follow.end(); ++it) {
        cout << setw(16) << left << it->first << " : { ";
        for (set<string>::const_iterator sit = it->second.begin(); sit != it->second.end(); ++sit) {
            cout << *sit << " ";
        }
        cout << "}\n";
    }
}
