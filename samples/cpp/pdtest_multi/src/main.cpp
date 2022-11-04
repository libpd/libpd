/*
 * Copyright (c) 2022 Dan Wilcox <danomatika@gmail.com>
 *
 * BSD Simplified License.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * See https://github.com/libpd/libpd for documentation
 *
 */
#include <stdlib.h>
#include <iostream>
#include "PdBase.hpp"
#include "PdObject.h"

int main(int argc, char **argv) {

	// our pd engines
	pd::PdBase pd1, pd2;
	std::cout << pd::PdBase::numInstances() << " instances" << std::endl;
	
	// one input channel, two output channels
	// block size 64, one tick per buffer
	float inbuf[64], outbuf[128];
	
	// custom receiver objects for messages and midi
	PdObject pdObject1(1), pdObject2(2);

	// init instance 1
	int srate = 44100;
	if(!pd1.init(1, 2, srate)) {
		std::cerr << "could not init pd1" << std::endl;
		exit(1);
	}
	pd1.setReceiver(&pdObject1);
	pd1.setMidiReceiver(&pdObject1);
	pd1.computeAudio(true);
	pd1.openPatch("test.pd", ".");

	// init instance 2
	if(!pd2.init(1, 2, srate)) {
		std::cerr << "could not init pd2" << std::endl;
		exit(1);
	}
	pd2.setReceiver(&pdObject2);
	pd2.setMidiReceiver(&pdObject2);
	pd2.computeAudio(true);
	pd2.openPatch("test.pd", ".");

	// [; pd frequency 480 (
	pd1.startMessage();
	pd1.addFloat(480);
	pd1.finishMessage("frequency", "float");

	// [; pd frequency -480 (
	pd2.startMessage();
	pd2.addFloat(-480);
	pd2.finishMessage("frequency", "float");

	// now run pd for 3 ticks
	int i, j;
	for(i = 0; i < 3; i++) {

		pd1.processFloat(1, inbuf, outbuf);
		std::cout << "instance 1, tick " << i << ":" << std::endl;
		for(j = 0; j < 8; j++) {
			std::cout << outbuf[j] << " ";
		}
		std::cout << "... " << std::endl;

		pd2.processFloat(1, inbuf, outbuf);
		std::cout << "instance 2, tick " << i << ":" << std::endl;
		for(j = 0; j < 8; j++) {
			std::cout << outbuf[j] << " ";
		}
		std::cout << "... " << std::endl;
	}

	pd1.closePatch("test");
	pd1.clear();

	pd2.closePatch("test");
	pd2.clear();
	
  return 0;
}
