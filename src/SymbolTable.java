import java.util.*;

/**
 * SymbolTable class for storing identifier information
 * CS4031 - Compiler Construction Assignment 01
 * 
 * Stores: identifier name, type, first occurrence, frequency
 */
public class SymbolTable {
    
    public static class SymbolEntry {
        private String name;
        private String type;
        private int firstOccurrenceLine;
        private int firstOccurrenceColumn;
        private int frequency;
        
        public SymbolEntry(String name, String type, int line, int column) {
            this.name = name;
            this.type = type;
            this.firstOccurrenceLine = line;
            this.firstOccurrenceColumn = column;
            this.frequency = 1;
        }
        
        public String getName() { return name; }
        public String getType() { return type; }
        public int getFirstOccurrenceLine() { return firstOccurrenceLine; }
        public int getFirstOccurrenceColumn() { return firstOccurrenceColumn; }
        public int getFrequency() { return frequency; }
        
        public void setType(String type) { this.type = type; }
        public void incrementFrequency() { this.frequency++; }
        
        @Override
        public String toString() {
            return String.format("%-20s %-15s Line: %-5d Col: %-5d Frequency: %d",
                    name, type != null ? type : "unknown", firstOccurrenceLine, 
                    firstOccurrenceColumn, frequency);
        }
    }
    
    private Map<String, SymbolEntry> symbols;
    
    public SymbolTable() {
        symbols = new LinkedHashMap<>();
    }
    
    public void addSymbol(String name, String type, int line, int column) {
        if (symbols.containsKey(name)) {
            symbols.get(name).incrementFrequency();
        } else {
            symbols.put(name, new SymbolEntry(name, type, line, column));
        }
    }
    
    public boolean contains(String name) { return symbols.containsKey(name); }
    public SymbolEntry getSymbol(String name) { return symbols.get(name); }
    public Collection<SymbolEntry> getAllSymbols() { return symbols.values(); }
    public int size() { return symbols.size(); }
    
    public void printTable() {
        System.out.println("\n" + "=".repeat(80));
        System.out.println("SYMBOL TABLE");
        System.out.println("=".repeat(80));
        System.out.printf("%-20s %-15s %-12s %-10s %s%n", 
                "Identifier", "Type", "First Line", "First Col", "Frequency");
        System.out.println("-".repeat(80));
        
        if (symbols.isEmpty()) {
            System.out.println("  (No identifiers found)");
        } else {
            for (SymbolEntry entry : symbols.values()) {
                System.out.println(entry);
            }
        }
        System.out.println("-".repeat(80));
        System.out.println("Total unique identifiers: " + symbols.size());
        System.out.println("=".repeat(80));
    }
}
