/*
 * Copyright (c) 2011 Dan Wilcox <danomatika@gmail.com>
 *
 * BSD Simplified License.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * See https://github.com/danomatika/ofxPd for documentation
 *
 * This file created by Marek Bereza on 24/01/2011. Updated by Dan Wilcox 2011.
 *
 */
#pragma once

#include "PdTypes.hpp"

namespace pd {

/// a pd message receiver base class
class PdReceiver {

	public:
	
		/// print
		virtual void print(const std::string& message) {};
		
		/// messages
		virtual void receiveBang(const std::string& dest) {}
		virtual void receiveFloat(const std::string& dest, float value) {}
		virtual void receiveSymbol(const std::string& dest, const std::string& symbol) {}
		virtual void receiveList(const std::string& dest, const List& list) {}
		virtual void receiveMessage(const std::string& dest, const std::string& msg, const List& list) {}
};

} // namespace
