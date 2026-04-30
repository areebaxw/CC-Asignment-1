%{
#include "AST.h"
#include <iostream>
#include <cstdlib>

using namespace std;

extern int yylex();
extern int yyparse();
extern FILE* yyin;

void yyerror(const char* s) {
    cerr << "Error: " << s << endl;
    exit(1);
}

ASTNode* root;
%}

%union {
    ASTNode* node;
    string* sval;
    bool bval;
}

%token LBRACE RBRACE LBRACKET RBRACKET COLON COMMA
%token STRING NUMBER BOOL NULL_TOKEN
%type <node> value object array
%type <sval> STRING
%type <bval> BOOL

%%

input:
    value { root = $1; }
    ;

value:
    object     { $$ = $1; }
    | array    { $$ = $1; }
    | STRING   { $$ = new StringNode(*$1); delete $1; }
    | NUMBER   { $$ = new NumberNode(*$1); delete $1; }
    | BOOL     { $$ = new BoolNode($1); }
    | NULL_TOKEN { $$ = new NullNode(); }
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
    STRING COLON value {
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

int main() {
    yyparse();
    
    if (root) {
        cout << "<root>" << endl;
        root->toXML(cout, "", 1);
        cout << "</root>" << endl;
        delete root;
    }
    
    return 0;
}
