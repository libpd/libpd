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
		
		protected List<LibPDPatch> FPatches = new List<LibPDPatch>();
		
		/// <summary>
		/// Init LibPD
		/// </summary>
		public LibPDManager()
		{
			LibPD.WriteMessageToDebug = true;
			LibPD.Print += new LibPDPrintHook(LibPD_Print);
			LibPD.Message += new LibPDMessageStringHook(LibPD_Message);
			
			LibPD.bind("toCPP");
		}

		void LibPD_Message(string recv, string msg, int argc, string argv)
		{
			Debug.WriteLine("Message from PD: {0} {1} count={2} args={3}", recv, msg, argc, argv);
		}

		void LibPD_Print(string recv)
		{
			Debug.Write(recv);
		}
		
		//PD Print hook
		protected void Print(string message)
		{
			Debug.WriteLine(message);
		}

		//AUDIO---------------------------------------------------------------------------
		
		/// <summary>
		/// Initialize or re-initialize the audio system
		/// </summary>
		/// <param name="inChannels">nr of input channels</param>
		/// <param name="outChannels">nr of output channels</param>
		/// <param name="sampleRate">sample rate</param>
		public void InitAudio(int inChannels=2, int outChannels=2, int sampleRate=44100)
		{
			LibPD.InitAudio(inChannels, outChannels, sampleRate);
		}
		
		/// <summary>
		/// Start audio processing
		/// </summary>
		public void EnableDSP()
		{
			LibPD.ComputeAudio(true);
		}
		
		/// <summary>
		/// Stop audio processing
		/// </summary>
		public void DisableDSP()
		{
			LibPD.ComputeAudio(false);
		}
		
		//PATCH HANDLING--------------------------------------------------------------------------
		
		/// <summary>
		/// Creates a new dynamic patch and adds ot to PD
		/// </summary>
		/// <param name="name">optional name of the new patch</param>
		/// <returns>The new patch</returns>
		public LibPDDynamicPatch NewPatch(string name = "")
		{
			if(name == "") name = string.Format("temp-{0:HH-mm-ss-fff}", DateTime.Now.ToLocalTime());
			
			var patch = new LibPDDynamicPatch(name);
			AddPatch(patch);
			return patch;
		}
		
		/// <summary>
		/// Creates a new dynamic patch and adds it to PD
		/// </summary>
		/// <param name="name">optional name of the new patch</param>
		/// <returns>The new patch</returns>
		public LibPDPatch LoadPatch(string fileName)
		{
			var patch = new LibPDPatch(fileName);
			AddPatch(patch);
			return patch;
		}
		
		/// <summary>
		/// Add a patch to PD
		/// </summary>
		/// <param name="patch"></param>
		public void AddPatch(LibPDPatch patch)
		{
			patch.Load();
			this.FPatches.Add(patch);
		}
		
		/// <summary>
		/// Removes a patch from PD
		/// </summary>
		/// <param name="patch"></param>
		public void RemovePatch(LibPDPatch patch)
		{
			patch.Close();
			FPatches.Remove(patch);
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
