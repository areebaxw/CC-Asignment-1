# JSON to XML Translator

A compiler-style translator that reads valid JSON input, parses it using Flex and Bison, constructs an Abstract Syntax Tree (AST), and converts the input into well-formed XML.

## Features Supported

- **Objects**: JSON objects with key-value pairs
- **Arrays**: JSON arrays with any value type
- **Strings**: String literals with escape sequences (\", \\, \n, \t)
- **Numbers**: Integer and floating-point numbers
- **Booleans**: true and false keywords
- **Null**: null keyword
- **Nested Structures**: Objects inside objects, arrays inside objects, objects inside arrays

## Conversion Rules

1. **Root Wrapper**: Output is wrapped in `<root>...</root>`
2. **JSON Object**: Becomes an XML element with children from key-value pairs
3. **Keys to XML Tags**: Each JSON key becomes the XML tag name
4. **Scalar Values**: Strings, numbers, and booleans become text content inside tags
5. **Null**: Becomes an empty XML element (e.g., `<editedAt/>`)
6. **Arrays**: Become a sequence of `<item>` elements
7. **Nested Structures**: Preserve nesting in XML

## AST Structure

The AST uses the following node types defined in `AST.h`:

- **StringNode**: Stores string values
- **NumberNode**: Stores numeric values as strings
- **BoolNode**: Stores boolean values
- **NullNode**: Represents null values
- **ArrayNode**: Contains a vector of child nodes (elements)
- **ObjectNode**: Contains a map of string keys to ASTNode values

All nodes inherit from the base `ASTNode` class which provides:
- `print(int indent)`: Debug printing of the AST
- `toXML(ostream& out, string tagName, int indent)`: XML generation

## Building the Project

### Prerequisites

- GCC/G++ compiler with C++11 support
- Flex (lexical analyzer generator)
- Bison (parser generator)
- Make utility

### Linux/Ubuntu/Debian

```bash
sudo apt update
sudo apt install -y build-essential flex bison
```

### Fedora/RHEL

```bash
sudo dnf install -y flex bison gcc-c++ make
```



### Windows

Use WSL2, MSYS2, Cygwin, or WinFlexBison as described in the installation guide.

### Build Command

```bash
make
```

This will:
1. Run Bison to generate `parser.tab.c` and `parser.tab.h`
2. Run Flex to generate `lex.yy.c`
3. Compile all source files with g++
4. Link to create the `json2xml` executable

### Clean Build

```bash
make clean
```

## Running the Program

### Basic Usage

```bash
./json2xml < input.json
```

### Example

```bash
./json2xml < test1_simple_object.json
```

### Test Files

Five test files are provided:

1. **test1_simple_object.json**: Simple object with scalar values
2. **test2_array_scalars.json**: Object with array of strings
3. **test3_nested_object.json**: Nested objects
4. **test4_null.json**: Null value handling
5. **test5_complex.json**: Complex nested structure with arrays of objects

Each test file has a corresponding `testN_expected.xml` file showing the expected output.

### Testing

```bash
# Test each file
./json2xml < test1_simple_object.json
./json2xml < test2_array_scalars.json
./json2xml < test3_nested_object.json
./json2xml < test4_null.json
./json2xml < test5_complex.json

# Compare with expected output
./json2xml < test1_simple_object.json | diff - test1_expected.xml
```

## Error Handling

The program reports clear error messages for:

- **Lexical errors**: Invalid characters, invalid escape sequences, unterminated strings
- **Syntax errors**: Malformed JSON, unexpected tokens

Example error messages:
```
Error: unexpected character '@'
Error: invalid escape sequence \x
Error: syntax error
```

The program stops at the first error and exits with a non-zero status.

## Assumptions and Limitations

1. **Valid XML tag names**: Test cases use simple keys that are valid XML tag names. No validation or transformation of keys is performed.
2. **Escape sequences**: Only supports \", \\, \n, and \t. Unicode escapes (\uXXXX) are not implemented (bonus feature).
3. **Number format**: Supports integers, floats, and scientific notation. No validation of numeric ranges.
4. **Memory management**: AST nodes are dynamically allocated and freed after XML generation.
5. **Whitespace**: JSON whitespace is ignored. XML output uses basic indentation (2 spaces per level).
6. **Error recovery**: No error recovery - the program stops at the first error.

## File Structure

```
.
├── AST.h                 # AST node class definitions
├── AST.cpp               # AST implementation and XML generation
├── scanner.l             # Flex lexer specification
├── parser.y              # Bison parser specification
├── Makefile              # Build configuration
├── README.md             # This file
├── test1_simple_object.json
├── test1_expected.xml
├── test2_array_scalars.json
├── test2_expected.xml
├── test3_nested_object.json
├── test3_expected.xml
├── test4_null.json
├── test4_expected.xml
├── test5_complex.json
└── test5_expected.xml
```

## Implementation Details

### Lexer (scanner.l)

- Recognizes JSON punctuation: `{ } [ ] : ,`
- Matches string literals with escape sequence handling
- Matches number literals (including scientific notation)
- Recognizes keywords: true, false, null
- Skips whitespace
- Reports lexical errors with clear messages

### Parser (parser.y)

- Implements JSON grammar: value, object, array, member, elements
- Constructs AST nodes during parsing using semantic actions
- Uses union type for different node types
- Reports syntax errors through yyerror

### XML Generation

- Escapes special XML characters: &, <, >, ", '
- Uses `<item>` tag for array elements
- Uses self-closing tags for null values
- Preserves structure through recursive traversal

## Bonus Features (Not Implemented)

The following bonus features from the assignment are not implemented but could be added:

- Pretty-printed XML with customizable indentation
- AST printing for debugging
- Column-based error detail in error messages
- Unicode escape support (\u1234)
- Scientific notation support (already partially supported in lexer)

## Author

CS-4031 Compiler Construction Assignment
FAST School of Computing
