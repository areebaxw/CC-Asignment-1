#pragma once
#include <string>
#include <set>

using namespace std;

class ErrorHandler {
public:
    int errorCount = 0;

    // Report a generic parse error with line:column info
    void reportError(int line, int col,
                     const string& expected,
                     const string& found);

    // Report "empty table entry" error
    void reportMissingProduction(int line, int col,
                                  const string& nt,
                                  const string& token);

    // Report premature end of input
    void reportPrematureEnd(int line, int col, const string& stackTop);

    // Panic-mode recovery message
    void reportRecovery(const string& skipped,
                        const string& syncSym);

    bool hasErrors() const { return errorCount > 0; }
};
