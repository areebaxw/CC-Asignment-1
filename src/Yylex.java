import java.io.*;
import java.util.*;

/**
 * Yylex - JFlex-style Scanner (Manual Implementation - Simplified)
 * CS4031 - Compiler Construction Assignment 01
 * 
 * Supports only: Int lit, Float lit, Identifier, Single-line comment, 
 * Boolean, Punctuation, Whitespace
 */
public class Yylex {
    
    private Reader reader;
    private char[] buffer;
    private int bufferPos;
    private int bufferEnd;
    private static final int BUFFER_SIZE = 16384;
    
    private int yyline;
    private int yycolumn;
    private StringBuilder yytext;
    private int tokenStartLine;
    private int tokenStartColumn;
    
    private SymbolTable symbolTable;
    private ErrorHandler errorHandler;
    private int commentCount;
    private boolean eof;
    
    private static final Set<String> BOOLEAN_LITERALS = new HashSet<>(Arrays.asList("true", "false"));
    
    public Yylex(Reader reader) {
        this.reader = reader;
        this.buffer = new char[BUFFER_SIZE];
        this.bufferPos = 0;
        this.bufferEnd = 0;
        this.yyline = 0;
        this.yycolumn = 0;
        this.yytext = new StringBuilder();
        this.symbolTable = new SymbolTable();
        this.errorHandler = new ErrorHandler();
        this.commentCount = 0;
        this.eof = false;
    }
    
    public Yylex(InputStream in) { this(new InputStreamReader(in)); }
    public Yylex(String source) { this(new StringReader(source)); }
    
    public SymbolTable getSymbolTable() { return symbolTable; }
    public ErrorHandler getErrorHandler() { return errorHandler; }
    public int getCommentCount() { return commentCount; }
    
    private int read() throws IOException {
        if (bufferPos >= bufferEnd) {
            bufferEnd = reader.read(buffer, 0, BUFFER_SIZE);
            bufferPos = 0;
            if (bufferEnd <= 0) { eof = true; return -1; }
        }
        char c = buffer[bufferPos++];
        if (c == '\n') { yyline++; yycolumn = 0; } else { yycolumn++; }
        return c;
    }
    
    private int peek() throws IOException {
        if (bufferPos >= bufferEnd) {
            bufferEnd = reader.read(buffer, 0, BUFFER_SIZE);
            bufferPos = 0;
            if (bufferEnd <= 0) return -1;
        }
        return buffer[bufferPos];
    }
    
    private Token token(TokenType type) {
        return new Token(type, yytext.toString(), tokenStartLine + 1, tokenStartColumn + 1);
    }
    
    private boolean isDigit(int c) { return c >= '0' && c <= '9'; }
    private boolean isUpperCase(int c) { return c >= 'A' && c <= 'Z'; }
    private boolean isLowerCase(int c) { return c >= 'a' && c <= 'z'; }
    
    public Token yylex() throws IOException {
        while (!eof) {
            yytext.setLength(0);
            tokenStartLine = yyline;
            tokenStartColumn = yycolumn;
            
            int c = read();
            if (c == -1) return token(TokenType.EOF);
            
            // Skip whitespace
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n') continue;
            
            yytext.append((char) c);
            
            // Single-line comment: ##[^\n]*
            if (c == '#') {
                if (peek() == '#') {
                    yytext.append((char) read());
                    while ((c = read()) != -1 && c != '\n') yytext.append((char) c);
                    commentCount++;
                    continue;
                } else {
                    errorHandler.invalidCharacter((char) c, tokenStartLine + 1, tokenStartColumn + 1);
                    return token(TokenType.ERROR);
                }
            }
            
            // Number (Integer or Float)
            if (isDigit(c)) {
                while (peek() != -1 && isDigit(peek())) yytext.append((char) read());
                
                if (peek() == '.') {
                    yytext.append((char) read());
                    int decimalDigits = 0;
                    while (peek() != -1 && isDigit(peek()) && decimalDigits < 6) {
                        yytext.append((char) read());
                        decimalDigits++;
                    }
                    if (isDigit(peek())) {
                        while (isDigit(peek())) yytext.append((char) read());
                        errorHandler.tooManyDecimals(yytext.toString(), tokenStartLine + 1, tokenStartColumn + 1);
                        return token(TokenType.ERROR);
                    }
                    
                    int p = peek();
                    if (p == 'e' || p == 'E') {
                        yytext.append((char) read());
                        p = peek();
                        if (p == '+' || p == '-') yytext.append((char) read());
                        if (!isDigit(peek())) {
                            errorHandler.invalidNumber(yytext.toString(), tokenStartLine + 1, tokenStartColumn + 1);
                            return token(TokenType.ERROR);
                        }
                        while (peek() != -1 && isDigit(peek())) yytext.append((char) read());
                    }
                    return token(TokenType.FLOAT_LIT);
                }
                
                int p = peek();
                if (p == 'e' || p == 'E') {
                    yytext.append((char) read());
                    p = peek();
                    if (p == '+' || p == '-') yytext.append((char) read());
                    if (!isDigit(peek())) {
                        errorHandler.invalidNumber(yytext.toString(), tokenStartLine + 1, tokenStartColumn + 1);
                        return token(TokenType.ERROR);
                    }
                    while (peek() != -1 && isDigit(peek())) yytext.append((char) read());
                    return token(TokenType.FLOAT_LIT);
                }
                
                return token(TokenType.INT_LIT);
            }
            
            // Identifier: [A-Z][a-z0-9_]{0,30}
            if (isUpperCase(c)) {
                while (peek() != -1 && (isLowerCase(peek()) || isDigit(peek()) || peek() == '_')) {
                    if (yytext.length() >= 31) {
                        while (isLowerCase(peek()) || isDigit(peek()) || peek() == '_') yytext.append((char) read());
                        errorHandler.invalidIdentifier(yytext.toString(), tokenStartLine + 1, tokenStartColumn + 1);
                        return token(TokenType.ERROR);
                    }
                    yytext.append((char) read());
                }
                symbolTable.addSymbol(yytext.toString(), null, tokenStartLine + 1, tokenStartColumn + 1);
                return token(TokenType.IDENTIFIER);
            }
            
            // Boolean or invalid identifier
            if (isLowerCase(c)) {
                while (peek() != -1 && isLowerCase(peek())) yytext.append((char) read());
                if (BOOLEAN_LITERALS.contains(yytext.toString())) {
                    return token(TokenType.BOOLEAN_LIT);
                } else {
                    errorHandler.invalidIdentifier(yytext.toString(), tokenStartLine + 1, tokenStartColumn + 1);
                    return token(TokenType.ERROR);
                }
            }
            
            // Punctuators
            switch (c) {
                case '(': return token(TokenType.LPAREN);
                case ')': return token(TokenType.RPAREN);
                case '{': return token(TokenType.LBRACE);
                case '}': return token(TokenType.RBRACE);
                case '[': return token(TokenType.LBRACKET);
                case ']': return token(TokenType.RBRACKET);
                case ',': return token(TokenType.COMMA);
                case ';': return token(TokenType.SEMICOLON);
                case ':': return token(TokenType.COLON);
                default:
                    errorHandler.invalidCharacter((char) c, tokenStartLine + 1, tokenStartColumn + 1);
                    return token(TokenType.ERROR);
            }
        }
        return new Token(TokenType.EOF, "", yyline + 1, yycolumn + 1);
    }
    
    public List<Token> scanAll() throws IOException {
        List<Token> tokens = new ArrayList<>();
        Token token;
        do {
            token = yylex();
            tokens.add(token);
        } while (token.getType() != TokenType.EOF);
        return tokens;
    }
    
    public static void main(String[] args) throws IOException {
        String testCode;
        if (args.length > 0) {
            testCode = new String(java.nio.file.Files.readAllBytes(java.nio.file.Paths.get(args[0])));
        } else {
            testCode = """
                ## Test code
                Count
                Variable_name
                42
                3.14
                true
                false
                ( ) { } [ ] , ; :
                """;
        }
        
        System.out.println("=== JFlex Scanner (Yylex) ===\n");
        System.out.println("Source:\n" + testCode);
        System.out.println("\nTokens:");
        System.out.println("-".repeat(60));
        
        Yylex scanner = new Yylex(testCode);
        List<Token> tokens = scanner.scanAll();
        for (Token token : tokens) System.out.println(token);
        
        System.out.println("\nComments removed: " + scanner.getCommentCount());
        scanner.getSymbolTable().printTable();
        if (scanner.getErrorHandler().hasErrors()) scanner.getErrorHandler().printSummary();
    }
}
