#include "GraphvizConverter.h"
#include <iostream>
#include <cstdlib>
#include <vector>
#include <io.h>
#include <cstdio>

namespace {
string trimSurroundingQuotes(const string& value) {
    string result = value;
    while (!result.empty() && result.front() == '"') {
        result.erase(0, 1);
    }
    while (!result.empty() && result.back() == '"') {
        result.pop_back();
    }
    return result;
}

string resolveDotExecutable() {
    // 1) Respect explicit override first.
    const char* envDot = getenv("GRAPHVIZ_DOT");
    if (envDot != nullptr) {
        string dotPath = trimSurroundingQuotes(string(envDot));
        if (!dotPath.empty() && _access(dotPath.c_str(), 0) == 0) {
            return dotPath;
        }
    }

    // 2) Check PATH.
    if (system("where dot >nul 2>&1") == 0) {
        return "dot";
    }

    // 3) Check common Graphviz install locations on Windows.
    vector<string> candidates;
    candidates.push_back("C:\\Program Files\\Graphviz\\bin\\dot.exe");
    candidates.push_back("C:\\Program Files (x86)\\Graphviz\\bin\\dot.exe");
    candidates.push_back("C:\\Program Files\\Graphviz2.38\\bin\\dot.exe");
    candidates.push_back("C:\\Program Files (x86)\\Graphviz2.38\\bin\\dot.exe");

    for (int i = 0; i < candidates.size(); i++) {
        if (_access(candidates[i].c_str(), 0) == 0) {
            return "\"" + candidates[i] + "\"";
        }
    }

    return "";
}
}

bool GraphvizConverter::isGraphvizInstalled() {
    string dotExecutable = resolveDotExecutable();
    return !dotExecutable.empty();
}

bool GraphvizConverter::convertToPNG(const string& dotFilePath, const string& pngFilePath) {
    string dotExecutable = trimSurroundingQuotes(resolveDotExecutable());

    if (dotExecutable.empty()) {
        cerr << "Warning: Graphviz not installed. Skipping PNG conversion." << endl;
        cerr << "Install graphviz from: https://graphviz.org/download/" << endl;
        cerr << "Tip: If installed, set GRAPHVIZ_DOT to full dot.exe path." << endl;
        return false;
    }
    
    // Build command: dot -Tpng input.dot -o output.png
    string command = "cmd /c \"\"" + dotExecutable + "\" -Tpng \"" + dotFilePath + "\" -o \"" + pngFilePath + "\"\"";
    
    int result = system(command.c_str());
    
    if (result == 0) {
        // Delete the dot file after successful conversion.
        // Use std::remove for reliable deletion regardless of shell context.
        remove(dotFilePath.c_str());
        
        cout << "✓ Created: " << pngFilePath << endl;
        return true;
    } else {
        cerr << "Error: Failed to convert " << dotFilePath << " to PNG" << endl;
        cerr << "Command used: " << command << endl;
        return false;
    }
}
