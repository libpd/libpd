/*
 * Copyright (c) 2012 Dan Wilcox <danomatika@gmail.com>
 *
 * BSD Simplified License.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * See https://github.com/libpd/libpd for documentation
 *
 * This file was originally written for the ofxPd openFrameworks addon:
 * https://github.com/danomatika/ofxPd
 *
 */
#include "PdTypes.hpp"

#include <iostream>
#include <sstream>

namespace pd {

/// PATCH
//----------------------------------------------------------
Patch::Patch() : _handle(NULL), _dollarZero(0), _dollarZeroStr("0"),
    _filename(""), _path("") {}

Patch::Patch(const std::string& filename, const std::string& path) :
    _handle(NULL), _dollarZero(0), _dollarZeroStr("0"),
    _filename(filename), _path(path) {}

Patch::Patch(void* handle, int dollarZero, const std::string& filename, const std::string& path) :
    _handle(handle), _dollarZero(dollarZero), _filename(filename), _path(path) {
    std::stringstream itoa;
    itoa << dollarZero;
    _dollarZeroStr = itoa.str();
}

bool Patch::isValid() const {
    if(_handle == NULL)
        return false;
    return true;
}

void Patch::clear() {
    _handle = NULL;
    _dollarZero = 0;
    _dollarZeroStr = "0";
}

Patch::Patch(const Patch& from) {
    _handle = from._handle;
    _dollarZero = from._dollarZero;
    _dollarZeroStr = from._dollarZeroStr;
    _filename = from._filename;
    _path = from._path;
}

void Patch::operator=(const Patch& from) {
    _handle = from._handle;
    _dollarZero = from._dollarZero;
    _dollarZeroStr = from._dollarZeroStr;
    _filename = from._filename;
    _path = from._path;
}

std::ostream& operator<<(std::ostream& os, const Patch& from) {
    return os << "Patch: \"" << from.filename() << "\" $0: " << from.dollarZeroStr()
                 << " valid: " << from.isValid();
}

/// LIST
//----------------------------------------------------------
List::List() {}

bool List::isFloat(const unsigned int index) const {
    if(index < objects.size())
        if(objects[index].type == List::FLOAT)
            return true;
    return false;
}

bool List::isSymbol(const unsigned int index) const {
    if(index < objects.size())
        if(objects[index].type == List::SYMBOL)
            return true;
    return false;
}

//----------------------------------------------------------
float List::getFloat(const unsigned int index) const {
    if(!isFloat(index)) {
        std::cerr << "Pd: List: object " << index << " is not a float" << std::endl;
        return 0;
    }
    return objects[index].value;
}

std::string List::getSymbol(const unsigned int index) const {
    if(!isSymbol(index)) {
        std::cerr << "Pd: List: object " << index << " is not a symbol" << std::endl;
        return "";
    }
    return objects[index].symbol;
}

//----------------------------------------------------------
void List::addFloat(const float num) {
    MsgObject o;
    o.type = List::FLOAT;
    o.value = num;
    objects.push_back(o);
    typeString += 'f';
}

void List::addSymbol(const std::string& symbol) {
    MsgObject o;
    o.type = List::SYMBOL;
    o.symbol = symbol;
    objects.push_back(o);
    typeString += 's';
}

List& List::operator<<(const bool var) {
    addFloat((float) var);
    return *this;
}

List& List::operator<<(const int var) {
    addFloat((float) var);
    return *this;
}

List& List::operator<<(const float var) {
    addFloat((float) var);
    return *this;
}

List& List::operator<<(const double var) {
    addFloat((float) var);
    return *this;
}

List& List::operator<<(const char var) {
    std::string s;
    s = var;
    addSymbol(s);
    return *this;
}

List& List::operator<<(const char* var) {
    addSymbol((std::string) var);
    return *this;
}

List& List::operator<<(const std::string& var) {
    addSymbol((std::string) var);
    return *this;
}

//----------------------------------------------------------
const unsigned int List::len() const {
    return (unsigned int) objects.size();
}

const std::string& List::types() const {
    return typeString;
}

void List::clear() {
    typeString = "";
    objects.clear();
}

std::string List::toString() const {

    std::string line;
    std::stringstream itoa;

    for(int i = 0; i < objects.size(); ++i) {
        if(isFloat(i)) {
            itoa << getFloat(i);
            line += itoa.str();
            itoa.str("");
        }
        else
            line += getSymbol(i);
        line += " ";
    }

    return line;
}

std::ostream& operator<<(std::ostream& os, const List& from) {
    return os << from.toString();
}

/// MESSAGE
//----------------------------------------------------------
Message::Message() : type(NONE) {
    clear();
}

Message::Message(MessageType type) {
    clear();
    this->type = type;
}

void Message::clear() {
    type = NONE;
    dest = "";
    num = 0;
    symbol = "";
    list.clear();
    channel = 0;
    pitch = 0;
    velocity = 0;
    controller = 0;
    value = 0;
    port = 0;
    byte = 0;
}

} // namespace
