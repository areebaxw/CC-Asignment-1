import java.io.*;
import java.nio.file.*;
import java.util.*;

/**
 * ScannerComparison - Runs both ManualScanner and Yylex on all test files
 * and writes a side-by-side comparison to ComparisonOutput.txt.
 * 
 * THIS FILE IS IN .gitignore - NOT part of the submission.
 */
public class ScannerComparison {

    private static PrintStream out;

    public static void main(String[] args) throws Exception {
        // Output file at project root
        String outputPath = "ComparisonOutput.txt";
        out = new PrintStream(new FileOutputStream(outputPath));

        String[] testFiles = {
            "tests/test1.pooh",
            "tests/test2.pooh",
            "tests/test3.pooh",
            "tests/test4.pooh",
            "tests/test5.pooh"
        };

        println("╔══════════════════════════════════════════════════════════════════════════════╗");
        println("║           SCANNER COMPARISON: ManualScanner vs Yylex (JFlex-style)          ║");
        println("╚══════════════════════════════════════════════════════════════════════════════╝");
        println("");

        // Aggregate stats
        long totalManualTime = 0;
        long totalYylexTime = 0;
        int totalManualTokens = 0;
        int totalYylexTokens = 0;
        int totalMatches = 0;
        int totalMismatches = 0;
        int totalFiles = 0;

        for (String testFile : testFiles) {
            Path path = Paths.get(testFile);
            if (!Files.exists(path)) {
                println("WARNING: " + testFile + " not found, skipping.\n");
                continue;
            }
            totalFiles++;
            String source = new String(Files.readAllBytes(path));

            println("=" .repeat(80));
            println("TEST FILE: " + testFile);
            println("=" .repeat(80));

            // ─── Run ManualScanner ───────────────────────────────────────────────
            long manualStart = System.nanoTime();
            ManualScanner manual = new ManualScanner(source);
            List<Token> manualAllTokens = manual.scan();
            long manualEnd = System.nanoTime();
            long manualTime = manualEnd - manualStart;
            totalManualTime += manualTime;

            // Filter out comments and whitespace (same as ManualScanner.getFilteredTokens)
            List<Token> manualTokens = new ArrayList<>();
            for (Token t : manualAllTokens) {
                if (t.getType() != TokenType.SINGLE_LINE_COMMENT && t.getType() != TokenType.WHITESPACE) {
                    manualTokens.add(t);
                }
            }

            // ─── Run Yylex ───────────────────────────────────────────────────────
            long ylexStart = System.nanoTime();
            Yylex ylex = new Yylex(source);
            List<Token> ylexTokens = ylex.scanAll();
            long ylexEnd = System.nanoTime();
            long ylexTime = ylexEnd - ylexStart;
            totalYylexTime += ylexTime;

            totalManualTokens += manualTokens.size();
            totalYylexTokens += ylexTokens.size();

            // ─── Side-by-Side Token Output ───────────────────────────────────────
            println("");
            println("┌─────────────────────────────────────────────────────────────────────────────┐");
            println("│ SIDE-BY-SIDE TOKEN OUTPUT                                                   │");
            println("├──────┬──────────────────────────────────┬──────────────────────────────────┤");
            println(String.format("│ %-4s │ %-36s │ %-36s │", "#", "ManualScanner", "Yylex"));
            println("├──────┼──────────────────────────────────┼──────────────────────────────────┤");

            int maxLen = Math.max(manualTokens.size(), ylexTokens.size());
            int matchCount = 0;
            int mismatchCount = 0;
            List<String> differences = new ArrayList<>();

            for (int i = 0; i < maxLen; i++) {
                String mStr = (i < manualTokens.size()) ? formatToken(manualTokens.get(i)) : "(no token)";
                String yStr = (i < ylexTokens.size()) ? formatToken(ylexTokens.get(i)) : "(no token)";

                boolean match = mStr.equals(yStr);
                String marker = match ? " " : "*";
                if (match) matchCount++; else mismatchCount++;

                if (!match) {
                    String mFull = (i < manualTokens.size()) ? manualTokens.get(i).toString() : "(no token)";
                    String yFull = (i < ylexTokens.size()) ? ylexTokens.get(i).toString() : "(no token)";
                    differences.add(String.format("  Token #%d:\n    Manual: %s\n    Yylex:  %s", i + 1, mFull, yFull));
                }

                // Truncate for table display
                if (mStr.length() > 34) mStr = mStr.substring(0, 31) + "...";
                if (yStr.length() > 34) yStr = yStr.substring(0, 31) + "...";

                println(String.format("│%s%-4d │ %-34s │ %-34s │", marker, i + 1, mStr, yStr));
            }

            println("└──────┴──────────────────────────────────┴──────────────────────────────────┘");
            println("");

            totalMatches += matchCount;
            totalMismatches += mismatchCount;

            // ─── Differences Detail ──────────────────────────────────────────────
            println("DIFFERENCES:");
            println("-" .repeat(60));
            if (differences.isEmpty()) {
                println("  ✓ ALL TOKENS MATCH - No differences found.");
            } else {
                println("  ✗ " + differences.size() + " difference(s) found:");
                for (String d : differences) {
                    println(d);
                }
            }
            println("");

            // ─── Symbol Table Comparison ─────────────────────────────────────────
            println("SYMBOL TABLE COMPARISON:");
            println("-" .repeat(60));
            SymbolTable mST = manual.getSymbolTable();
            SymbolTable yST = ylex.getSymbolTable();
            println("  ManualScanner identifiers: " + mST.size());
            println("  Yylex identifiers:         " + yST.size());

            Set<String> allSymbols = new LinkedHashSet<>();
            for (SymbolTable.SymbolEntry e : mST.getAllSymbols()) allSymbols.add(e.getName());
            for (SymbolTable.SymbolEntry e : yST.getAllSymbols()) allSymbols.add(e.getName());

            boolean symbolMatch = true;
            for (String sym : allSymbols) {
                boolean inManual = mST.contains(sym);
                boolean inYlex = yST.contains(sym);
                if (inManual && inYlex) {
                    int mFreq = mST.getSymbol(sym).getFrequency();
                    int yFreq = yST.getSymbol(sym).getFrequency();
                    if (mFreq != yFreq) {
                        println("  DIFF: '" + sym + "' frequency: Manual=" + mFreq + " Yylex=" + yFreq);
                        symbolMatch = false;
                    }
                } else if (inManual && !inYlex) {
                    println("  DIFF: '" + sym + "' only in ManualScanner");
                    symbolMatch = false;
                } else {
                    println("  DIFF: '" + sym + "' only in Yylex");
                    symbolMatch = false;
                }
            }
            if (symbolMatch) println("  ✓ Symbol tables match.");
            println("");

            // ─── Error Comparison ────────────────────────────────────────────────
            println("ERROR COMPARISON:");
            println("-" .repeat(60));
            ErrorHandler mEH = manual.getErrorHandler();
            ErrorHandler yEH = ylex.getErrorHandler();
            println("  ManualScanner errors: " + mEH.getErrorCount());
            println("  Yylex errors:         " + yEH.getErrorCount());
            if (mEH.getErrorCount() > 0 || yEH.getErrorCount() > 0) {
                if (mEH.getErrorCount() > 0) {
                    println("  ManualScanner error details:");
                    for (ErrorHandler.LexicalError e : mEH.getErrors())
                        println("    " + e);
                }
                if (yEH.getErrorCount() > 0) {
                    println("  Yylex error details:");
                    for (ErrorHandler.LexicalError e : yEH.getErrors())
                        println("    " + e);
                }
            }
            println("");

            // ─── Performance for this file ───────────────────────────────────────
            println("PERFORMANCE:");
            println("-" .repeat(60));
            println(String.format("  ManualScanner: %,d ns (%.3f ms)", manualTime, manualTime / 1_000_000.0));
            println(String.format("  Yylex:         %,d ns (%.3f ms)", ylexTime, ylexTime / 1_000_000.0));
            String faster = manualTime < ylexTime ? "ManualScanner" : "Yylex";
            double ratio = manualTime < ylexTime
                    ? (double) ylexTime / manualTime
                    : (double) manualTime / ylexTime;
            println(String.format("  → %s was %.2fx faster for this file.", faster, ratio));
            println("");
            println("");
        }

        // ═══════════════════════════════════════════════════════════════════════
        // OVERALL SUMMARY
        // ═══════════════════════════════════════════════════════════════════════
        println("╔══════════════════════════════════════════════════════════════════════════════╗");
        println("║                           OVERALL SUMMARY                                   ║");
        println("╚══════════════════════════════════════════════════════════════════════════════╝");
        println("");
        println("Files tested:       " + totalFiles);
        println("Total token matches:     " + totalMatches);
        println("Total token mismatches:  " + totalMismatches);
        println(String.format("Match rate:              %.1f%%",
                (totalMatches + totalMismatches) > 0
                        ? 100.0 * totalMatches / (totalMatches + totalMismatches)
                        : 0));
        println("");
        println("ManualScanner total tokens: " + totalManualTokens);
        println("Yylex total tokens:         " + totalYylexTokens);
        println("");
        println("OVERALL PERFORMANCE:");
        println(String.format("  ManualScanner total time: %,d ns (%.3f ms)", totalManualTime, totalManualTime / 1_000_000.0));
        println(String.format("  Yylex total time:         %,d ns (%.3f ms)", totalYylexTime, totalYylexTime / 1_000_000.0));
        String overallFaster = totalManualTime < totalYylexTime ? "ManualScanner" : "Yylex";
        double overallRatio = totalManualTime < totalYylexTime
                ? (double) totalYylexTime / totalManualTime
                : (double) totalManualTime / totalYylexTime;
        println(String.format("  → %s was %.2fx faster overall.", overallFaster, overallRatio));
        println("");

        out.close();
        System.out.println("Comparison complete! Output written to: " + outputPath);
    }

    /** Short format for table display: TYPE "lexeme" */
    private static String formatToken(Token t) {
        String lex = t.getLexeme();
        if (lex.length() > 15) lex = lex.substring(0, 12) + "...";
        return t.getType() + " \"" + lex + "\"";
    }

    private static void println(String s) {
        out.println(s);
    }
}
