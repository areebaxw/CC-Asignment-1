import java.util.*;

/**
 * ErrorHandler - Handles lexical errors during scanning
 * CS4031 - Compiler Construction Assignment 01
 * 
 * Error Types: Invalid characters, Malformed literals, Invalid identifiers
 * Error Reporting: Error type, line, column, lexeme, reason
 * Error Recovery: Skip to next valid token, continue scanning
 */
public class ErrorHandler {
    
    public enum ErrorType {
        INVALID_CHARACTER("Invalid Character"),
        MALFORMED_NUMBER("Malformed Number"),
        INVALID_IDENTIFIER("Invalid Identifier"),
        IDENTIFIER_TOO_LONG("Identifier Too Long"),
        TOO_MANY_DECIMALS("Too Many Decimal Places");
        
        private final String description;
        ErrorType(String description) { this.description = description; }
        public String getDescription() { return description; }
    }
    
    public static class LexicalError {
        private ErrorType type;
        private int line;
        private int column;
        private String lexeme;
        private String reason;
        
        public LexicalError(ErrorType type, int line, int column, String lexeme, String reason) {
            this.type = type;
            this.line = line;
            this.column = column;
            this.lexeme = lexeme;
            this.reason = reason;
        }
        
        public ErrorType getType() { return type; }
        public int getLine() { return line; }
        public int getColumn() { return column; }
        public String getLexeme() { return lexeme; }
        public String getReason() { return reason; }
        
        @Override
        public String toString() {
            return String.format("[%s] Line: %d, Col: %d, Lexeme: \"%s\", Reason: %s",
                    type.getDescription(), line, column, lexeme, reason);
        }
    }
    
    private List<LexicalError> errors;
    
    public ErrorHandler() {
        this.errors = new ArrayList<>();
    }
    
    public void reportError(ErrorType type, int line, int column, String lexeme, String reason) {
        LexicalError error = new LexicalError(type, line, column, lexeme, reason);
        errors.add(error);
        System.err.println("ERROR: " + error);
    }
    
    public void invalidCharacter(char c, int line, int column) {
        reportError(ErrorType.INVALID_CHARACTER, line, column, String.valueOf(c),
            "Character '" + c + "' is not recognized");
    }
    
    public void invalidNumber(String lexeme, int line, int column) {
        reportError(ErrorType.MALFORMED_NUMBER, line, column, lexeme, "Invalid number format");
    }
    
    public void tooManyDecimals(String lexeme, int line, int column) {
        reportError(ErrorType.TOO_MANY_DECIMALS, line, column, lexeme,
            "Float literal exceeds maximum of 6 decimal places");
    }
    
    public void invalidIdentifier(String lexeme, int line, int column) {
        String reason = "Identifier must start with uppercase letter (A-Z)";
        if (lexeme.length() > 31) {
            reason = "Identifier exceeds maximum length of 31 characters";
        }
        reportError(ErrorType.INVALID_IDENTIFIER, line, column, lexeme, reason);
    }
    
    public boolean hasErrors() { return !errors.isEmpty(); }
    public int getErrorCount() { return errors.size(); }
    public List<LexicalError> getErrors() { return new ArrayList<>(errors); }
    
    public void printSummary() {
        System.out.println("\n" + "=".repeat(60));
        System.out.println("ERROR SUMMARY");
        System.out.println("=".repeat(60));
        System.out.println("Total errors found: " + errors.size());
        System.out.println("-".repeat(60));
        for (int i = 0; i < errors.size(); i++) {
            System.out.printf("%3d. %s%n", i + 1, errors.get(i));
        }
        System.out.println("=".repeat(60));
    }
}
