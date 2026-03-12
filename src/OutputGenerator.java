import java.io.*;
import java.nio.file.*;
import java.util.*;

/**
 * Generates 10 output files: 5 from ManualScanner, 5 from Yylex.
 * All output files go in tests/ folder.
 * THIS FILE IS IN .gitignore.
 */
public class OutputGenerator {

    public static void main(String[] args) throws Exception {
        String[] testFiles = {
            "tests/test1.pooh",
            "tests/test2.pooh",
            "tests/test3.pooh",
            "tests/test4.pooh",
            "tests/test5.pooh"
        };

        for (int i = 0; i < testFiles.length; i++) {
            int num = i + 1;
            String source = new String(Files.readAllBytes(Paths.get(testFiles[i])));

            // ManualScanner output
            writeManualOutput(source, "tests/manual_output_test" + num + ".txt", testFiles[i]);

            // Yylex output
            writeYylexOutput(source, "tests/yylex_output_test" + num + ".txt", testFiles[i]);

            System.out.println("Done: test" + num + ".pooh");
        }
        System.out.println("All 10 output files generated.");
    }

    private static void writeManualOutput(String source, String outputPath, String testFile) throws Exception {
        PrintStream ps = new PrintStream(new FileOutputStream(outputPath));

        ps.println("============================================================");
        ps.println("MANUAL SCANNER OUTPUT");
        ps.println("Test File: " + testFile);
        ps.println("============================================================");
        ps.println();

        ManualScanner scanner = new ManualScanner(source);
        List<Token> allTokens = scanner.scan();

        // Filtered tokens (no comments, no whitespace)
        List<Token> filtered = scanner.getFilteredTokens();

        ps.println("TOKEN LIST (Comments Removed)");
        ps.println("------------------------------------------------------------");
        for (Token t : filtered) {
            ps.println(t);
        }
        ps.println("------------------------------------------------------------");
        ps.println();

        // Statistics
        ps.println("SCANNER STATISTICS");
        ps.println("------------------------------------------------------------");
        ps.println("Total tokens (including comments): " + allTokens.size());
        ps.println("Total tokens (excluding comments): " + filtered.size());
        ps.println("Lines processed: " + scanner.getTotalLines());
        ps.println("Comments removed: " + scanner.getCommentsRemoved());
        ps.println("Errors found: " + scanner.getErrorHandler().getErrorCount());
        ps.println();
        ps.println("Token counts by type:");

        Map<TokenType, Integer> counts = scanner.getTokenCounts();
        for (TokenType type : TokenType.values()) {
            int count = counts.get(type);
            if (count > 0) {
                ps.println("  " + padRight(type.toString(), 25) + ": " + count);
            }
        }
        ps.println("------------------------------------------------------------");
        ps.println();

        // Symbol Table
        ps.println("SYMBOL TABLE");
        ps.println("------------------------------------------------------------");
        ps.println(padRight("Identifier", 20) + padRight("Type", 15)
                + padRight("First Line", 12) + padRight("First Col", 10) + "Frequency");
        ps.println("------------------------------------------------------------");
        SymbolTable st = scanner.getSymbolTable();
        if (st.size() == 0) {
            ps.println("  (No identifiers found)");
        } else {
            for (SymbolTable.SymbolEntry e : st.getAllSymbols()) {
                ps.println(padRight(e.getName(), 20)
                        + padRight(e.getType() != null ? e.getType() : "unknown", 15)
                        + padRight("" + e.getFirstOccurrenceLine(), 12)
                        + padRight("" + e.getFirstOccurrenceColumn(), 10)
                        + e.getFrequency());
            }
        }
        ps.println("Total unique identifiers: " + st.size());
        ps.println("------------------------------------------------------------");
        ps.println();

        // Errors
        ErrorHandler eh = scanner.getErrorHandler();
        if (eh.hasErrors()) {
            ps.println("ERROR SUMMARY");
            ps.println("------------------------------------------------------------");
            ps.println("Total errors found: " + eh.getErrorCount());
            List<ErrorHandler.LexicalError> errors = eh.getErrors();
            for (int i = 0; i < errors.size(); i++) {
                ps.println("  " + (i + 1) + ". " + errors.get(i));
            }
            ps.println("------------------------------------------------------------");
        } else {
            ps.println("No lexical errors found.");
        }

        ps.close();
    }

    private static void writeYylexOutput(String source, String outputPath, String testFile) throws Exception {
        PrintStream ps = new PrintStream(new FileOutputStream(outputPath));

        ps.println("============================================================");
        ps.println("YYLEX (JFLEX) SCANNER OUTPUT");
        ps.println("Test File: " + testFile);
        ps.println("============================================================");
        ps.println();

        Yylex scanner = new Yylex(source);
        List<Token> allTokens = scanner.scanAll();

        // Filter out EOF for display, keep it at end
        List<Token> display = new ArrayList<>();
        for (Token t : allTokens) {
            display.add(t);
        }

        ps.println("TOKEN LIST (Comments Removed)");
        ps.println("------------------------------------------------------------");
        for (Token t : display) {
            ps.println(t);
        }
        ps.println("------------------------------------------------------------");
        ps.println();

        // Statistics
        ps.println("SCANNER STATISTICS");
        ps.println("------------------------------------------------------------");
        ps.println("Total tokens: " + allTokens.size());
        ps.println("Comments removed: " + scanner.getCommentCount());
        ps.println("Errors found: " + scanner.getErrorHandler().getErrorCount());
        ps.println("------------------------------------------------------------");
        ps.println();

        // Symbol Table
        ps.println("SYMBOL TABLE");
        ps.println("------------------------------------------------------------");
        ps.println(padRight("Identifier", 20) + padRight("Type", 15)
                + padRight("First Line", 12) + padRight("First Col", 10) + "Frequency");
        ps.println("------------------------------------------------------------");
        SymbolTable st = scanner.getSymbolTable();
        if (st.size() == 0) {
            ps.println("  (No identifiers found)");
        } else {
            for (SymbolTable.SymbolEntry e : st.getAllSymbols()) {
                ps.println(padRight(e.getName(), 20)
                        + padRight(e.getType() != null ? e.getType() : "unknown", 15)
                        + padRight("" + e.getFirstOccurrenceLine(), 12)
                        + padRight("" + e.getFirstOccurrenceColumn(), 10)
                        + e.getFrequency());
            }
        }
        ps.println("Total unique identifiers: " + st.size());
        ps.println("------------------------------------------------------------");
        ps.println();

        // Errors
        ErrorHandler eh = scanner.getErrorHandler();
        if (eh.hasErrors()) {
            ps.println("ERROR SUMMARY");
            ps.println("------------------------------------------------------------");
            ps.println("Total errors found: " + eh.getErrorCount());
            List<ErrorHandler.LexicalError> errors = eh.getErrors();
            for (int i = 0; i < errors.size(); i++) {
                ps.println("  " + (i + 1) + ". " + errors.get(i));
            }
            ps.println("------------------------------------------------------------");
        } else {
            ps.println("No lexical errors found.");
        }

        ps.close();
    }

    private static String padRight(String s, int width) {
        if (s.length() >= width) return s;
        StringBuilder sb = new StringBuilder(s);
        while (sb.length() < width) sb.append(' ');
        return sb.toString();
    }
}
