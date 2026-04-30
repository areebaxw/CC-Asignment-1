CC = g++
CXXFLAGS = -std=c++11 -Wall
FLEX = flex
BISON = bison

TARGET = json2xml
SOURCES = AST.cpp parser.tab.c lex.yy.c
HEADERS = AST.h
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) -lfl

parser.tab.c: parser.y
	$(BISON) -d parser.y

lex.yy.c: scanner.l parser.tab.h
	$(FLEX) scanner.l

%.o: %.c $(HEADERS)
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) lex.yy.c parser.tab.c parser.tab.h $(TARGET)

.PHONY: all clean
