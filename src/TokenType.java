/**
 * Supports only: Int lit, Float lit, Identifier, Single-line comment, 
 * Boolean, Punctuation, Whitespace
 */
public enum TokenType {
    // Identifiers 
    // [A-Z][a-z0-9_]{0,30}
    IDENTIFIER,
    

   
    // Literals
    INT_LIT,        // [+-]?[0-9]+
    FLOAT_LIT,      // [+-]?[0-9]+\.[0-9]{1,6}([eE][+-]?[0-9]+)?
    BOOLEAN_LIT,    // true|false
    
    
 
    // Punctuators 
    LPAREN,         // (
    RPAREN,         // )
    LBRACE,         // {
    RBRACE,         // }
    LBRACKET,       // [
    RBRACKET,       // ]
    COMMA,          // ,
    SEMICOLON,      // ;
    COLON,          // :
    
    // Comments 
    SINGLE_LINE_COMMENT,  // ##[^\n]*
    
    // Whitespace 
    WHITESPACE,
    
    // Special
    EOF,
    ERROR
}
