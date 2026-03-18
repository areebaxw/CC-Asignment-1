#include "parser.h"
#include "stack.h"
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;

// ─────────────────────────────────────────────────────────────
// buildTable
// ─────────────────────────────────────────────────────────────
void Parser::buildTable(const Grammar& g, const FirstFollow& ff) {
    for (const auto& nt : g.ntOrder) {
        const auto& alts = g.productions.at(nt);

        for (const auto& alt : alts) {
            // Compute FIRST of this alternative
            set<string> firstAlpha = ff.firstOfSequence(alt, g);

            // Rule 1: for each terminal a in FIRST(alpha), add to table[nt][a]
            for (const auto& sym : firstAlpha) {
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
                auto followIt = ff.follow.find(nt);
                if (followIt != ff.follow.end()) {
                    for (const auto& sym : followIt->second) {
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
    for (const auto& ntEntry : table)
        for (const auto& termEntry : ntEntry.second)
            terminals.insert(termEntry.first);

    // Header row
    int colW = 20;
    cout << setw(16) << left << "NT \\ Terminal";
    for (const auto& t : terminals)
        cout << setw(colW) << left << t;
    cout << "\n" << string(16 + colW * (int)terminals.size(), '-') << "\n";

    // Data rows
    for (const auto& nt : g.ntOrder) {
        cout << setw(16) << left << nt;
        for (const auto& t : terminals) {
            auto rowIt = table.find(nt);
            if (rowIt != table.end()) {
                auto cellIt = rowIt->second.find(t);
                if (cellIt != rowIt->second.end()) {
                    // Build production string
                    string prod = nt + "->";
                    for (const auto& s : cellIt->second) prod += s + " ";
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
            accepted = true;
            break;
        }

        // ── Case 2: X is terminal (or $) ───────────────────
        if (!g.isNonTerminal(X)) {
            if (X == a) {
                // Match
                cout << "Match '" << a << "'\n";
                stack.pop();
                nodeStack.pop_back();
                pos++;
            } else {
                // Terminal mismatch error
                err.reportError(lineNum, col, X, a);
                cout << "\n"; // newline after action column
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
        for (const auto& s : rhs) actionStr += s + " ";
        cout << actionStr << "\n";

        // Pop X from stack
        auto parentNode = nodeStack.back();
        stack.pop();
        nodeStack.pop_back();

        // Push RHS in REVERSE order (so first symbol ends on top)
        if (!(rhs.size() == 1 && rhs[0] == "epsilon")) {
            for (int i = (int)rhs.size() - 1; i >= 0; i--) {
                auto child = make_shared<TreeNode>(rhs[i]);
                if (parentNode) parentNode->children.insert(
                    parentNode->children.begin(), child);
                stack.push(rhs[i]);
                nodeStack.push_back(child);
            }
            // Fix children order: we inserted at begin, so reverse back
            // Actually let's redo: push children in order but reverse the pushes
            // The above inserts at begin each time — that reverses naturally.
            // Let's just rebuild cleanly:
            // (undo the above and redo correctly)
            // -- pop what we just pushed --
            for (size_t i = 0; i < rhs.size(); i++) { stack.pop(); nodeStack.pop_back(); }
            if (parentNode) parentNode->children.clear();

            // Push in reverse on stack; build children in forward order
            vector<shared_ptr<TreeNode>> childNodes;
            for (const auto& sym : rhs) {
                childNodes.push_back(make_shared<TreeNode>(sym));
            }
            if (parentNode)
                for (auto& cn : childNodes) parentNode->children.push_back(cn);

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
