import java.io.*;
import java.util.*;

/**
 * ManualScanner - A DFA-based lexical analyzer (Simplified)
 * CS4031 - Compiler Construction Assignment 01
 * 
 * Supports only:
 * - Identifiers: [A-Z][a-z0-9_]{0,30}
 * - Integer Literals: [0-9]+
 * - Float Literals: [0-9]+\.[0-9]{1,6}([eE][+-]?[0-9]+)?
 * - Boolean Literals: true|false
 * - Single-line Comments: ##[^\n]*
 * - Punctuators: ( ) { } [ ] , ; :
 * - Whitespace: [ \t\r\n]+
 */
public class ManualScanner {
    
    private String source;
    private int position;
    private int line;
    private int column;
    private int startLine;
    private int startColumn;
    
    private List<Token> tokens;
    private SymbolTable symbolTable;
    private ErrorHandler errorHandler;
    
    private Map<TokenType, Integer> tokenCounts;
    private int totalLines;
    private int commentsRemoved;
    
    // Boolean literals
    private static final Set<String> BOOLEAN_LITERALS = new HashSet<>(Arrays.asList("true", "false"));
    
    public ManualScanner(String source) {
        this.source = source;
        this.position = 0;
        this.line = 1;
        this.column = 1;
        this.tokens = new ArrayList<>();
        this.symbolTable = new SymbolTable();
        this.errorHandler = new ErrorHandler();
        this.tokenCounts = new HashMap<>();
        this.totalLines = 1;
        this.commentsRemoved = 0;
        
        for (TokenType type : TokenType.values()) {
            tokenCounts.put(type, 0);
        }
    }
    
    public List<Token> scan() {
        while (!isAtEnd()) {
            startLine = line;
            startColumn = column;
            scanToken();
        }
        addToken(TokenType.EOF, "");
        return tokens;
    }
    
    private void scanToken() {
        char c = advance();
        
        switch (c) {
            // Punctuators
            case '(': addToken(TokenType.LPAREN, "("); break;
            case ')': addToken(TokenType.RPAREN, ")"); break;
            case '{': addToken(TokenType.LBRACE, "{"); break;
            case '}': addToken(TokenType.RBRACE, "}"); break;
            case '[': addToken(TokenType.LBRACKET, "["); break;
            case ']': addToken(TokenType.RBRACKET, "]"); break;
            case ',': addToken(TokenType.COMMA, ","); break;
            case ';': addToken(TokenType.SEMICOLON, ";"); break;
            case ':': addToken(TokenType.COLON, ":"); break;
            
            // Single-line comment: ##[^\n]*
            case '#':
                if (match('#')) {
                    scanSingleLineComment();
                } else {
                    errorHandler.invalidCharacter(c, startLine, startColumn);
                    addToken(TokenType.ERROR, "#");
                }
                break;
            
            // Whitespace
            case ' ':
            case '\t':
            case '\r':
                break;
            case '\n':
                line++;
                totalLines++;
                column = 1;
                break;
            
            default:
                if (isDigit(c)) {
                    position--; column--;
                    scanNumber();
                } else if (isUpperCase(c)) {
                    position--; column--;
                    scanIdentifier();
                } else if (isLowerCase(c)) {
                    position--; column--;
                    scanBooleanOrError();
                } else {
                    errorHandler.invalidCharacter(c, startLine, startColumn);
                    addToken(TokenType.ERROR, String.valueOf(c));
                }
                break;
        }
    }
    
    private enum NumberState { START, INTEGER, DOT, DECIMAL, EXP_START, EXP_SIGN, EXPONENT }
    
    private void scanNumber() {
        startLine = line;
        startColumn = column;
        
        StringBuilder lexeme = new StringBuilder();
        NumberState state = NumberState.START;
        boolean isFloat = false;
        int decimalDigits = 0;
        
        while (!isAtEnd()) {
            char c = peek();
            
            switch (state) {
                case START:
                    if (isDigit(c)) { lexeme.append(advance()); state = NumberState.INTEGER; }
                    else return;
                    break;
                case INTEGER:
                    if (isDigit(c)) { lexeme.append(advance()); }
                    else if (c == '.') {
                        if (position + 1 < source.length() && isDigit(source.charAt(position + 1))) {
                            lexeme.append(advance()); state = NumberState.DOT; isFloat = true;
                        } else {
                            addToken(TokenType.INT_LIT, lexeme.toString()); return;
                        }
                    } else if (c == 'e' || c == 'E') {
                        lexeme.append(advance()); state = NumberState.EXP_START; isFloat = true;
                    } else {
                        addToken(TokenType.INT_LIT, lexeme.toString()); return;
                    }
                    break;
                case DOT:
                    if (isDigit(c)) { lexeme.append(advance()); decimalDigits++; state = NumberState.DECIMAL; }
                    else { errorHandler.invalidNumber(lexeme.toString(), startLine, startColumn); addToken(TokenType.ERROR, lexeme.toString()); return; }
                    break;
                case DECIMAL:
                    if (isDigit(c)) {
                        if (decimalDigits < 6) { lexeme.append(advance()); decimalDigits++; }
                        else {
                            while (!isAtEnd() && isDigit(peek())) lexeme.append(advance());
                            errorHandler.tooManyDecimals(lexeme.toString(), startLine, startColumn);
                            addToken(TokenType.ERROR, lexeme.toString()); return;
                        }
                    } else if (c == 'e' || c == 'E') {
                        lexeme.append(advance()); state = NumberState.EXP_START;
                    } else {
                        addToken(TokenType.FLOAT_LIT, lexeme.toString()); return;
                    }
                    break;
                case EXP_START:
                    if (isDigit(c)) { lexeme.append(advance()); state = NumberState.EXPONENT; }
                    else if (c == '+' || c == '-') { lexeme.append(advance()); state = NumberState.EXP_SIGN; }
                    else { errorHandler.invalidNumber(lexeme.toString(), startLine, startColumn); addToken(TokenType.ERROR, lexeme.toString()); return; }
                    break;
                case EXP_SIGN:
                    if (isDigit(c)) { lexeme.append(advance()); state = NumberState.EXPONENT; }
                    else { errorHandler.invalidNumber(lexeme.toString(), startLine, startColumn); addToken(TokenType.ERROR, lexeme.toString()); return; }
                    break;
                case EXPONENT:
                    if (isDigit(c)) { lexeme.append(advance()); }
                    else { addToken(TokenType.FLOAT_LIT, lexeme.toString()); return; }
                    break;
            }
        }
        
        if (state == NumberState.INTEGER) addToken(TokenType.INT_LIT, lexeme.toString());
        else if (state == NumberState.DECIMAL || state == NumberState.EXPONENT) addToken(TokenType.FLOAT_LIT, lexeme.toString());
        else { errorHandler.invalidNumber(lexeme.toString(), startLine, startColumn); addToken(TokenType.ERROR, lexeme.toString()); }
    }
    
    private void scanIdentifier() {
        startLine = line;
        startColumn = column;
        StringBuilder lexeme = new StringBuilder();
        
        char first = advance();
        lexeme.append(first);
        
        while (!isAtEnd() && lexeme.length() < 31) {
            char c = peek();
            if (isLowerCase(c) || isDigit(c) || c == '_') {
                lexeme.append(advance());
            } else break;
        }
        
        String text = lexeme.toString();
        
        if (!isAtEnd() && (isLowerCase(peek()) || isDigit(peek()) || peek() == '_')) {
            while (!isAtEnd() && (isLowerCase(peek()) || isDigit(peek()) || peek() == '_')) {
                lexeme.append(advance());
            }
            errorHandler.invalidIdentifier(lexeme.toString(), startLine, startColumn);
            addToken(TokenType.ERROR, lexeme.toString());
            return;
        }
        
        symbolTable.addSymbol(text, null, startLine, startColumn);
        addToken(TokenType.IDENTIFIER, text);
    }
    
    private void scanBooleanOrError() {
        startLine = line;
        startColumn = column;
        StringBuilder lexeme = new StringBuilder();
        
        while (!isAtEnd() && isLowerCase(peek())) {
            lexeme.append(advance());
        }
        
        String text = lexeme.toString();
        
        if (BOOLEAN_LITERALS.contains(text)) {
            addToken(TokenType.BOOLEAN_LIT, text);
        } else {
            errorHandler.invalidIdentifier(text, startLine, startColumn);
            addToken(TokenType.ERROR, text);
        }
    }
    
    private void scanSingleLineComment() {
        StringBuilder lexeme = new StringBuilder("##");
        while (!isAtEnd() && peek() != '\n') {
            lexeme.append(advance());
        }
        commentsRemoved++;
        addToken(TokenType.SINGLE_LINE_COMMENT, lexeme.toString());
    }
    
    private boolean isAtEnd() { return position >= source.length(); }
    private char advance() { column++; return source.charAt(position++); }
    private char peek() { return isAtEnd() ? '\0' : source.charAt(position); }
    private boolean match(char expected) {
        if (isAtEnd() || source.charAt(position) != expected) return false;
        position++; column++; return true;
    }
    private boolean isDigit(char c) { return c >= '0' && c <= '9'; }
    private boolean isUpperCase(char c) { return c >= 'A' && c <= 'Z'; }
    private boolean isLowerCase(char c) { return c >= 'a' && c <= 'z'; }
    
    private void addToken(TokenType type, String lexeme) {
        tokens.add(new Token(type, lexeme, startLine, startColumn));
        tokenCounts.put(type, tokenCounts.get(type) + 1);
    }
    
    public SymbolTable getSymbolTable() { return symbolTable; }
    public ErrorHandler getErrorHandler() { return errorHandler; }
    public Map<TokenType, Integer> getTokenCounts() { return tokenCounts; }
    public int getTotalLines() { return totalLines; }
    public int getCommentsRemoved() { return commentsRemoved; }
    
    public List<Token> getFilteredTokens() {
        List<Token> filtered = new ArrayList<>();
        for (Token token : tokens) {
            if (token.getType() != TokenType.SINGLE_LINE_COMMENT && 
                token.getType() != TokenType.WHITESPACE) {
                filtered.add(token);
            }
        }
        return filtered;
    }
    
    public void printStatistics() {
        System.out.println("\n" + "=".repeat(60));
        System.out.println("SCANNER STATISTICS");
        System.out.println("=".repeat(60));
        System.out.println("Total tokens (including comments): " + tokens.size());
        System.out.println("Total tokens (excluding comments): " + getFilteredTokens().size());
        System.out.println("Lines processed: " + totalLines);
        System.out.println("Comments removed: " + commentsRemoved);
        System.out.println("Errors found: " + errorHandler.getErrorCount());
        System.out.println("\nToken counts by type:");
        System.out.println("-".repeat(40));
        for (TokenType type : TokenType.values()) {
            int count = tokenCounts.get(type);
            if (count > 0) System.out.printf("  %-25s: %d%n", type, count);
        }
        System.out.println("=".repeat(60));
    }
    
    public void printTokens() {
        System.out.println("\n" + "=".repeat(60));
        System.out.println("TOKEN LIST (Pre-processed - Comments Removed)");
        System.out.println("=".repeat(60));
        for (Token token : getFilteredTokens()) {
            System.out.println(token);
        }
        System.out.println("=".repeat(60));
    }
    
    public static void main(String[] args) {
        String testCode;
        if (args.length > 0) {
            try {
                testCode = new String(java.nio.file.Files.readAllBytes(java.nio.file.Paths.get(args[0])));
            } catch (IOException e) {
                System.err.println("Error reading file: " + e.getMessage()); return;
            }
        } else {
            testCode = """
                ## Test program
                Count
                Variable_name
                X
                Total_sum_2024
                
                42
                100
                0
                
                3.14
                2.5e10
                0.123456
                
                true
                false
                
                ( ) { } [ ] , ; :
                
                ## End of test
                """;
        }
        
        System.out.println("SOURCE CODE:");
        System.out.println("=".repeat(60));
        System.out.println(testCode);
        
        ManualScanner scanner = new ManualScanner(testCode);
        scanner.scan();
        scanner.printTokens();
        scanner.printStatistics();
        scanner.getSymbolTable().printTable();
        
        if (scanner.getErrorHandler().hasErrors()) {
            scanner.getErrorHandler().printSummary();
        } else {
            System.out.println("\nNo lexical errors found.");
        }
    }
}
