#pragma once
#include <string>
#include <set>

using namespace std;

class ErrorHandler {
public:
    int errorCount = 0;

    // Report a generic parse error
    void reportError(int step,
                     const string& expected,
                     const string& found);

    // Report "empty table entry" error
    void reportMissingProduction(int step,
                                  const string& nt,
                                  const string& token);

    // Report premature end of input
    void reportPrematureEnd(int step, const string& stackTop);

    // Panic-mode recovery message
    void reportRecovery(const string& skipped,
                        const string& syncSym);

    bool hasErrors() const { return errorCount > 0; }
};
