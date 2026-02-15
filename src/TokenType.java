/**
 * TokenType enumeration for the simplified scanner
 * CS4031 - Compiler Construction Assignment 01
 * 
 * Supports only: Int lit, Float lit, Identifier, Single-line comment, 
 * Boolean, Punctuation, Whitespace
 */
public enum TokenType {
    // Identifiers (Section 3.2)
    // [A-Z][a-z0-9_]{0,30}
    IDENTIFIER,
    
    // Literals
    INT_LIT,        // [+-]?[0-9]+
    FLOAT_LIT,      // [+-]?[0-9]+\.[0-9]{1,6}([eE][+-]?[0-9]+)?
    BOOLEAN_LIT,    // true|false
    
    // Punctuators (Section 3.9)
    LPAREN,         // (
    RPAREN,         // )
    LBRACE,         // {
    RBRACE,         // }
    LBRACKET,       // [
    RBRACKET,       // ]
    COMMA,          // ,
    SEMICOLON,      // ;
    COLON,          // :
    
    // Comments (Section 3.10)
    SINGLE_LINE_COMMENT,  // ##[^\n]*
    
    // Whitespace (Section 3.11)
    WHITESPACE,
    
    // Special
    EOF,
    ERROR
}
