#include "parser.h"
#include "stack.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

using namespace std;

// ─────────────────────────────────────────────────────────────
// buildTable
// ─────────────────────────────────────────────────────────────
void Parser::buildTable(const Grammar& g, const FirstFollow& ff) {
    for (size_t i = 0; i < g.ntOrder.size(); i++) {
        const string& nt = g.ntOrder[i];
        const vector<vector<string>>& alts = g.productions.at(nt);

        for (size_t j = 0; j < alts.size(); j++) {
            const vector<string>& alt = alts[j];
            // Compute FIRST of this alternative
            set<string> firstAlpha = ff.firstOfSequence(alt, g);

            // Rule 1: for each terminal a in FIRST(alpha), add to table[nt][a]
            for (set<string>::const_iterator sit = firstAlpha.begin(); sit != firstAlpha.end(); ++sit) {
                const string& sym = *sit;
                if (sym == "epsilon") continue;
                if (table[nt].count(sym)) {
                    // Conflict!
                    cout << "  [CONFLICT] table[" << nt << "][" << sym
                              << "] already has a production — grammar is NOT LL(1)\n";
                    isLL1 = false;
                }
                table[nt][sym] = alt;
            }

            // Rule 2: if epsilon in FIRST(alpha), use FOLLOW(nt)
            if (firstAlpha.count("epsilon")) {
                map<string, set<string>>::const_iterator followIt = ff.follow.find(nt);
                if (followIt != ff.follow.end()) {
                    const set<string>& followSet = followIt->second;
                    for (set<string>::const_iterator sit = followSet.begin(); sit != followSet.end(); ++sit) {
                        const string& sym = *sit;
                        if (table[nt].count(sym)) {
                            cout << "  [CONFLICT] table[" << nt << "][" << sym
                                      << "] already has a production — grammar is NOT LL(1)\n";
                            isLL1 = false;
                        }
                        table[nt][sym] = alt; // epsilon production
                    }
                }
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────
// printTable – simple grid layout
// ─────────────────────────────────────────────────────────────
void Parser::printTable(const Grammar& g) const {
    cout << "\n--- LL(1) Parsing Table ---\n";

    // Collect all terminals (columns)
    set<string> terminals;
    for (map<string, map<string, vector<string>>>::const_iterator ntIt = table.begin(); ntIt != table.end(); ++ntIt) {
        for (map<string, vector<string>>::const_iterator termIt = ntIt->second.begin(); termIt != ntIt->second.end(); ++termIt) {
            terminals.insert(termIt->first);
        }
    }

    // Header row
    int colW = 20;
    cout << setw(16) << left << "NT \\ Terminal";
    for (set<string>::const_iterator tit = terminals.begin(); tit != terminals.end(); ++tit) {
        const string& t = *tit;
        cout << setw(colW) << left << t;
    }
    cout << "\n" << string(16 + colW * (int)terminals.size(), '-') << "\n";

    // Data rows
    for (size_t i = 0; i < g.ntOrder.size(); i++) {
        const string& nt = g.ntOrder[i];
        cout << setw(16) << left << nt;
        for (set<string>::const_iterator tit = terminals.begin(); tit != terminals.end(); ++tit) {
            const string& t = *tit;
            auto rowIt = table.find(nt);
            if (rowIt != table.end()) {
                auto cellIt = rowIt->second.find(t);
                if (cellIt != rowIt->second.end()) {
                    // Build production string
                    string prod = nt + "->";
                    for (size_t si = 0; si < cellIt->second.size(); si++) {
                        prod += cellIt->second[si] + " ";
                    }
                    cout << setw(colW) << left << prod;
                } else {
                    cout << setw(colW) << left << " ";
                }
            } else {
                cout << setw(colW) << left << " ";
            }
        }
        cout << "\n";
    }

    cout << "\nGrammar is " << (isLL1 ? "LL(1)" : "NOT LL(1)") << "\n";
}

// ─────────────────────────────────────────────────────────────
// displayTableDOT – Output LL(1) table as Graphviz DOT file
// ─────────────────────────────────────────────────────────────
void Parser::displayTableDOT(const Grammar& g, const string& outputFolder) const {
    // Create filename: table.dot
    string filename = (outputFolder.empty() ? "" : outputFolder + "/") + "table.dot";
    ofstream out(filename);
    if (!out.is_open()) {
        cerr << "Could not open " << filename << " for writing\n";
        return;
    }

    out << "digraph LL1Table {\n";
    out << "  rankdir=LR;\n";
    out << "  graph [splines=ortho, nodesep=0.5];\n";
    out << "  node [shape=box, fontname=\"Courier\", fontsize=10];\n\n";

    // Collect all terminals (columns)
    set<string> terminals;
    for (map<string, map<string, vector<string>>>::const_iterator ntIt = table.begin(); ntIt != table.end(); ++ntIt) {
        for (map<string, vector<string>>::const_iterator termIt = ntIt->second.begin(); termIt != ntIt->second.end(); ++termIt) {
            terminals.insert(termIt->first);
        }
    }

    // Header row nodes
    out << "  // Header row\n";
    out << "  header [shape=plaintext, label=<\n";
    out << "    <table border=\"1\" cellborder=\"1\" cellspacing=\"0\">\n";
    out << "      <tr>\n";
    out << "        <td><b>NT \\ Terminal</b></td>\n";
    for (set<string>::const_iterator tit = terminals.begin(); tit != terminals.end(); ++tit) {
        out << "        <td><b>" << *tit << "</b></td>\n";
    }
    out << "      </tr>\n";

    // Data rows
    for (size_t i = 0; i < g.ntOrder.size(); i++) {
        const string& nt = g.ntOrder[i];
        out << "      <tr>\n";
        out << "        <td><b>" << nt << "</b></td>\n";
        
        for (set<string>::const_iterator tit = terminals.begin(); tit != terminals.end(); ++tit) {
            const string& t = *tit;
            map<string, map<string, vector<string>>>::const_iterator rowIt = table.find(nt);
            
            out << "        <td>";
            if (rowIt != table.end()) {
                map<string, vector<string>>::const_iterator cellIt = rowIt->second.find(t);
                if (cellIt != rowIt->second.end()) {
                    // Build production string
                    out << nt << " &rarr; ";
                    for (size_t si = 0; si < cellIt->second.size(); si++) {
                        if (si > 0) out << " ";
                        out << cellIt->second[si];
                    }
                }
            }
            out << "</td>\n";
        }
        out << "      </tr>\n";
    }

    out << "    </table>\n";
    out << "  >];\n\n";
    out << "}\n";
    out.close();

    cout << "[OK] Parsing table exported to " << filename << "\n";

    // Try to invoke dot command to generate PNG
    #ifdef _WIN32
        string dotPath = "C:\\Program Files\\Graphviz\\bin\\dot.exe";
        string pngCmd = "\"" + dotPath + "\" -Tpng " + filename + " -o " + 
                        filename.substr(0, filename.rfind('.')) + ".png";
    #else
        string pngCmd = "dot -Tpng " + filename + " -o " + filename.substr(0, filename.rfind('.')) + ".png";
    #endif
    
    int ret = system(pngCmd.c_str());
    if (ret == 0) {
        cout << "[OK] PNG generated: " << filename.substr(0, filename.rfind('.')) << ".png\n";
    } else {
        cout << "Note: PNG generation failed. DOT file created (install Graphviz or run: dot -Tpng " << filename << " -o " << filename.substr(0, filename.rfind('.')) << ".png)\n";
    }
}

// ─────────────────────────────────────────────────────────────
// displayParseLtraceDOT – Output parsing trace as Graphviz DOT file
// ─────────────────────────────────────────────────────────────
void Parser::displayParseLtraceDOT(const string& outputFolder) const {
    if (parseTrace.empty()) {
        cout << "No parsing trace to visualize.\n";
        return;
    }

    static int traceNum = 0;
    traceNum++;
    
    // Create filename: trace1.dot, trace2.dot, ... (within output folder)
    string filename = (outputFolder.empty() ? "" : outputFolder + "/") + "trace" + to_string(traceNum) + ".dot";
    ofstream out(filename);
    if (!out.is_open()) {
        cerr << "Could not open " << filename << " for writing\n";
        return;
    }

    out << "digraph ParseTrace {\n";
    out << "  rankdir=LR;\n";
    out << "  graph [splines=ortho, nodesep=0.5];\n";
    out << "  node [shape=box, fontname=\"Courier\", fontsize=9];\n\n";

    out << "  // Parsing trace table\n";
    out << "  trace [shape=plaintext, label=<\n";
    out << "    <table border=\"1\" cellborder=\"1\" cellspacing=\"0\">\n";
    out << "      <tr>\n";
    out << "        <td><b>Step</b></td>\n";
    out << "        <td><b>Stack (bottom→top)</b></td>\n";
    out << "        <td><b>Input (remaining)</b></td>\n";
    out << "        <td><b>Action</b></td>\n";
    out << "      </tr>\n";

    // Output each step with HTML-escaped content
    for (size_t i = 0; i < parseTrace.size(); i++) {
        const ParseStep& step = parseTrace[i];
        
        // Escape HTML special characters
        auto htmlEscape = [](const string& s) {
            string result = s;
            for (size_t j = 0; j < result.size(); ++j) {
                if (result[j] == '<') { result.replace(j, 1, "&lt;"); j += 3; }
                else if (result[j] == '>') { result.replace(j, 1, "&gt;"); j += 3; }
                else if (result[j] == '&') { result.replace(j, 1, "&amp;"); j += 4; }
                else if (result[j] == '"') { result.replace(j, 1, "&quot;"); j += 5; }
            }
            return result;
        };
        
        out << "      <tr>\n";
        out << "        <td>" << step.step << "</td>\n";
        out << "        <td><font face=\"monospace\">" << htmlEscape(step.stack) << "</font></td>\n";
        out << "        <td><font face=\"monospace\">" << htmlEscape(step.input) << "</font></td>\n";
        out << "        <td><font face=\"monospace\">" << htmlEscape(step.action) << "</font></td>\n";
        out << "      </tr>\n";
    }

    out << "    </table>\n";
    out << "  >];\n\n";
    out << "}\n";
    out.close();

    cout << "[OK] Parsing trace exported to " << filename << "\n";

    // Try to invoke dot command to generate PNG
    #ifdef _WIN32
        string dotPath = "C:\\Program Files\\Graphviz\\bin\\dot.exe";
        string pngCmd = "\"" + dotPath + "\" -Tpng " + filename + " -o " + 
                        filename.substr(0, filename.rfind('.')) + ".png";
    #else
        string pngCmd = "dot -Tpng " + filename + " -o " + filename.substr(0, filename.rfind('.')) + ".png";
    #endif
    
    int ret = system(pngCmd.c_str());
    if (ret == 0) {
        cout << "[OK] PNG generated: " << filename.substr(0, filename.rfind('.')) << ".png\n";
    } else {
        cout << "Note: PNG generation failed. DOT file created.\n";
    }
}

// ─────────────────────────────────────────────────────────────
// Helper: produce a display string for remaining tokens
// ─────────────────────────────────────────────────────────────
static string tokensToString(const vector<string>& tokens,
                                   size_t pos) {
    string s;
    for (size_t i = pos; i < tokens.size(); i++) s += tokens[i] + " ";
    return s.empty() ? "$" : s;
}

// ─────────────────────────────────────────────────────────────
// parse
// Stack-based LL(1) parsing with panic-mode error recovery.
// Returns the parse tree root (nullptr on total failure).
// ─────────────────────────────────────────────────────────────
shared_ptr<TreeNode> Parser::parse(const vector<string>& tokens,
                                         const Grammar& g,
                                         const FirstFollow& ff,
                                         ErrorHandler& err) {
    // ── Setup ──────────────────────────────────────────────
    parseTrace.clear();  // Clear previous parse trace
    
    Stack stack;
    stack.push("$");
    stack.push(g.startSymbol);

    // Append "$" sentinel to token stream
    vector<string> input = tokens;
    input.push_back("$");
    size_t pos = 0;   // current position in input
    int lineNum = 1;  // line number tracking
    int col = 1;      // column number tracking

    // ── Parse tree support ─────────────────────────────────
    // We maintain a parallel "tree node stack" that mirrors the symbol stack.
    // When we pop a symbol we also pop (and fill) its tree node.
    vector<shared_ptr<TreeNode>> nodeStack;
    auto rootNode = make_shared<TreeNode>(g.startSymbol);
    nodeStack.push_back(nullptr);          // dummy for "$"
    nodeStack.push_back(rootNode);

    // ── Trace header ───────────────────────────────────────
    int stepW = 5, stackW = 36, inputW = 28, actionW = 40;
    cout << "\n"
              << setw(stepW)  << left << "Step"
              << setw(stackW) << left << "Stack (bottom->top)"
              << setw(inputW) << left << "Input"
              << "Action\n"
              << string(stepW + stackW + inputW + actionW, '-') << "\n";

    int step = 1;
    bool accepted = false;

    // ── Main parsing loop ──────────────────────────────────
    while (!stack.isEmpty()) {
        string X   = stack.top();
        string a   = input[pos];

        // Print trace row
        cout << setw(stepW)  << left << step
                  << setw(stackW) << left << stack.toString()
                  << setw(inputW) << left << tokensToString(input, pos);

        // ── Case 1: both are $ ─────────────────────────────
        if (X == "$" && a == "$") {
            cout << "Accept\n";
            ParseStep ps;
            ps.step = step;
            ps.stack = stack.toString();
            ps.input = tokensToString(input, pos);
            ps.action = "Accept";
            parseTrace.push_back(ps);
            accepted = true;
            break;
        }

        // ── Case 2: X is terminal (or $) ───────────────────
        if (!g.isNonTerminal(X)) {
            if (X == a) {
                // Match
                cout << "Match '" << a << "'\n";
                ParseStep ps;
                ps.step = step;
                ps.stack = stack.toString();
                ps.input = tokensToString(input, pos);
                ps.action = "Match '" + a + "'";
                parseTrace.push_back(ps);
                stack.pop();
                nodeStack.pop_back();
                pos++;
            } else {
                // Terminal mismatch error
                err.reportError(lineNum, col, X, a);
                cout << "\n"; // newline after action column
                ParseStep ps;
                ps.step = step;
                ps.stack = stack.toString();
                ps.input = tokensToString(input, pos);
                ps.action = "ERROR: expected '" + X + "' but found '" + a + "'";
                parseTrace.push_back(ps);
                // Recovery: pop the offending symbol from the stack
                stack.pop();
                nodeStack.pop_back();
            }
            step++;
            continue;
        }

        // ── Case 3: X is a non-terminal ────────────────────
        auto rowIt = table.find(X);
        if (rowIt == table.end() || rowIt->second.find(a) == rowIt->second.end()) {
            // Empty table entry — panic mode recovery
            err.reportMissingProduction(lineNum, col, X, a);
            cout << "\n";

            // Panic mode: pop stack until we find a synchronizing symbol
            const auto& followX = ff.follow.at(X);
            // First, pop stack symbols until we find one in FOLLOW(X)
            while (!stack.isEmpty() && stack.top() != "$" && !followX.count(stack.top())) {
                stack.pop();
                nodeStack.pop_back();
            }
            // Then skip input tokens until we find one in FOLLOW(X)
            while (pos < input.size() && !followX.count(input[pos]) && input[pos] != "$") {
                err.reportRecovery(input[pos], "FOLLOW("+X+")");
                col++;
                pos++;
            }
            step++;
            continue;
        }

        // Found a production: expand X -> rhs
        const auto& rhs = rowIt->second.at(a);

        // Build action string
        string actionStr = X + " -> ";
        for (size_t si = 0; si < rhs.size(); si++) {
            actionStr += rhs[si] + " ";
        }
        cout << actionStr << "\n";
        
        // Record this step
        ParseStep ps;
        ps.step = step;
        ps.stack = stack.toString();
        ps.input = tokensToString(input, pos);
        ps.action = actionStr;
        parseTrace.push_back(ps);

        // Pop X from stack
        auto parentNode = nodeStack.back();
        stack.pop();
        nodeStack.pop_back();

        // Push RHS in REVERSE order (so first symbol ends on top)
        if (!(rhs.size() == 1 && rhs[0] == "epsilon")) {
            // Create child nodes (in production order)
            vector<shared_ptr<TreeNode>> childNodes;
            for (size_t si = 0; si < rhs.size(); si++) {
                childNodes.push_back(make_shared<TreeNode>(rhs[si]));
            }
            
            // Add all children to parent node (in production order)
            if (parentNode) {
                for (size_t cn_idx = 0; cn_idx < childNodes.size(); cn_idx++) {
                    parentNode->children.push_back(childNodes[cn_idx]);
                }
            }

            // Push children to stack in REVERSE order (so first is on top)
            for (int i = (int)childNodes.size() - 1; i >= 0; i--) {
                stack.push(childNodes[i]->label);
                nodeStack.push_back(childNodes[i]);
            }
        } else {
            // epsilon production — add epsilon leaf to tree
            if (parentNode)
                parentNode->children.push_back(
                    make_shared<TreeNode>("epsilon"));
        }

        step++;
    }

    if (!accepted && !err.hasErrors()) {
        // Stack exhausted before input
        err.reportPrematureEnd(lineNum, col, stack.isEmpty() ? "$" : stack.top());
    }

    cout << string(stepW + stackW + inputW + actionW, '-') << "\n";
    if (accepted && !err.hasErrors())
        cout << "Result: String ACCEPTED successfully!\n";
    else if (accepted)
        cout << "Result: Parsing completed with " << err.errorCount << " error(s).\n";
    else
        cout << "Result: String REJECTED. " << err.errorCount << " error(s).\n";

    return accepted ? rootNode : nullptr;
}
