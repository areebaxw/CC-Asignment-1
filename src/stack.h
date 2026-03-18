#pragma once
#include <vector>
#include <string>
#include <stdexcept>

using namespace std;

// Simple stack using a vector underneath.
// Top of the stack = back of the vector.
class Stack {
private:
    vector<string> data;

public:
    void push(const string& s) {
        data.push_back(s);
    }

    void pop() {
        if (data.empty()) throw runtime_error("Stack underflow");
        data.pop_back();
    }

    string top() const {
        if (data.empty()) throw runtime_error("Stack is empty");
        return data.back();
    }

    bool isEmpty() const {
        return data.empty();
    }

    // Return full stack contents as a string (bottom -> top, left -> right)
    string toString() const {
        string result;
        for (const auto& s : data) {
            result += s + " ";
        }
        return result;
    }
};
