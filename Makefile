# Makefile for SLR(1) and LR(1) Parser
# For Windows with MinGW/g++

CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2
LDFLAGS = 

# Source and object files
SOURCES = src/Token.cpp src/Grammar.cpp src/Items.cpp src/Stack.cpp \
          src/ParsingTable.cpp src/SLRParser.cpp src/LR1Parser.cpp \
		  src/Tree.cpp src/GraphvizConverter.cpp src/main.cpp

OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = bin/parser.exe

# Default target
all: $(EXECUTABLE)

# Build executable
$(EXECUTABLE): $(OBJECTS)
	@echo Linking...
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo Build complete!

# Compile source files
%.o: %.cpp
	@echo Compiling $<...
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create bin directory if needed
bin:
	@if not exist bin mkdir bin

# Clean
clean:
	@echo Cleaning...
	@for %%f in ($(OBJECTS)) do @if exist %%f del %%f
	@if exist $(EXECUTABLE) del $(EXECUTABLE)
	@echo Clean complete!

# Run
run: $(EXECUTABLE)
	@$(EXECUTABLE)

# Rebuild
rebuild: clean all

# Help
help:
	@echo Usage:
	@echo   make          - Build the project
	@echo   make run      - Build and run
	@echo   make clean    - Remove build artifacts
	@echo   make rebuild  - Clean and build

.PHONY: all clean run rebuild help
