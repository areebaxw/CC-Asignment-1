/**
 * JFlex Scanner Specification (Simplified)
 * CS4031 - Compiler Construction Assignment 01
 * 
 * Supports only: Int lit, Float lit, Identifier, Single-line comment, 
 * Boolean, Punctuation, Whitespace
 * 
 * To generate Yylex.java: jflex Scanner.flex
 */

%%

%public
%class Yylex
%unicode
%line
%column
%type Token

%{
    private SymbolTable symbolTable = new SymbolTable();
    private ErrorHandler errorHandler = new ErrorHandler();
    private int commentCount = 0;
    
    public SymbolTable getSymbolTable() { return symbolTable; }
    public ErrorHandler getErrorHandler() { return errorHandler; }
    public int getCommentCount() { return commentCount; }
    
    private Token token(TokenType type) {
        return new Token(type, yytext(), yyline + 1, yycolumn + 1);
    }
%}

/* Regular Definitions */
LineTerminator = \r|\n|\r\n
WhiteSpace = [ \t\r]+

/* Identifier: [A-Z][a-z0-9_]{0,30} */
Identifier = [A-Z][a-z0-9_]{0,30}
InvalidIdentifier = [a-z][a-zA-Z0-9_]*

/* Integer: [0-9]+ */
IntLiteral = [0-9]+

/* Float: [0-9]+\.[0-9]{1,6}([eE][+-]?[0-9]+)? */
FloatLiteral = [0-9]+\.[0-9]{1,6}([eE][+-]?[0-9]+)?

/* Boolean: true|false */
BooleanLiteral = "true"|"false"

/* Single-line comment: ##[^\n]* */
SingleLineComment = ##[^\n]*

%%

/* Whitespace - skip */
{WhiteSpace}           { /* skip */ }
{LineTerminator}       { /* skip */ }

/* Single-line comment - skip but count */
{SingleLineComment}    { commentCount++; /* skip */ }

/* Boolean literals */
{BooleanLiteral}       { return token(TokenType.BOOLEAN_LIT); }

/* Float literals (before Integer for longest match) */
{FloatLiteral}         { return token(TokenType.FLOAT_LIT); }

/* Integer literals */
{IntLiteral}           { return token(TokenType.INT_LIT); }

/* Identifiers */
{Identifier}           { 
                         symbolTable.addSymbol(yytext(), null, yyline + 1, yycolumn + 1);
                         return token(TokenType.IDENTIFIER); 
                       }

/* Punctuators */
"("                    { return token(TokenType.LPAREN); }
")"                    { return token(TokenType.RPAREN); }
"{"                    { return token(TokenType.LBRACE); }
"}"                    { return token(TokenType.RBRACE); }
"["                    { return token(TokenType.LBRACKET); }
"]"                    { return token(TokenType.RBRACKET); }
","                    { return token(TokenType.COMMA); }
";"                    { return token(TokenType.SEMICOLON); }
":"                    { return token(TokenType.COLON); }

/* Invalid identifiers (starting with lowercase) */
{InvalidIdentifier}    { 
                         errorHandler.invalidIdentifier(yytext(), yyline + 1, yycolumn + 1);
                         return token(TokenType.ERROR); 
                       }

/* Invalid characters */
.                      { 
                         errorHandler.invalidCharacter(yytext().charAt(0), yyline + 1, yycolumn + 1);
                         return token(TokenType.ERROR); 
                       }

<<EOF>>                { return token(TokenType.EOF); }
