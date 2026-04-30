%{
#include "parser.tab.h"
#include <stdlib.h>
%}

%%
"{"             { return LBRACE; }
"}"             { return RBRACE; }
"["             { return LBRACKET; }
"]"             { return RBRACKET; }
":"             { return COLON; }
","             { return COMMA; }

\"([^\"\\]|\\.)\" {
    string str(yytext + 1, yyleng - 2);
    string unescaped;
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == '\\' && i + 1 < str.length()) {
            switch (str[i + 1]) {
                case '"': unescaped += '"'; i++; break;
                case '\\': unescaped += '\\'; i++; break;
                case 'n': unescaped += '\n'; i++; break;
                case 't': unescaped += '\t'; i++; break;
                default: 
                    cerr << "Error: invalid escape sequence \\" << str[i + 1] << endl;
                    exit(1);
            }
        } else {
            unescaped += str[i];
        }
    }
    yylval.sval = new string(unescaped);
    return STRING;
}

-?[0-9]+(\.[0-9]+)?([eE][+-]?[0-9]+)? {
    yylval.sval = new string(yytext);
    return NUMBER;
}

"true"          { yylval.bval = true; return BOOL; }
"false"         { yylval.bval = false; return BOOL; }
"null"          { return NULL_TOKEN; }

[ \t\r\n]+      { /* skip whitespace */ }

.               {
    cerr << "Error: unexpected character '" << yytext[0] << "'" << endl;
    exit(1);
}

%%
int yywrap() {
    return 1;
}
