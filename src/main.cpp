#include <iostream>
#include <vector>
#include <fstream>
#include <io.h>
#include <direct.h>
#include "Grammar.h"
#include "SLRParser.h"
#include "LR1Parser.h"

using namespace std;

bool directoryExists(const string& path) {
    return _access(path.c_str(), 0) == 0;
}

bool ensureDirectory(const string& path) {
    if (directoryExists(path)) {
        return true;
    }
    return _mkdir(path.c_str()) == 0;
}

string resolveOutputDirectory() {
    vector<string> projectRoots;
    projectRoots.push_back(".");
    projectRoots.push_back("..");
    projectRoots.push_back("../..");

    for (int i = 0; i < projectRoots.size(); i++) {
        string root = projectRoots[i];
        bool hasSrc = directoryExists(root + "/src");
        bool hasInput = directoryExists(root + "/input");
        if (hasSrc && hasInput) {
            string outputPath = root + "/output";
            if (ensureDirectory(outputPath)) {
                return outputPath;
            }
        }
    }

    // Final fallback (if project root was not detected)
    return "output";
}

// Display menu
void displayMenu() {
    cout << "\n=== Parser Comparison Tool ===" << endl;
    cout << "1. Load Grammar" << endl;
    cout << "2. Build SLR(1) Parser" << endl;
    cout << "3. Build LR(1) Parser" << endl;
    cout << "4. Parse String (SLR)" << endl;
    cout << "5. Parse String (LR1)" << endl;
    cout << "6. Display Canonical Collections" << endl;
    cout << "7. Display Parsing Tables" << endl;
    cout << "8. Generate Visualizations (PNG)" << endl;
    cout << "9. Compare Parsers" << endl;
    cout << "10. Exit" << endl;
    cout << "Enter choice: ";
}

// Parse input string into symbols
vector<Symbol> parseInputString(const string& input, const set<Symbol>& terminals) {
    vector<Symbol> result;
    
    for (char ch : input) {
        if (ch == ' ') continue;
        
        // Try to match with terminals
        bool found = false;
        for (const Symbol& term : terminals) {
            if (term.name.length() == 1 && term.name[0] == ch) {
                result.push_back(term);
                found = true;
                break;
            }
        }
        
        if (!found) {
            // Try multi-character terminals
            if (ch == 'i' && input.find("id") != string::npos) {
                result.push_back(Symbol("id", TERMINAL));
            }
        }
    }
    
    return result;
}

int main() {
    Grammar grammar;
    SLRParser* slrParser = nullptr;
    LR1Parser* lr1Parser = nullptr;
    string outputDir = resolveOutputDirectory();
    bool grammarLoaded = false;
    bool slrBuilt = false;
    bool lr1Built = false;
    
    while (true) {
        displayMenu();
        int choice;
        cin >> choice;
        cin.ignore();
        
        switch (choice) {
            case 1: {
                cout << "Enter grammar file path: ";
                string filename;
                getline(cin, filename);
                
                if (grammar.loadFromFile(filename)) {
                    grammar.augmentGrammar();
                    grammar.displayAugmentedGrammar();
                    grammar.displayFirstAndFollowSets();
                    grammarLoaded = true;
                    
                    cout << "\nGrammar loaded and augmented successfully!" << endl;
                    grammar.saveAugmentedGrammarToFile(outputDir + "/augmented_grammar.txt");
                } else {
                    cout << "Failed to load grammar!" << endl;
                }
                break;
            }
            
            case 2: {
                if (!grammarLoaded) {
                    cout << "Please load grammar first!" << endl;
                    break;
                }
                
                slrParser = new SLRParser(grammar);
                slrParser->setOutputDirectory(outputDir);
                if (slrParser->build()) {
                    slrBuilt = true;
                    cout << "\nSLR(1) parser built successfully!" << endl;
                    cout << "Number of states: " << slrParser->getNumberOfStates() << endl;
                    slrParser->saveCanonicalCollectionToFile(outputDir + "/slr_items.txt");
                    slrParser->saveParsingTableToFile(outputDir + "/slr_parsing_table.txt");
                } else {
                    cout << "Failed to build SLR(1) parser!" << endl;
                    delete slrParser;
                    slrParser = nullptr;
                }
                break;
            }
            
            case 3: {
                if (!grammarLoaded) {
                    cout << "Please load grammar first!" << endl;
                    break;
                }
                
                lr1Parser = new LR1Parser(grammar);
                lr1Parser->setOutputDirectory(outputDir);
                if (lr1Parser->build()) {
                    lr1Built = true;
                    cout << "\nLR(1) parser built successfully!" << endl;
                    cout << "Number of states: " << lr1Parser->getNumberOfStates() << endl;
                    lr1Parser->saveCanonicalCollectionToFile(outputDir + "/lr1_items.txt");
                    lr1Parser->saveParsingTableToFile(outputDir + "/lr1_parsing_table.txt");
                } else {
                    cout << "Failed to build LR(1) parser!" << endl;
                    delete lr1Parser;
                    lr1Parser = nullptr;
                }
                break;
            }
            
            case 4: {
                if (!slrBuilt) {
                    cout << "Please build SLR(1) parser first!" << endl;
                    break;
                }
                
                cout << "Enter input string to parse: ";
                string input;
                getline(cin, input);
                
                vector<Symbol> symbols = parseInputString(input, grammar.getTerminals());
                ParseTree* tree = nullptr;
                
                if (slrParser->parse(symbols, tree)) {
                    if (tree) {
                        tree->displayTree();
                        tree->saveTreeAsImage(outputDir + "/slr_parse_tree.png");
                        delete tree;
                    }
                } else {
                    cout << "Parsing failed!" << endl;
                }
                break;
            }
            
            case 5: {
                if (!lr1Built) {
                    cout << "Please build LR(1) parser first!" << endl;
                    break;
                }
                
                cout << "Enter input string to parse: ";
                string input;
                getline(cin, input);
                
                vector<Symbol> symbols = parseInputString(input, grammar.getTerminals());
                ParseTree* tree = nullptr;
                
                if (lr1Parser->parse(symbols, tree)) {
                    if (tree) {
                        tree->displayTree();
                        tree->saveTreeAsImage(outputDir + "/lr1_parse_tree.png");
                        delete tree;
                    }
                } else {
                    cout << "Parsing failed!" << endl;
                }
                break;
            }
            
            case 6: {
                if (slrBuilt) {
                    slrParser->displayCanonicalCollection();
                }
                if (lr1Built) {
                    lr1Parser->displayCanonicalCollection();
                }
                if (!slrBuilt && !lr1Built) {
                    cout << "Please build at least one parser first!" << endl;
                }
                break;
            }
            
            case 7: {
                if (slrBuilt) {
                    cout << "\n========== SLR(1) Parsing Table ==========" << endl;
                    slrParser->displayParsingTable();
                }
                if (lr1Built) {
                    cout << "\n========== LR(1) Parsing Table ==========" << endl;
                    lr1Parser->displayParsingTable();
                }
                if (!slrBuilt && !lr1Built) {
                    cout << "Please build at least one parser first!" << endl;
                }
                break;
            }
            
            case 8: {
                cout << "Generating PNG visualizations in " << outputDir << "/ ..." << endl;
                if (slrBuilt) {
                    slrParser->saveStateDiagramAsImage(outputDir + "/slr_dfa.png");
                    slrParser->saveParsingTableAsImage(outputDir + "/slr_parsing_table.png");
                    cout << "  SLR(1) PNG files generated" << endl;
                }
                if (lr1Built) {
                    lr1Parser->saveStateDiagramAsImage(outputDir + "/lr1_dfa.png");
                    lr1Parser->saveParsingTableAsImage(outputDir + "/lr1_parsing_table.png");
                    cout << "  LR(1) PNG files generated" << endl;
                }
                cout << "Done. No dot/graphviz source files are kept in output/." << endl;
                break;
            }
            
            case 9: {
                if (!slrBuilt || !lr1Built) {
                    cout << "Please build both parsers first!" << endl;
                    break;
                }
                
                cout << "\n=== Parser Comparison ===" << endl;
                cout << "SLR(1) states: " << slrParser->getNumberOfStates() << endl;
                cout << "LR(1) states: " << lr1Parser->getNumberOfStates() << endl;
                cout << "Difference: " << (lr1Parser->getNumberOfStates() - slrParser->getNumberOfStates()) 
                     << " states" << endl;
                break;
            }
            
            case 10: {
                cout << "Exiting..." << endl;
                if (slrParser) delete slrParser;
                if (lr1Parser) delete lr1Parser;
                return 0;
            }
            
            default:
                cout << "Invalid choice!" << endl;
        }
    }
    
    return 0;
}
