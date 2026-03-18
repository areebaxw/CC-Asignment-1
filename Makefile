CXX      = g++
CXXFLAGS = -std=c++14 -Wall -Wextra -g
SRCDIR   = src
OUTDIR   = output

SOURCES  = $(SRCDIR)/main.cpp \
           $(SRCDIR)/grammar.cpp \
           $(SRCDIR)/first_follow.cpp \
           $(SRCDIR)/parser.cpp \
           $(SRCDIR)/tree.cpp \
           $(SRCDIR)/error_handler.cpp

TARGET   = parser

.PHONY: all clean run1 run2 run3 run4

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

# Run with Grammar 1 (simple) + valid inputs
run1: $(TARGET)
	./$(TARGET) input/grammar1.txt input/input_edge_cases.txt

# Run with Grammar 2 (expressions) + valid inputs
run2: $(TARGET)
	./$(TARGET) input/grammar2.txt input/input_valid.txt

# Run with Grammar 2 + error inputs
run3: $(TARGET)
	./$(TARGET) input/grammar2.txt input/input_errors.txt

# Run with Grammar 3 (if-then-else)
run4: $(TARGET)
	./$(TARGET) input/grammar3.txt input/input_valid.txt

clean:
	rm -f $(TARGET)
