# CS4031 - Compiler Construction Assignment 01
## Lexical Analyzer (Scanner) - Simplified Implementation

### Team Members
- 23i-6002 - Areeba Waqar
- 23i-0537 - Mahad Malik

---

## Language Name: **Pooh**
## File Extension: **.pooh**

---

## Supported Token Types

This scanner recognizes the following token types ONLY:

| Token Type | Description | Regex Pattern |
|------------|-------------|---------------|
| **IDENTIFIER** | Variable names | `[A-Z][a-z0-9_]{0,30}` |
| **INT_LIT** | Integer literals | `[0-9]+` |
| **FLOAT_LIT** | Float literals | `[0-9]+\.[0-9]{1,6}([eE][+-]?[0-9]+)?` |
| **BOOLEAN_LIT** | Boolean values | `true\|false` |
| **SINGLE_LINE_COMMENT** | Comments | `##[^\n]*` |
| **PUNCTUATORS** | Delimiters | `( ) { } [ ] , ; :` |
| **WHITESPACE** | Spaces/tabs/newlines | `[ \t\r\n]+` |

---

## Identifier Rules

**Regex:** `[A-Z][a-z0-9_]{0,30}`

### Rules:
1. Must start with **uppercase letter** (A-Z)
2. Followed by lowercase letters (a-z), digits (0-9), or underscore (_)
3. Maximum **31 characters** total

### Valid Examples:
- `Count`
- `Variable_name`
- `X`
- `Total_sum_2024`

### Invalid Examples:
- `count` (starts with lowercase)
- `2Count` (starts with digit)
- `myVariable` (starts with lowercase)

---

## Literal Formats

### Integer Literals
**Regex:** `[0-9]+`
- Valid: `42`, `100`, `0`, `999999`

### Floating-Point Literals
**Regex:** `[0-9]+\.[0-9]{1,6}([eE][+-]?[0-9]+)?`
- Valid: `3.14`, `2.5`, `0.123456`, `1.5e10`, `2.0E-3`
- Invalid: `3.` (no decimals), `1.2345678` (>6 decimals)

### Boolean Literals
**Regex:** `(true|false)` (case-sensitive)
- Valid: `true`, `false`
- Invalid: `True`, `FALSE`

---

## Comment Syntax

### Single-Line Comments
**Regex:** `##[^\n]*`
```
## This is a single-line comment
```

---

## Punctuators

| Symbol | Token Type |
|--------|------------|
| `(` | LPAREN |
| `)` | RPAREN |
| `{` | LBRACE |
| `}` | RBRACE |
| `[` | LBRACKET |
| `]` | RBRACKET |
| `,` | COMMA |
| `;` | SEMICOLON |
| `:` | COLON |

---

## Sample Programs

### Sample 1: Basic Tokens
```
## Sample program with all token types
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
```

### Sample 2: Mixed Tokens
```
## Variable declarations
Counter
Max_value
Index

## Numbers
100
3.14159
2.5e-3

## Booleans
true
false

## Punctuation grouping
(Counter, Max_value)
{Index; true}
[3.14: false]
```

### Sample 3: Complex Identifiers
```
## Testing identifier patterns
A
Ab
Abc_def
Variable_name_123
Long_identifier_name
```

---

## Compilation and Execution

### Compile
```bash
cd src
javac *.java
```

### Run Manual Scanner
```bash
java ManualScanner              # Default test
java ManualScanner test.lang    # With file
```

### Run JFlex Scanner
```bash
java Yylex                      # Default test
java Yylex test.lang            # With file
```

### Generate JFlex Scanner
```bash
jflex Scanner.flex
javac *.java
```

---

## Token Output Format

```
<TOKEN_TYPE, "lexeme", Line: n, Col: m>
```

Example:
```
<IDENTIFIER, "Count", Line: 1, Col: 1>
<INT_LIT, "42", Line: 2, Col: 1>
<FLOAT_LIT, "3.14", Line: 3, Col: 1>
<BOOLEAN_LIT, "true", Line: 4, Col: 1>
```

---

## Error Handling

Detected errors:
- **Invalid characters**: `@`, `$`, `+`, `-`, `*`, etc.
- **Malformed numbers**: Multiple decimals, incomplete exponent
- **Invalid identifiers**: Starting with lowercase, too long

Error format:
```
[Error Type] Line: n, Col: m, Lexeme: "...", Reason: ...
```

---

## Project Structure

```
23i6002-23i0537-E/
├── src/
│   ├── ManualScanner.java
│   ├── Token.java
│   ├── TokenType.java
│   ├── SymbolTable.java
│   ├── ErrorHandler.java
│   ├── Scanner.flex
│   └── Yylex.java
├── docs/
│   ├── Automata_Design.pdf
│   └── Comparison.pdf
├── tests/
│   ├── test1.lang - test5.lang
│   └── TestResults.txt
├── LanguageGrammar.txt
└── README.md
```
