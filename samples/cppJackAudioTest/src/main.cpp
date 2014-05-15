#include <iostream> 
#include <unistd.h>
#include <stdlib.h>
#include <jack/jack.h>

#include "PdBase.hpp"
#include "PdObject.h"

int sampleRate = 0;
int ticks = 0;

jack_client_t *client;
jack_port_t *portO1;
jack_port_t *portO2;

pd::PdBase lpd;
PdObject pdObject;

float *output = (float*)malloc(1024*2*sizeof(float));
jack_default_audio_sample_t *out1;
jack_default_audio_sample_t *out2;

// Audio process callback
int process(jack_nframes_t nframes, void *arg){
	 // Get pointers to the output buffers
	 out1 = (jack_default_audio_sample_t *) jack_port_get_buffer(portO1, nframes);
	 out2 = (jack_default_audio_sample_t *) jack_port_get_buffer(portO2, nframes);
 
	 // Pd magic
    ticks = nframes / 64;
	 lpd.processFloat(ticks, NULL, output);
	 
    // Jack uses mono ports and pd expects interleaved stereo buffers.
    for(i=0; i<nframes; i++){
       *out1 = output[i*2];
       *out2 = output[(i*2)+1];
       out1++;
       out2++;
    }

	 return 0;
}  

// Initializes libpd and jack audio.
void init(){
   // Create client:
   if((client = jack_client_open("LibPdTest", JackNullOption, NULL)) == NULL){
      std::cerr << "Jack server not running?" << std::endl;
      exit(1);
   }

   // Get sample rate from jack server
   sampleRate = jack_get_sample_rate(client);

   // Init pd
   if(!lpd.init(0, 2, sampleRate)) {
      std::cerr << "Could not init pd" << std::endl;
      exit(1);
   }

   // Receive messages from pd
   lpd.setReceiver(&pdObject);
   lpd.subscribe("cursor");

   // send DSP 1 message to pd
   lpd.computeAudio(true);

   // load the patch
   pd::Patch patch = lpd.openPatch("test.pd", "./pd");
   std::cout << patch << std::endl;
   
   // Register io ports:
   portO1 = jack_port_register(client, "lalslas", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
   portO2 = jack_port_register(client, "out2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

   // Register audio process callback:
   jack_set_process_callback(client, process, 0);

   //Go!
   if (jack_activate (client)) {
      std::cout << "Could not activate client";
      exit(1);
   }
}


int main (int argc, char *argv[]) {
   init();

   // Keep the program alive until it's killed with ctrl+c
   while(1){
      lpd.receiveMessages();
      usleep(100);
   }

   return 0;
}
