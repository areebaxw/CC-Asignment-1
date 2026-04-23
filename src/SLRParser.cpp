#include "SLRParser.h"
#include "GraphvizConverter.h"
#include <iostream>
#include <fstream>
#include <algorithm>

SLRParser::SLRParser(const Grammar& g) : grammar(g), outputDirectory("output") {}

LR0ItemSet SLRParser::closure(const LR0ItemSet& itemSet) const {
    LR0ItemSet result = itemSet;
    bool changed = true;
    
    while (changed) {
        changed = false;
        set<LR0Item> currentItems = result.items;
        
        for (const LR0Item& item : currentItems) {
            // Get symbol after dot
            Symbol symbolAfterDot = item.getSymbolAfterDot();
            
            // If it's a non-terminal, add all productions for it
            if (symbolAfterDot.type == NON_TERMINAL) {
                vector<Production> productions = grammar.getProductionsFor(symbolAfterDot);
                
                for (const Production& prod : productions) {
                    LR0Item newItem(prod, 0);  // Dot at beginning
                    
                    if (!result.contains(newItem)) {
                        result.addItem(newItem);
                        changed = true;
                    }
                }
            }
        }
    }
    
    return result;
}

LR0ItemSet SLRParser::gotoFunc(const LR0ItemSet& itemSet, const Symbol& symbol) const {
    LR0ItemSet result;
    
    // Find all items with symbol after dot
    for (const LR0Item& item : itemSet.items) {
        Symbol afterDot = item.getSymbolAfterDot();
        
        if (afterDot == symbol) {
            // Move dot past symbol
            LR0Item newItem(item.production, item.dotPosition + 1);
            result.addItem(newItem);
        }
    }
    
    // Apply closure
    if (!result.items.empty()) {
        result = closure(result);
    }
    
    return result;
}

void SLRParser::buildCanonicalCollection() {
    // Start with augmented start production
    Production startProd = grammar.getProductions()[0];
    LR0Item startItem(startProd, 0);
    
    LR0ItemSet I0;
    I0.addItem(startItem);
    I0 = closure(I0);
    I0.stateNumber = 0;
    
    canonicalCollection.push_back(I0);
    
    // Build collection
    for (int i = 0; i < canonicalCollection.size(); i++) {
        set<Symbol> symbols;
        
        // Collect all symbols that appear after dots
        for (const LR0Item& item : canonicalCollection[i].items) {
            Symbol sym = item.getSymbolAfterDot();
            if (sym.type != TERMINAL || sym.name != "") {
                if (sym.name != "") {
                    symbols.insert(sym);
                }
            }
        }
        
        // For each symbol, compute GOTO
        for (const Symbol& sym : symbols) {
            LR0ItemSet gotoSet = gotoFunc(canonicalCollection[i], sym);
            
            if (!gotoSet.items.empty()) {
                // Check if this set already exists
                bool found = false;
                int stateNum = -1;
                
                for (int j = 0; j < canonicalCollection.size(); j++) {
                    if (canonicalCollection[j] == gotoSet) {
                        found = true;
                        stateNum = j;
                        break;
                    }
                }
                
                if (!found) {
                    gotoSet.stateNumber = canonicalCollection.size();
                    canonicalCollection.push_back(gotoSet);
                }
            }
        }
    }
}

void SLRParser::buildParsingTable() {
    parsingTable.setNumStates(canonicalCollection.size());
    
    for (int i = 0; i < canonicalCollection.size(); i++) {
        const LR0ItemSet& itemSet = canonicalCollection[i];
        
        for (const LR0Item& item : itemSet.items) {
            if (item.isDotAtEnd()) {
                // Reduce action
                if (item.production.productionId == 0) {
                    // Accept action for augmented start
                    parsingTable.setAction(i, Grammar::END_MARKER, Action(ACCEPT, 0));
                } else {
                    // Reduce action for other productions
                    set<Symbol> followSet = grammar.getFollowSet(item.production.leftSide);
                    
                    for (const Symbol& terminal : followSet) {
                        Action existingAction = parsingTable.getAction(i, terminal);
                        
                        if (existingAction.type == SHIFT) {
                            // Shift/Reduce conflict - let's prefer shift
                            // (default SLR(1) behavior)
                        } else if (existingAction.type == NONE || existingAction.type == ERROR) {
                            parsingTable.setAction(i, terminal, 
                                                 Action(REDUCE, item.production.productionId));
                        }
                    }
                }
            } else {
                // Shift action
                Symbol afterDot = item.getSymbolAfterDot();
                
                if (afterDot.type != TERMINAL || afterDot.name == "") continue;
                
                // Find the goto state
                LR0ItemSet gotoSet = gotoFunc(itemSet, afterDot);
                
                if (!gotoSet.items.empty()) {
                    for (int j = 0; j < canonicalCollection.size(); j++) {
                        if (canonicalCollection[j] == gotoSet) {
                            parsingTable.setAction(i, afterDot, Action(SHIFT, j));
                            break;
                        }
                    }
                }
            }
        }
        
        // GOTO entries for non-terminals
        for (const Symbol& nonTerm : grammar.getNonTerminals()) {
            if (nonTerm == grammar.getAugmentedStartSymbol()) continue;
            
            LR0ItemSet gotoSet = gotoFunc(itemSet, nonTerm);
            
            if (!gotoSet.items.empty()) {
                for (int j = 0; j < canonicalCollection.size(); j++) {
                    if (canonicalCollection[j] == gotoSet) {
                        parsingTable.setGoto(i, nonTerm, j);
                        break;
                    }
                }
            }
        }
    }
}

bool SLRParser::build() {
    try {
        canonicalCollection.clear();
        parsingTable.clear();
        buildCanonicalCollection();
        buildParsingTable();
        return true;
    } catch (const exception& e) {
        cerr << "Error building SLR(1) parser: " << e.what() << endl;
        return false;
    }
}

bool SLRParser::parse(const vector<Symbol>& input, ParseTree*& resultTree) {
    ParsingStack stack;
    stack.pushState(0);
    vector<TreeNode*> nodeStack;
    
    int inputIndex = 0;
    vector<Symbol> inputWithEnd = input;
    inputWithEnd.push_back(Grammar::END_MARKER);
    vector<string> traceStacks;
    vector<string> traceRemainingInputs;
    vector<string> traceActions;
    
    while (true) {
        int currentState = stack.topState();
        Symbol currentSymbol = inputWithEnd[inputIndex];
        
        Action action = parsingTable.getAction(currentState, currentSymbol);
        string actionText = action.toString();
        if (actionText.empty()) {
            actionText = "error";
        }

        string remainingInput = "";
        for (int i = inputIndex; i < inputWithEnd.size(); i++) {
            if (!remainingInput.empty()) {
                remainingInput += " ";
            }
            remainingInput += inputWithEnd[i].toString();
        }

        traceStacks.push_back(stack.toString());
        traceRemainingInputs.push_back(remainingInput);
        traceActions.push_back(actionText);
        
        if (action.type == SHIFT) {
            stack.pushSymbol(currentSymbol);
            stack.pushState(action.value);
            nodeStack.push_back(new TreeNode(currentSymbol));
            inputIndex++;
            
        } else if (action.type == REDUCE) {
            const Production& prod = grammar.getProductions()[action.value];
            int rhsSize = prod.isEmpty() ? 0 : prod.rightSide.size();
            
            // Pop 2 * rhsSize symbols
            for (int i = 0; i < 2 * rhsSize; i++) {
                stack.popState();
            }
            
            vector<TreeNode*> children;
            for (int i = 0; i < rhsSize; i++) {
                if (!nodeStack.empty()) {
                    children.push_back(nodeStack.back());
                    nodeStack.pop_back();
                }
            }
            reverse(children.begin(), children.end());

            TreeNode* parentNode = new TreeNode(prod.leftSide);
            if (prod.isEmpty()) {
                parentNode->addChild(new TreeNode(Symbol("epsilon", EPSILON)));
            } else {
                for (TreeNode* child : children) {
                    parentNode->addChild(child);
                }
            }

            int newState = stack.topState();
            int gotoState = parsingTable.getGoto(newState, prod.leftSide);
            
            stack.pushSymbol(prod.leftSide);
            stack.pushState(gotoState);
            nodeStack.push_back(parentNode);
            
        } else if (action.type == ACCEPT) {
            cout << "\n=== ACCEPT ===" << endl;
            cout << "String accepted successfully!" << endl;
            saveStackVisualization(traceStacks,
                                   traceRemainingInputs,
                                   traceActions,
                                   outputDirectory + "/slr_parsing_trace.png");
            
            TreeNode* finalRoot = nullptr;
            if (!nodeStack.empty()) {
                finalRoot = nodeStack.back();
                nodeStack.pop_back();
            } else {
                finalRoot = new TreeNode(grammar.getAugmentedStartSymbol());
            }

            resultTree = new ParseTree(finalRoot->symbol);
            resultTree->setRoot(finalRoot);
            return true;
            
        } else {
            cout << "\n=== ERROR ===" << endl;
            cout << "Parsing error at state " << currentState << " with symbol " << currentSymbol.toString() << endl;
            saveStackVisualization(traceStacks,
                                   traceRemainingInputs,
                                   traceActions,
                                   outputDirectory + "/slr_parsing_trace.png");
            for (TreeNode* node : nodeStack) {
                delete node;
            }
            return false;
        }
    }
}

void SLRParser::displayCanonicalCollection() const {
    cout << "\n=== SLR(1) Canonical Collection ===" << endl;
    
    for (int i = 0; i < canonicalCollection.size(); i++) {
        cout << "\nI" << i << ":" << endl;
        for (const LR0Item& item : canonicalCollection[i].items) {
            cout << "  " << item.toString() << endl;
        }
    }
}

void SLRParser::displayParsingTable() const {
    parsingTable.displayTable(grammar.getTerminals(), grammar.getNonTerminals());
}

void SLRParser::saveCanonicalCollectionToFile(const string& filename) const {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Cannot create file " << filename << endl;
        return;
    }
    
    for (int i = 0; i < canonicalCollection.size(); i++) {
        file << "I" << i << ":" << endl;
        for (const LR0Item& item : canonicalCollection[i].items) {
            file << "  " << item.toString() << endl;
        }
        file << endl;
    }
    
    file.close();
}

void SLRParser::saveParsingTableToFile(const string& filename) const {
    parsingTable.saveTableToFile(filename, grammar.getTerminals(), grammar.getNonTerminals());
}

void SLRParser::saveStateDiagramAsImage(const string& filename) const {
    // Create temporary .dot file
    string dotFilename = filename.substr(0, filename.find_last_of(".")) + ".dot";
    
    ofstream file(dotFilename);
    if (!file.is_open()) {
        cerr << "Error: Cannot create state diagram file " << dotFilename << endl;
        return;
    }
    
    file << "digraph SLRStateDiagram {" << endl;
    file << "  rankdir=LR;" << endl;
    file << "  node [shape=box, fontname=\"Courier New\", fontsize=9];" << endl;
    file << "  edge [fontname=\"Arial\", fontsize=8];" << endl;

    auto escapeDotLabel = [](const string& text) {
        string escaped = "";
        for (int i = 0; i < text.size(); i++) {
            char c = text[i];
            if (c == '\\' || c == '"') {
                escaped += '\\';
            }
            escaped += c;
        }
        return escaped;
    };
    
    // Create nodes for each state
    for (int i = 0; i < canonicalCollection.size(); i++) {
        string stateLabel = "I" + to_string(i) + "\\l";
        for (const LR0Item& item : canonicalCollection[i].items) {
            stateLabel += escapeDotLabel(item.toString()) + "\\l";
        }

        bool isAcceptState = (parsingTable.getAction(i, Grammar::END_MARKER).type == ACCEPT);
        bool hasCompletedItem = false;
        for (const LR0Item& item : canonicalCollection[i].items) {
            if (item.isDotAtEnd()) {
                hasCompletedItem = true;
                break;
            }
        }

        file << "  I" << i << " [label=\"" << stateLabel << "\"";
        if (isAcceptState) {
            file << ", peripheries=2, style=\"rounded,filled,bold\", fillcolor=\"palegreen\", color=\"darkgreen\"";
        } else if (hasCompletedItem) {
            file << ", peripheries=2, style=\"rounded,filled\", fillcolor=\"lightgoldenrod1\", color=\"goldenrod4\"";
        } else {
            file << ", style=\"rounded\"";
        }
        file << "];" << endl;
    }
    
    file << endl;
    
    // Create transitions
    set<pair<int, Symbol>> transitions;
    for (int i = 0; i < canonicalCollection.size(); i++) {
        // Check shift transitions
        for (const Symbol& term : grammar.getTerminals()) {
            Action act = parsingTable.getAction(i, term);
            if (act.type == SHIFT) {
                if (transitions.find(make_pair(i, term)) == transitions.end()) {
                    file << "  I" << i << " -> I" << act.value 
                         << " [label=\"" << term.toString() << "\"];" << endl;
                    transitions.insert(make_pair(i, term));
                }
            }
        }
        
        // Check goto transitions
        for (const Symbol& nonTerm : grammar.getNonTerminals()) {
            if (nonTerm == grammar.getAugmentedStartSymbol()) continue;
            int go = parsingTable.getGoto(i, nonTerm);
            if (go >= 0) {
                if (transitions.find(make_pair(i, nonTerm)) == transitions.end()) {
                    file << "  I" << i << " -> I" << go 
                         << " [label=\"" << nonTerm.toString() << "\", style=dashed];" << endl;
                    transitions.insert(make_pair(i, nonTerm));
                }
            }
        }
    }
    
    file << "}" << endl;
    file.close();
    
    // Convert to PNG and delete dot file
    GraphvizConverter::convertToPNG(dotFilename, filename);
}

void SLRParser::saveParsingTableAsImage(const string& filename) const {
    // Create temporary .dot file
    string dotFilename = filename.substr(0, filename.find_last_of(".")) + ".dot";
    
    ofstream file(dotFilename);
    if (!file.is_open()) {
        cerr << "Error: Cannot create parsing table diagram " << dotFilename << endl;
        return;
    }
    
    file << "digraph ParsingTable {" << endl;
    file << "  rankdir=TB;" << endl;
    file << "  node [shape=plaintext, fontname=\"Courier\", fontsize=9];" << endl;
    
    // Create table as HTML-like node
    file << "  table [label=<" << endl;
    file << "    <TABLE BORDER=\"1\" CELLBORDER=\"1\" CELLSPACING=\"0\">" << endl;
    
    // Header row
    file << "      <TR>";
    file << "<TD BGCOLOR=\"lightblue\" PORT=\"state\"><B>State</B></TD>";
    
    for (const Symbol& term : grammar.getTerminals()) {
        file << "<TD BGCOLOR=\"lightblue\"><B>" << term.toString() << "</B></TD>";
    }
    
    for (const Symbol& nonTerm : grammar.getNonTerminals()) {
        if (nonTerm.name.find("Prime") == string::npos) {
            file << "<TD BGCOLOR=\"lightgreen\"><B>" << nonTerm.toString() << "</B></TD>";
        }
    }
    file << "</TR>" << endl;
    
    // Data rows
    for (int i = 0; i < parsingTable.getNumStates(); i++) {
        file << "      <TR>";
        file << "<TD BGCOLOR=\"lightyellow\"><B>I" << i << "</B></TD>";
        
        for (const Symbol& term : grammar.getTerminals()) {
            Action act = parsingTable.getAction(i, term);
            file << "<TD>";
            if (act.type != NONE) {
                file << act.toString();
            }
            file << "</TD>";
        }
        
        for (const Symbol& nonTerm : grammar.getNonTerminals()) {
            if (nonTerm.name.find("Prime") == string::npos) {
                int go = parsingTable.getGoto(i, nonTerm);
                file << "<TD>";
                if (go >= 0) {
                    file << go;
                }
                file << "</TD>";
            }
        }
        file << "</TR>" << endl;
    }
    
    file << "    </TABLE>" << endl;
    file << "  >];" << endl;
    file << "}" << endl;
    file.close();
    
    // Convert to PNG and delete dot file
    GraphvizConverter::convertToPNG(dotFilename, filename);
}

void SLRParser::saveStackVisualization(const vector<string>& stackStates,
                                       const vector<string>& remainingInputs,
                                       const vector<string>& actions,
                                       const string& filename) const {
    // Create temporary .dot file
    string dotFilename = filename.substr(0, filename.find_last_of(".")) + ".dot";
    
    ofstream file(dotFilename);
    if (!file.is_open()) {
        cerr << "Error: Cannot create stack visualization " << dotFilename << endl;
        return;
    }
    
    file << "digraph ParsingTrace {" << endl;
    file << "  rankdir=TB;" << endl;
    file << "  node [shape=plaintext, fontname=\"Courier\", fontsize=10];" << endl;
    file << "  label=\"Parsing Trace\";" << endl;

    auto escapeHtml = [](const string& text) {
        string escaped = "";
        for (int i = 0; i < text.size(); i++) {
            char c = text[i];
            if (c == '&') {
                escaped += "&amp;";
            } else if (c == '<') {
                escaped += "&lt;";
            } else if (c == '>') {
                escaped += "&gt;";
            } else if (c == '"') {
                escaped += "&quot;";
            } else {
                escaped += c;
            }
        }
        return escaped;
    };

    file << "  trace [label=<" << endl;
    file << "    <TABLE BORDER=\"1\" CELLBORDER=\"1\" CELLSPACING=\"0\">" << endl;
    file << "      <TR>"
         << "<TD BGCOLOR=\"lightblue\"><B>Stack State</B></TD>"
         << "<TD BGCOLOR=\"lightblue\"><B>Remaining String</B></TD>"
         << "<TD BGCOLOR=\"lightblue\"><B>Action Taken</B></TD>"
         << "</TR>" << endl;

    int rowCount = stackStates.size();
    for (int i = 0; i < rowCount; i++) {
        file << "      <TR>"
             << "<TD ALIGN=\"LEFT\">" << escapeHtml(stackStates[i]) << "</TD>"
             << "<TD ALIGN=\"LEFT\">" << escapeHtml(remainingInputs[i]) << "</TD>"
             << "<TD ALIGN=\"LEFT\">" << escapeHtml(actions[i]) << "</TD>"
             << "</TR>" << endl;
    }

    file << "    </TABLE>" << endl;
    file << "  >];" << endl;
    
    file << "}" << endl;
    file.close();
    
    // Convert to PNG and delete dot file
    GraphvizConverter::convertToPNG(dotFilename, filename);
}
