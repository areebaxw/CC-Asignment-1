#include "error_handler.h"
#include <iostream>

using namespace std;

void ErrorHandler::reportError(int step,
                                const string& expected,
                                const string& found) {
    errorCount++;
    cout << "  *** ERROR (step " << step << "): "
              << "expected '" << expected
              << "' but found '" << found << "'\n";
}

void ErrorHandler::reportMissingProduction(int step,
                                            const string& nt,
                                            const string& token) {
    errorCount++;
    cout << "  *** ERROR (step " << step << "): "
              << "no production for [" << nt << ", '" << token << "']\n"
              << "             (empty table entry)\n";
}

void ErrorHandler::reportPrematureEnd(int step,
                                       const string& stackTop) {
    errorCount++;
    cout << "  *** ERROR (step " << step << "): "
              << "unexpected end of input, stack still has '" << stackTop << "'\n";
}

void ErrorHandler::reportRecovery(const string& skipped,
                                   const string& syncSym) {
    cout << "         (recovery) skipping '" << skipped
              << "', resuming at '" << syncSym << "'\n";
}
