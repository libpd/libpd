/*
 * Created by SharpDevelop.
 * User: Tebjan Halm
 * Date: 08.04.2012
 * Time: 21:33
 * 
 */
 
using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace LibPDBinding
{
	/// <summary>
	/// Convenience Wrapper for native LibPD calls
	/// </summary>
	public class LibPDManager 
	{
		private t_libpd_printhook FPrint_Hook;
		
		protected List<LibPDPatch> FPatches = new List<LibPDPatch>();
		
		/// <summary>
		/// Init LibPD
		/// </summary>
		public LibPDManager()
		{
			LibPD.init();
			
//			FPrint_Hook = new t_libpd_printhook(Print);
//			LibPD.set_printhook(FPrint_Hook);
//			LibPD.test_printhook();
			
		}
		
		//PD Print hook
		protected void Print(string message)
		{
			Debug.WriteLine(message);
		}

		/// <summary>
		/// Initialize or re-initialize the audio system
		/// </summary>
		/// <param name="inChannels">nr of input channels</param>
		/// <param name="outChannels">nr of output channels</param>
		/// <param name="sampleRate">sample rate</param>
		public void InitAudio(int inChannels=2, int outChannels=2, int sampleRate=44100)
		{
			LibPD.init_audio(inChannels, outChannels, sampleRate);
		}
		
		//MESSAGING-------------------------------------------------------------------------------------
		
		/// <summary>
		/// Start audio processing
		/// </summary>
		public void EnableDSP()
		{
			var start = LibPD.start_message(3);
			LibPD.add_float(1);
			var end = LibPD.finish_message("pd", "dsp");
			
			Debug.WriteLine("Enable DSP Start: {0} End: {1}", start, end);
		}
		
		/// <summary>
		/// Stop audio processing
		/// </summary>
		public void DisableDSP()
		{
			var start = LibPD.start_message(3);
			LibPD.add_float(0);
			var end = LibPD.finish_message("pd", "dsp");
			
			Debug.WriteLine("Disable DSP Start: {0} End: {1}", start, end);
		}
		
		/// <summary>
		/// Creates a new dynamic patch
		/// </summary>
		/// <param name="name">optional name of the new patch</param>
		/// <returns>The new patch</returns>
		public LibPDDynamicPatch NewPatch(string name = "")
		{
			if(name == "") name = string.Format("temp-{0:HH-mm-ss-fff}", DateTime.Now.ToLocalTime());
			
			var patch = new LibPDDynamicPatch(name);
			patch.Load();
			return patch;
		}
		
		/// <summary>
		/// Creates a new dynamic patch
		/// </summary>
		/// <param name="name">optional name of the new patch</param>
		/// <returns>The new patch</returns>
		public LibPDPatch LoadPatch(string fileName)
		{
			var patch = new LibPDPatch(fileName);
			patch.Load();
			return patch;
		}

		/// <summary>
		/// Sends a message to PD
		/// </summary>
		/// <param name="message">Message to be sent</param>
		public void SendMessage(LibPDMessage message)
		{
			message.SendTo("pd");
		}
	}
}
