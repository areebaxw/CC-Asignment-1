/**
 * Token class representing a lexical token
 * CS4031 - Compiler Construction Assignment 01
 * Output format: <TOKEN_TYPE, "lexeme", Line: n, Col: m>
 */
public class Token {
    private TokenType type;
    private String lexeme;
    private int lineNumber;
    private int columnNumber;
    
    public Token(TokenType type, String lexeme, int lineNumber, int columnNumber) {
        this.type = type;
        this.lexeme = lexeme;
        this.lineNumber = lineNumber;
        this.columnNumber = columnNumber;
    }
    
    public TokenType getType() { return type; }
    public String getLexeme() { return lexeme; }
    public int getLineNumber() { return lineNumber; }
    public int getColumnNumber() { return columnNumber; }
    
    @Override
    public String toString() {
        return "<" + type + ", \"" + escapeString(lexeme) + "\", Line: " + lineNumber + ", Col: " + columnNumber + ">";
    }
    
    private String escapeString(String str) {
        if (str == null) return "";
        return str.replace("\\", "\\\\")
                  .replace("\"", "\\\"")
                  .replace("\n", "\\n")
                  .replace("\r", "\\r")
                  .replace("\t", "\\t");
    }
}
