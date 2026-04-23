#include "Grammar.h"
#include <algorithm>
#include <sstream>

const Symbol Grammar::END_MARKER = Symbol("$", TERMINAL);

Grammar::Grammar() : startSymbol("", NON_TERMINAL), augmentedStartSymbol("", NON_TERMINAL) {}

bool Grammar::isTerminal(const string& symbol) const {
    if (symbol == "epsilon" || symbol == "@") return false;
    if (symbol.empty()) return false;
    
    // Terminals: lowercase letters or symbols, or keywords
    if (symbol[0] >= 'a' && symbol[0] <= 'z') return true;
    if (symbol == "+" || symbol == "*" || symbol == "(" || symbol == ")" || symbol == "=" || symbol == "|") return true;
    if (symbol == "id" || symbol == "if" || symbol == "then" || symbol == "else" || symbol == "other") return true;
    
    return false;
}

bool Grammar::isNonTerminal(const string& symbol) const {
    if (symbol.empty()) return false;
    if (symbol[0] >= 'A' && symbol[0] <= 'Z') return true;
    return false;
}

bool Grammar::loadFromFile(const string& filename) {
    ifstream file;
    vector<string> candidatePaths;
    set<string> seen;

    // Try the path as provided first.
    candidatePaths.push_back(filename);

    // Also try from parent folders to support running executable from bin/.
    candidatePaths.push_back("../" + filename);
    candidatePaths.push_back("../../" + filename);

    // If only a file name is provided, try common input folder locations.
    bool hasSlash = filename.find('/') != string::npos || filename.find('\\') != string::npos;
    if (!hasSlash) {
        candidatePaths.push_back("input/" + filename);
        candidatePaths.push_back("../input/" + filename);
        candidatePaths.push_back("../../input/" + filename);
    }

    string openedPath = "";
    for (int i = 0; i < candidatePaths.size(); i++) {
        const string& path = candidatePaths[i];
        if (seen.find(path) != seen.end()) {
            continue;
        }
        seen.insert(path);

        file.open(path);
        if (file.is_open()) {
            openedPath = path;
            break;
        }
        file.clear();
    }

    if (!file.is_open()) {
        cerr << "Error: Cannot open file " << filename << endl;
        cerr << "Tried paths:" << endl;
        for (int i = 0; i < candidatePaths.size(); i++) {
            cerr << "  - " << candidatePaths[i] << endl;
        }
        return false;
    }
    
    string line;
    int lineNum = 0;
    
    while (getline(file, line)) {
        lineNum++;
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;
        
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        // Find the arrow
        size_t arrowPos = line.find("->");
        if (arrowPos == string::npos) {
            cerr << "Error at line " << lineNum << ": No arrow (->) found" << endl;
            continue;
        }
        
        // Parse left side
        string leftStr = line.substr(0, arrowPos);
        leftStr.erase(0, leftStr.find_first_not_of(" \t"));
        leftStr.erase(leftStr.find_last_not_of(" \t") + 1);
        
        if (!isNonTerminal(leftStr)) {
            cerr << "Error at line " << lineNum << ": Left side must be non-terminal" << endl;
            continue;
        }
        
        Symbol leftSymbol(leftStr, NON_TERMINAL);
        if (productions.empty()) {
            startSymbol = leftSymbol;
        }
        nonTerminals.insert(leftSymbol);
        
        // Parse right side (handle multiple alternatives separated by |)
        string rightStr = line.substr(arrowPos + 2);
        size_t pos = 0;
        string alternative;
        
        while (pos < rightStr.length()) {
            size_t pipePos = rightStr.find('|', pos);
            if (pipePos == string::npos) {
                alternative = rightStr.substr(pos);
            } else {
                alternative = rightStr.substr(pos, pipePos - pos);
            }
            
            // Trim whitespace
            alternative.erase(0, alternative.find_first_not_of(" \t"));
            alternative.erase(alternative.find_last_not_of(" \t") + 1);
            
            if (!alternative.empty()) {
                // Parse symbols in this alternative
                vector<Symbol> rightSymbols;
                istringstream iss(alternative);
                string symbol;
                
                while (iss >> symbol) {
                    if (symbol == "epsilon" || symbol == "@") {
                        rightSymbols.push_back(Symbol("epsilon", EPSILON));
                    } else if (isNonTerminal(symbol)) {
                        Symbol sym(symbol, NON_TERMINAL);
                        rightSymbols.push_back(sym);
                        nonTerminals.insert(sym);
                    } else if (isTerminal(symbol)) {
                        Symbol sym(symbol, TERMINAL);
                        rightSymbols.push_back(sym);
                        terminals.insert(sym);
                    } else {
                        cerr << "Warning at line " << lineNum << ": Unknown symbol '" << symbol << "'" << endl;
                    }
                }
                
                // Add this production
                Production prod(leftSymbol, rightSymbols);
                prod.productionId = productions.size();
                productions.push_back(prod);
            }
            
            if (pipePos == string::npos) break;
            pos = pipePos + 1;
        }
    }
    
    file.close();
    
    // Add end marker
    terminals.insert(END_MARKER);
    
    // Compute FIRST and FOLLOW sets
    computeFirstSets();
    computeFollowSets();
    
    return true;
}

void Grammar::augmentGrammar() {
    if (startSymbol.name.empty()) {
        cerr << "Error: No start symbol found" << endl;
        return;
    }
    
    augmentedStartSymbol = Symbol(startSymbol.name + "Prime", NON_TERMINAL);
    nonTerminals.insert(augmentedStartSymbol);
    
    // Add new production: S' -> S
    vector<Symbol> rightSide;
    rightSide.push_back(startSymbol);
    Production augmentedProduction(augmentedStartSymbol, rightSide);
    augmentedProduction.productionId = 0;  // This is always production 0
    
    // Insert at the beginning
    for (int i = 0; i < productions.size(); i++) {
        productions[i].productionId = i + 1;
    }
    productions.insert(productions.begin(), augmentedProduction);

    // Recompute sets because we introduced a new non-terminal and production.
    firstSets.clear();
    followSets.clear();
    computeFirstSets();
    computeFollowSets();
}

void Grammar::computeFirstSets() {
    // Initialize: FIRST(terminal) = {terminal}
    for (const Symbol& term : terminals) {
        firstSets[term].insert(term);
    }
    
    // FIRST for non-terminals - iterate until no changes
    bool changed = true;
    while (changed) {
        changed = false;
        
        for (const Production& prod : productions) {
            Symbol A = prod.leftSide;
            int beforeSize = firstSets[A].size();
            
            if (prod.rightSide.empty()) continue;
            
            // For each production A -> X1 X2 ... Xn
            for (int i = 0; i < prod.rightSide.size(); i++) {
                Symbol X = prod.rightSide[i];
                
                // Add FIRST(X) - {epsilon}
                for (const Symbol& sym : firstSets[X]) {
                    if (sym.type != EPSILON) {
                        firstSets[A].insert(sym);
                    }
                }
                
                // If epsilon not in FIRST(X), stop
                if (firstSets[X].find(Symbol("epsilon", EPSILON)) == firstSets[X].end()) {
                    break;
                }
                
                // If this is the last symbol and epsilon in FIRST(X), add epsilon
                if (i == prod.rightSide.size() - 1) {
                    firstSets[A].insert(Symbol("epsilon", EPSILON));
                }
            }
            
            if (firstSets[A].size() > beforeSize) {
                changed = true;
            }
        }
    }
}

void Grammar::computeFollowSets() {
    // Initialize: FOLLOW(S') = {$}
    followSets[augmentedStartSymbol].insert(END_MARKER);
    
    // Iterate until no changes
    bool changed = true;
    while (changed) {
        changed = false;
        
        for (const Production& prod : productions) {
            vector<Symbol> beta = prod.rightSide;
            
            // For each production A -> alpha B beta
            for (int i = 0; i < beta.size(); i++) {
                Symbol B = beta[i];
                if (B.type != NON_TERMINAL) continue;
                
                int beforeSize = followSets[B].size();
                
                // beta = symbols after B
                vector<Symbol> betaAfterB;
                for (int j = i + 1; j < beta.size(); j++) {
                    betaAfterB.push_back(beta[j]);
                }
                
                // Add FIRST(beta) - {epsilon}
                set<Symbol> firstBeta = getFirstSetOfString(betaAfterB);
                for (const Symbol& sym : firstBeta) {
                    if (sym.type != EPSILON) {
                        followSets[B].insert(sym);
                    }
                }
                
                // If epsilon in FIRST(beta), add FOLLOW(A)
                if (firstBeta.find(Symbol("epsilon", EPSILON)) != firstBeta.end()) {
                    for (const Symbol& sym : followSets[prod.leftSide]) {
                        followSets[B].insert(sym);
                    }
                }
                
                if (followSets[B].size() > beforeSize) {
                    changed = true;
                }
            }
        }
    }
}

set<Symbol> Grammar::getFirstSet(const Symbol& symbol) const {
    auto it = firstSets.find(symbol);
    if (it != firstSets.end()) {
        return it->second;
    }
    return set<Symbol>();
}

set<Symbol> Grammar::getFirstSetOfString(const vector<Symbol>& symbols) const {
    set<Symbol> result;
    
    if (symbols.empty()) {
        result.insert(Symbol("epsilon", EPSILON));
        return result;
    }
    
    for (int i = 0; i < symbols.size(); i++) {
        set<Symbol> firstI = getFirstSet(symbols[i]);
        
        for (const Symbol& sym : firstI) {
            if (sym.type != EPSILON) {
                result.insert(sym);
            }
        }
        
        // If epsilon not in FIRST(symbols[i]), stop
        if (firstI.find(Symbol("epsilon", EPSILON)) == firstI.end()) {
            return result;
        }
    }
    
    // All have epsilon, so epsilon is in result
    result.insert(Symbol("epsilon", EPSILON));
    return result;
}

set<Symbol> Grammar::getFollowSet(const Symbol& symbol) const {
    auto it = followSets.find(symbol);
    if (it != followSets.end()) {
        return it->second;
    }
    return set<Symbol>();
}

vector<Production> Grammar::getProductionsFor(const Symbol& nonTerminal) const {
    vector<Production> result;
    for (const Production& prod : productions) {
        if (prod.leftSide == nonTerminal) {
            result.push_back(prod);
        }
    }
    return result;
}

void Grammar::displayGrammar() const {
    cout << "\n=== Original Grammar ===" << endl;
    for (const Production& prod : productions) {
        cout << prod.toString() << endl;
    }
}

void Grammar::displayAugmentedGrammar() const {
    cout << "\n=== Augmented Grammar ===" << endl;
    for (const Production& prod : productions) {
        cout << prod.toString() << endl;
    }
}

void Grammar::displayFirstAndFollowSets() const {
    cout << "\n=== FIRST Sets ===" << endl;
    for (const Symbol& sym : nonTerminals) {
        cout << "FIRST(" << sym.toString() << ") = { ";
        auto firstIt = firstSets.find(sym);
        if (firstIt != firstSets.end()) {
            for (const Symbol& first : firstIt->second) {
                cout << first.toString() << " ";
            }
        }
        cout << "}" << endl;
    }
    
    cout << "\n=== FOLLOW Sets ===" << endl;
    for (const Symbol& sym : nonTerminals) {
        auto it = followSets.find(sym);
        if (it != followSets.end()) {
            cout << "FOLLOW(" << sym.toString() << ") = { ";
            for (const Symbol& follow : it->second) {
                cout << follow.toString() << " ";
            }
            cout << "}" << endl;
        }
    }
}

void Grammar::saveAugmentedGrammarToFile(const string& filename) const {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Cannot create file " << filename << endl;
        return;
    }
    
    for (const Production& prod : productions) {
        file << prod.toString() << endl;
    }
    
    file.close();
}
