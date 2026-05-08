CC = g++
CXXFLAGS = -std=c++11 -Wall
FLEX = flex
BISON = bison

TARGET = json2xml
CSOURCES = parser.tab.c lex.yy.c
CPPSOURCES = AST.cpp
HEADERS = AST.h
COBJECTS = $(CSOURCES:.c=.o)
CPPOBJECTS = $(CPPSOURCES:.cpp=.o)
OBJECTS = $(COBJECTS) $(CPPOBJECTS)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) -lfl

parser.tab.c: parser.y
	$(BISON) -d parser.y

lex.yy.c: scanner.l parser.tab.h
	$(FLEX) scanner.l

AST.o: AST.cpp AST.h
	$(CC) $(CXXFLAGS) -c AST.cpp -o AST.o

%.o: %.c $(HEADERS)
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) lex.yy.c parser.tab.c parser.tab.h $(TARGET)

.PHONY: all clean
