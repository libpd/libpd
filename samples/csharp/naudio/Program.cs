/*
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 * 
 * Copyright(c) 2016 Thomas Mayer<thomas@residuum.org>
 */
using System;
using System.Linq;
using JackSharp;
using NAudio.CoreAudioApi;
using NAudio.Jack;
using NAudio.Wave;

namespace LibPdBindingNaudio
{
	class Program
	{
		static bool useJack;

		static void Main (string[] args)
		{
			if (args.Any () && args.First () == "jack") {
				useJack = true;
			}
			if (useJack) {
				PlayWithJack ();
			} else {
				PlayWithWasapi ();
			}
		}

		static void PlayWithJack ()
		{
			using (Processor processor = new Processor ("PdTest", 0, 2, 0, 0, true))
			using (IWavePlayer output = new JackOut (processor))
			using (NewApiPdProvider pd = new NewApiPdProvider ()) {
				output.Init (pd);
				output.Play ();
				Console.ReadLine ();
				output.Stop ();
			}
		}

		static void PlayWithWasapi ()
		{
			using (IWavePlayer output = new WasapiOut (AudioClientShareMode.Shared, 10))
			using (NewApiPdProvider pd = new NewApiPdProvider ()) {
				output.Init (pd);
				output.Play ();
				Console.ReadLine ();
				output.Stop ();
			}
		}
	}
}