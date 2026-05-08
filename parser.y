%code requires {
#include "AST.h"
#include <string>
#include <utility>
}

%{
#include "AST.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <utility>

using namespace std;

extern int yylex();
extern int yyparse();
extern FILE* yyin;
extern int line_num;
extern int col_num;

void yyerror(const char* s) {
    cerr << "Error: " << s << " at line " << line_num << ", column " << col_num << endl;
    exit(1);
}

ASTNode* root;
bool printAST = false;
%}

%union {
    ASTNode* node;
    string* sval;
    bool bval;
    pair<string, ASTNode*>* member;
}

%token LBRACE RBRACE LBRACKET RBRACKET COLON COMMA
%token T_STRING T_NUMBER T_BOOL T_NULL
%type <node> value object array members elements
%type <sval> T_STRING T_NUMBER
%type <bval> T_BOOL
%type <member> member

%%

input:
    value { root = $1; }
    ;

value:
    object     { $$ = $1; }
    | array    { $$ = $1; }
    | T_STRING   { $$ = new StringNode(*$1); delete $1; }
    | T_NUMBER   { $$ = new NumberNode(*$1); delete $1; }
    | T_BOOL     { $$ = new BoolNode($1); }
    | T_NULL { $$ = new NullNode(); }
    ;

object:
    LBRACE RBRACE { $$ = new ObjectNode(); }
    | LBRACE members RBRACE { $$ = $2; }
    ;

members:
    member { 
        $$ = new ObjectNode();
        ((ObjectNode*)$$)->addMember($1->first, $1->second);
        delete $1;
    }
    | members COMMA member {
        $$ = $1;
        ((ObjectNode*)$$)->addMember($3->first, $3->second);
        delete $3;
    }
    ;

member:
    T_STRING COLON value {
        $$ = new pair<string, ASTNode*>(*$1, $3);
        delete $1;
    }
    ;

array:
    LBRACKET RBRACKET { $$ = new ArrayNode(); }
    | LBRACKET elements RBRACKET { $$ = $2; }
    ;

elements:
    value {
        $$ = new ArrayNode();
        ((ArrayNode*)$$)->addElement($1);
    }
    | elements COMMA value {
        $$ = $1;
        ((ArrayNode*)$$)->addElement($3);
    }
    ;

%%

int main(int argc, char** argv) {
    // Check for --print-ast flag
    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == "--print-ast") {
            printAST = true;
        }
    }

    yyparse();

    if (root) {
        if (printAST) {
            cout << "AST Structure:" << endl;
            root->print(0);
        }
        cout << "<root>" << endl;
        root->toXML(cout, "", 0);
        cout << "</root>" << endl;
        delete root;
    }

    return 0;
}
