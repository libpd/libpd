/*
 * Copyright (c) 2012-2017 Dan Wilcox <danomatika@gmail.com>
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
#pragma once

#include "PdTypes.hpp"

namespace pd {

/// a pd message receiver base class
class PdReceiver {

    public:

        /// receieve a print
        virtual void print(const std::string& message) {};

        /// receive a bang
        virtual void receiveBang(const std::string& dest) {}

        /// receive a float
        virtual void receiveFloat(const std::string& dest, float num) {}

        /// receive a symbol
        virtual void receiveSymbol(const std::string& dest, const std::string& symbol) {}

        /// receive a list
        virtual void receiveList(const std::string& dest, const List& list) {}

        /// receive a named message ie. sent from a message box [; dest msg arg1 arg2 arg3... <
        virtual void receiveMessage(const std::string& dest, const std::string& msg, const List& list) {}
};

} // namespace
