%{
#include "parser.tab.h"
#include <stdlib.h>
#include <string>
#include <iostream>
#ifdef _WIN32
#include <io.h>
#define fileno _fileno
#endif
using namespace std;

int line_num = 1;
int col_num = 1;

void update_location() {
    for (int i = 0; i < yyleng; i++) {
        if (yytext[i] == '\n') {
            line_num++;
            col_num = 1;
        } else {
            col_num++;
        }
    }
}
%}

%%
"{"             { update_location(); return LBRACE; }
"}"             { update_location(); return RBRACE; }
"["             { update_location(); return LBRACKET; }
"]"             { update_location(); return RBRACKET; }
":"             { update_location(); return COLON; }
","             { update_location(); return COMMA; }

\"([^\"\\]|\\.)*\" {
    update_location();
    string str(yytext + 1, yyleng - 2);
    string unescaped;
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == '\\' && i + 1 < str.length()) {
            switch (str[i + 1]) {
                case '"': unescaped += '"'; i++; break;
                case '\\': unescaped += '\\'; i++; break;
                case 'n': unescaped += '\n'; i++; break;
                case 't': unescaped += '\t'; i++; break;
                case 'u':
                    if (i + 5 < str.length()) {
                        string hex = str.substr(i + 2, 4);
                        unsigned int codepoint;
                        sscanf(hex.c_str(), "%x", &codepoint);
                        if (codepoint <= 0x7F) {
                            unescaped += (char)codepoint;
                        } else if (codepoint <= 0x7FF) {
                            unescaped += (char)(0xC0 | (codepoint >> 6));
                            unescaped += (char)(0x80 | (codepoint & 0x3F));
                        } else if (codepoint <= 0xFFFF) {
                            unescaped += (char)(0xE0 | (codepoint >> 12));
                            unescaped += (char)(0x80 | ((codepoint >> 6) & 0x3F));
                            unescaped += (char)(0x80 | (codepoint & 0x3F));
                        } else {
                            // 4-byte UTF-8 (for codepoints > 0xFFFF)
                            unescaped += (char)(0xF0 | (codepoint >> 18));
                            unescaped += (char)(0x80 | ((codepoint >> 12) & 0x3F));
                            unescaped += (char)(0x80 | ((codepoint >> 6) & 0x3F));
                            unescaped += (char)(0x80 | (codepoint & 0x3F));
                        }
                        i += 5;
                    } else {
                        cerr << "Error: incomplete Unicode escape at line " << line_num << ", column " << col_num << endl;
                        exit(1);
                    }
                    break;
                default:
                    cerr << "Error: invalid escape sequence \\" << str[i + 1] << " at line " << line_num << ", column " << col_num << endl;
                    exit(1);
            }
        } else {
            unescaped += str[i];
        }
    }
    yylval.sval = new string(unescaped);
    return T_STRING;
}

-?[0-9]+(\.[0-9]+)?([eE][+-]?[0-9]+)? {
    update_location();
    yylval.sval = new string(yytext);
    return T_NUMBER;
}

"true"          { update_location(); yylval.bval = true; return T_BOOL; }
"false"         { update_location(); yylval.bval = false; return T_BOOL; }
"null"          { update_location(); return T_NULL; }

[ \t\r\n]+      { update_location(); /* skip whitespace */ }

.               {
    update_location();
    cerr << "Error: unexpected character '" << yytext[0] << "' at line " << line_num << ", column " << col_num << endl;
    exit(1);
}

%%
int yywrap() {
    return 1;
}
