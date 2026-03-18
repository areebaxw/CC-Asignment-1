#include "error_handler.h"
#include <iostream>

using namespace std;

void ErrorHandler::reportError(int line, int col,
                                const string& expected,
                                const string& found) {
    errorCount++;
    cout << "  *** ERROR [" << line << ":" << col << "]: "
         << "expected '" << expected
         << "' but found '" << found << "'\n";
}

void ErrorHandler::reportMissingProduction(int line, int col,
                                            const string& nt,
                                            const string& token) {
    errorCount++;
    cout << "  *** ERROR [" << line << ":" << col << "]: "
         << "no production for [" << nt << ", '" << token << "']\n"
         << "             (empty table entry)\n";
}

void ErrorHandler::reportPrematureEnd(int line, int col,
                                       const string& stackTop) {
    errorCount++;
    cout << "  *** ERROR [" << line << ":" << col << "]: "
         << "unexpected end of input, stack still has '" << stackTop << "'\n";
}

void ErrorHandler::reportRecovery(const string& skipped,
                                   const string& syncSym) {
    cout << "         (recovery) skipping '" << skipped
         << "', resuming at '" << syncSym << "'\n";
}
