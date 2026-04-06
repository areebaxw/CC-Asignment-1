#include "grammar.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

// ─────────────────────────────────────────────────────────────
// Utility: trim whitespace from both ends of a string
// ─────────────────────────────────────────────────────────────
static string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// ─────────────────────────────────────────────────────────────
// Split a string by a delimiter string (not char)
// ─────────────────────────────────────────────────────────────
static vector<string> splitBy(const string& s,
                                         const string& delim) {
    vector<string> parts;
    size_t pos = 0, found;
    while ((found = s.find(delim, pos)) != string::npos) {
        parts.push_back(trim(s.substr(pos, found - pos)));
        pos = found + delim.size();
    }
    parts.push_back(trim(s.substr(pos)));
    return parts;
}

// ─────────────────────────────────────────────────────────────
// Tokenise a production RHS into individual symbols
// e.g. "Term ExprPrime" -> {"Term", "ExprPrime"}
// ─────────────────────────────────────────────────────────────
static vector<string> tokenise(const string& s) {
    vector<string> tokens;
    istringstream iss(s);
    string tok;
    while (iss >> tok) tokens.push_back(tok);
    return tokens;
}

// ─────────────────────────────────────────────────────────────
// loadFromFile
// Expected format:  NonTerminal -> alt1 | alt2 | ...
// ─────────────────────────────────────────────────────────────
bool Grammar::loadFromFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Cannot open grammar file: " << filename << "\n";
        return false;
    }

    string line;
    bool firstRule = true;
    while (getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;   // skip blanks/comments

        // Split on "->"
        auto parts = splitBy(line, "->");
        if (parts.size() != 2) {
            cerr << "Bad line (no ->): " << line << "\n";
            continue;
        }

        string nt = trim(parts[0]);
        string rhsSide = trim(parts[1]);

        // First non-terminal seen becomes the start symbol
        if (firstRule) { startSymbol = nt; firstRule = false; }

        // Track insertion order
        if (productions.find(nt) == productions.end()) {
            ntOrder.push_back(nt);
            productions[nt] = {};
        }

        // Split alternatives on "|"
        vector<string> alts = splitBy(rhsSide, "|");
        for (size_t i = 0; i < alts.size(); i++) {
            string& alt = alts[i];
            alt = trim(alt);
            if (alt.empty()) continue;
            productions[nt].push_back(tokenise(alt));
        }
    }
    return true;
}

// ─────────────────────────────────────────────────────────────
// isNonTerminal: a symbol is a NT if it exists in productions
// ─────────────────────────────────────────────────────────────
bool Grammar::isNonTerminal(const string& sym) const {
    return productions.find(sym) != productions.end();
}

// ─────────────────────────────────────────────────────────────
// print
// ─────────────────────────────────────────────────────────────
void Grammar::print() const {
    for (size_t i = 0; i < ntOrder.size(); i++) {
        const string& nt = ntOrder[i];
        auto it = productions.find(nt);
        if (it == productions.end()) continue;
        cout << nt << " -> ";
        const auto& alts = it->second;
        for (size_t i = 0; i < alts.size(); i++) {
            for (size_t j = 0; j < alts[i].size(); j++) {
                cout << alts[i][j];
                if (j + 1 < alts[i].size()) cout << " ";
            }
            if (i + 1 < alts.size()) cout << " | ";
        }
        cout << "\n";
    }
}

// ─────────────────────────────────────────────────────────────
// freshName: create  "ExprPrime", then "ExprPrime2", etc.
// ─────────────────────────────────────────────────────────────
string Grammar::freshName(const string& base) {
    string candidate = base + "Prime";
    int counter = 1;
    while (productions.find(candidate) != productions.end()) {
        candidate = base + "Prime" + to_string(++counter);
    }
    return candidate;
}

// ─────────────────────────────────────────────────────────────
// longestCommonPrefix among a set of alternatives
// ─────────────────────────────────────────────────────────────
vector<string> Grammar::longestCommonPrefix(
        const vector<vector<string>>& alts) const {
    if (alts.empty()) return {};
    vector<string> prefix = alts[0];
    for (size_t i = 1; i < alts.size(); i++) {
        size_t len = min(prefix.size(), alts[i].size());
        size_t match = 0;
        while (match < len && prefix[match] == alts[i][match]) match++;
        prefix.resize(match);
        if (prefix.empty()) break;
    }
    return prefix;
}

// ─────────────────────────────────────────────────────────────
// leftFactor  – repeated fixpoint until no common prefix exists
// ─────────────────────────────────────────────────────────────
void Grammar::leftFactor() {
    bool changed = true;
    while (changed) {
        changed = false;

        // We work on a snapshot of ntOrder because we may add new NTs
        vector<string> currentNTs = ntOrder;

        for (size_t i = 0; i < currentNTs.size(); i++) {
            const string& nt = currentNTs[i];
            auto& alts = productions[nt];
            if (alts.size() < 2) continue;

            // Group alternatives by their first symbol
            map<string, vector<vector<string>>> groups;
            for (size_t j = 0; j < alts.size(); j++) {
                vector<string>& alt = alts[j];
                if (alt.empty()) { groups["__eps__"].push_back(alt); continue; }
                groups[alt[0]].push_back(alt);
            }

            // Check if any group has more than one alternative (= common prefix)
            bool needsFactor = false;
            for (map<string, vector<vector<string>>>::iterator git = groups.begin(); git != groups.end(); ++git) {
                auto& g = *git;
                if (g.first != "__eps__" && g.second.size() > 1) {
                    needsFactor = true;
                    break;
                }
            }

            if (!needsFactor) continue;
            changed = true;

            // Build new alternatives for nt
            vector<vector<string>> newAlts;

            for (map<string, vector<vector<string>>>::iterator gEntryIt = groups.begin(); gEntryIt != groups.end(); ++gEntryIt) {
                auto& gEntry = *gEntryIt;
                const string& firstSym = gEntry.first;
                vector<vector<string>>& group = gEntry.second;
                if (firstSym == "__eps__") {
                    // keep epsilon alternatives as-is
                    for (size_t gi = 0; gi < group.size(); gi++) {
                        newAlts.push_back(group[gi]);
                    }
                    continue;
                }

                if (group.size() == 1) {
                    // Only one alternative starts with firstSym — no factoring needed
                    newAlts.push_back(group[0]);
                    continue;
                }

                // Find the longest common prefix of this group
                vector<string> prefix = longestCommonPrefix(group);

                // Create new NT:  nt + "Prime"
                string newNT = freshName(nt);
                ntOrder.push_back(newNT);
                productions[newNT] = {};

                // Suffixes become productions of newNT
                for (size_t gi = 0; gi < group.size(); gi++) {
                    vector<string>& alt = group[gi];
                    vector<string> suffix(alt.begin() + prefix.size(),
                                                     alt.end());
                    if (suffix.empty()) suffix = {"epsilon"};
                    productions[newNT].push_back(suffix);
                }

                // New production for nt: prefix + newNT
                vector<string> factored = prefix;
                factored.push_back(newNT);
                newAlts.push_back(factored);
            }

            alts = newAlts;
        }
    }
}

// ─────────────────────────────────────────────────────────────
// removeDirect – remove direct left recursion from ONE non-terminal
// A -> A alpha | beta   =>  A -> beta A'
//                           A' -> alpha A' | epsilon
// ─────────────────────────────────────────────────────────────
void Grammar::removeDirect(const string& nt) {
    vector<vector<string>>& alts = productions[nt];

    vector<vector<string>> recursive;   // A -> A ...
    vector<vector<string>> nonRecursive;// A -> ...

    for (size_t i = 0; i < alts.size(); i++) {
        vector<string>& alt = alts[i];
        if (!alt.empty() && alt[0] == nt)
            recursive.push_back(vector<string>(alt.begin()+1, alt.end()));
        else
            nonRecursive.push_back(alt);
    }

    if (recursive.empty()) return; // no direct left recursion

    string prime = freshName(nt);
    ntOrder.push_back(prime);
    productions[prime] = {};

    // A -> beta A'  for each non-recursive alt
    alts.clear();
    for (size_t i = 0; i < nonRecursive.size(); i++) {
        vector<string>& beta = nonRecursive[i];
        auto newAlt = beta;
        newAlt.push_back(prime);
        alts.push_back(newAlt);
    }
    // If there were no non-recursive alts at all
    if (alts.empty()) {
        alts.push_back({prime});
    }

    // A' -> alpha A'  for each recursive alt
    for (size_t i = 0; i < recursive.size(); i++) {
        vector<string>& alpha = recursive[i];
        auto newAlt = alpha;
        newAlt.push_back(prime);
        productions[prime].push_back(newAlt);
    }
    // A' -> epsilon
    productions[prime].push_back({"epsilon"});
}

// ─────────────────────────────────────────────────────────────
// removeLeftRecursion – handles both direct and indirect
// Uses the classic textbook algorithm (order-based substitution)
// ─────────────────────────────────────────────────────────────
void Grammar::removeLeftRecursion() {
    // Work on a snapshot of the original NT list
    vector<string> orderedNTs = ntOrder;
    int n = (int)orderedNTs.size();

    for (int i = 0; i < n; i++) {
        const string& Ai = orderedNTs[i];

        // For each j < i, replace Ai -> Aj gamma  with  Ai -> delta gamma
        for (int j = 0; j < i; j++) {
            const string& Aj = orderedNTs[j];
            vector<vector<string>>& AiAlts = productions[Ai];
            vector<vector<string>> newAlts;

            for (size_t altIdx = 0; altIdx < AiAlts.size(); altIdx++) {
                vector<string>& alt = AiAlts[altIdx];
                if (!alt.empty() && alt[0] == Aj) {
                    // Replace Aj with each of Aj's productions
                    vector<string> suffix(alt.begin()+1, alt.end());
                    const vector<vector<string>>& AjProds = productions[Aj];
                    for (size_t ajIdx = 0; ajIdx < AjProds.size(); ajIdx++) {
                        const vector<string>& AjAlt = AjProds[ajIdx];
                        vector<string> expanded = AjAlt;
                        // don't append epsilon symbol, just the suffix
                        if (expanded.size() == 1 && expanded[0] == "epsilon") {
                            expanded = suffix;
                        } else {
                            for (size_t sIdx = 0; sIdx < suffix.size(); sIdx++) {
                                expanded.push_back(suffix[sIdx]);
                            }
                        }
                        if (expanded.empty()) expanded = {"epsilon"};
                        newAlts.push_back(expanded);
                    }
                } else {
                    newAlts.push_back(alt);
                }
            }
            AiAlts = newAlts;
        }

        // Now eliminate direct left recursion on Ai
        removeDirect(Ai);
    }
}

// ─────────────────────────────────────────────────────────────
// displayTransformationDOT – output grammar as DOT table format
// ─────────────────────────────────────────────────────────────
void Grammar::displayTransformationDOT(const string& outputFolder) const {
    string filename = (outputFolder.empty() ? "" : outputFolder + "/") + "grammar_transformed.dot";
    ofstream out(filename);
    if (!out.is_open()) {
        cerr << "Cannot write " << filename << "\n";
        return;
    }

    out << "digraph GrammarTransformation {\n";
    out << "  rankdir=LR;\n";
    out << "  node [shape=box, style=\"rounded,filled\", fillcolor=lightblue];\n";
    out << "  table [shape=plaintext, label=<\n";
    out << "    <TABLE BORDER=\"1\" CELLBORDER=\"1\" CELLSPACING=\"0\">\n";
    out << "      <TR><TD COLSPAN=\"2\" BGCOLOR=\"lightgray\"><B>Transformed Grammar Productions</B></TD></TR>\n";

    for (size_t i = 0; i < ntOrder.size(); i++) {
        const string& nt = ntOrder[i];
        auto it = productions.find(nt);
        if (it == productions.end()) continue;

        const auto& alts = it->second;
        out << "      <TR><TD ALIGN=\"RIGHT\" BGCOLOR=\"lightyellow\"><B>" << nt << "</B></TD>";
        out << "<TD ALIGN=\"LEFT\">";

        for (size_t j = 0; j < alts.size(); j++) {
            if (j > 0) out << " | ";
            for (size_t k = 0; k < alts[j].size(); k++) {
                if (k > 0) out << " ";
                out << alts[j][k];
            }
        }
        out << "</TD></TR>\n";
    }

    out << "    </TABLE>\n";
    out << "  >];\n";
    out << "}\n";
    out.close();

    cout << "Ô£ô Grammar transformation exported to " << filename << "\n";

    // Try to generate PNG
    string pngName = filename.substr(0, filename.length() - 4) + ".png";
    string cmd = "\"C:\\Program Files\\Graphviz\\bin\\dot.exe\" -Tpng " + filename + " -o " + pngName + " 2>nul";
    if (system(cmd.c_str()) == 0) {
        cout << "Ô£ô PNG generated: " << pngName << "\n";
    }
}
