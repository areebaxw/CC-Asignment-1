#include "AST.h"
#include <sstream>

string escapeXML(const string& s) {
    string result;
    for (char c : s) {
        switch (c) {
            case '&': result += "&amp;"; break;
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '"': result += "&quot;"; break;
            case '\'': result += "&apos;"; break;
            default: result += c; break;
        }
    }
    return result;
}

void StringNode::toXML(ostream& out, const string& tagName, int indent) const {
    if (!tagName.empty()) {
        for (int i = 0; i < indent; i++) out << "  ";
        out << "<" << tagName << ">" << escapeXML(value) << "</" << tagName << ">" << endl;
    }
}

void NumberNode::toXML(ostream& out, const string& tagName, int indent) const {
    if (!tagName.empty()) {
        for (int i = 0; i < indent; i++) out << "  ";
        out << "<" << tagName << ">" << value << "</" << tagName << ">" << endl;
    }
}

void BoolNode::toXML(ostream& out, const string& tagName, int indent) const {
    if (!tagName.empty()) {
        for (int i = 0; i < indent; i++) out << "  ";
        out << "<" << tagName << ">" << (value ? "true" : "false") << "</" << tagName << ">" << endl;
    }
}

void NullNode::toXML(ostream& out, const string& tagName, int indent) const {
    if (!tagName.empty()) {
        for (int i = 0; i < indent; i++) out << "  ";
        out << "<" << tagName << "/>" << endl;
    }
}

void ArrayNode::toXML(ostream& out, const string& tagName, int indent) const {
    if (!tagName.empty()) {
        for (int i = 0; i < indent; i++) out << "  ";
        out << "<" << tagName << ">" << endl;
    }
    for (auto elem : elements) {
        elem->toXML(out, "item", indent + 1);
    }
    if (!tagName.empty()) {
        for (int i = 0; i < indent; i++) out << "  ";
        out << "</" << tagName << ">" << endl;
    }
}

void ObjectNode::toXML(ostream& out, const string& tagName, int indent) const {
    if (!tagName.empty()) {
        for (int i = 0; i < indent; i++) out << "  ";
        out << "<" << tagName << ">" << endl;
    }
    for (auto& pair : members) {
        pair.second->toXML(out, pair.first, indent + 1);
    }
    if (!tagName.empty()) {
        for (int i = 0; i < indent; i++) out << "  ";
        out << "</" << tagName << ">" << endl;
    }
}
