#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <map>
#include <iostream>

using namespace std;

enum NodeType {
    OBJECT,
    ARRAY,
    STRING,
    NUMBER,
    BOOL,
    NULL_NODE
};

class ASTNode {
public:
    NodeType type;
    
    ASTNode(NodeType t) : type(t) {}
    virtual ~ASTNode() {}
    
    virtual void print(int indent = 0) const = 0;
    virtual void toXML(ostream& out, const string& tagName = "", int indent = 0) const = 0;
};

class StringNode : public ASTNode {
public:
    string value;
    
    StringNode(const string& v) : ASTNode(STRING), value(v) {}
    
    void print(int indent = 0) const override {
        for (int i = 0; i < indent; i++) cout << "  ";
        cout << "STRING: \"" << value << "\"" << endl;
    }
    
    void toXML(ostream& out, const string& tagName = "", int indent = 0) const override;
};

class NumberNode : public ASTNode {
public:
    string value;
    
    NumberNode(const string& v) : ASTNode(NUMBER), value(v) {}
    
    void print(int indent = 0) const override {
        for (int i = 0; i < indent; i++) cout << "  ";
        cout << "NUMBER: " << value << endl;
    }
    
    void toXML(ostream& out, const string& tagName = "", int indent = 0) const override;
};

class BoolNode : public ASTNode {
public:
    bool value;
    
    BoolNode(bool v) : ASTNode(BOOL), value(v) {}
    
    void print(int indent = 0) const override {
        for (int i = 0; i < indent; i++) cout << "  ";
        cout << "BOOL: " << (value ? "true" : "false") << endl;
    }
    
    void toXML(ostream& out, const string& tagName = "", int indent = 0) const override;
};

class NullNode : public ASTNode {
public:
    NullNode() : ASTNode(NULL_NODE) {}
    
    void print(int indent = 0) const override {
        for (int i = 0; i < indent; i++) cout << "  ";
        cout << "NULL" << endl;
    }
    
    void toXML(ostream& out, const string& tagName = "", int indent = 0) const override;
};

class ArrayNode : public ASTNode {
public:
    vector<ASTNode*> elements;
    
    ArrayNode() : ASTNode(ARRAY) {}
    ~ArrayNode() {
        for (auto elem : elements) {
            delete elem;
        }
    }
    
    void addElement(ASTNode* elem) {
        elements.push_back(elem);
    }
    
    void print(int indent = 0) const override {
        for (int i = 0; i < indent; i++) cout << "  ";
        cout << "ARRAY [" << endl;
        for (auto elem : elements) {
            elem->print(indent + 1);
        }
        for (int i = 0; i < indent; i++) cout << "  ";
        cout << "]" << endl;
    }
    
    void toXML(ostream& out, const string& tagName = "", int indent = 0) const override;
};

class ObjectNode : public ASTNode {
public:
    map<string, ASTNode*> members;
    
    ObjectNode() : ASTNode(OBJECT) {}
    ~ObjectNode() {
        for (auto& pair : members) {
            delete pair.second;
        }
    }
    
    void addMember(const string& key, ASTNode* value) {
        members[key] = value;
    }
    
    void print(int indent = 0) const override {
        for (int i = 0; i < indent; i++) cout << "  ";
        cout << "OBJECT {" << endl;
        for (auto& pair : members) {
            for (int i = 0; i < indent + 1; i++) cout << "  ";
            cout << pair.first << ":" << endl;
            pair.second->print(indent + 2);
        }
        for (int i = 0; i < indent; i++) cout << "  ";
        cout << "}" << endl;
    }
    
    void toXML(ostream& out, const string& tagName = "", int indent = 0) const override;
};

string escapeXML(const string& s);

#endif
