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
			LibPD.Bang += new LibPDBangHook(LibPD_Bang);
			LibPD.Float += new LibPDFloatHook(LibPD_Float);
			LibPD.Symbol += new LibPDSymbolHook(LibPD_Symbol);
			LibPD.List += new LibPDListHook(LibPD_List);
			LibPD.Message += new LibPDMessageHook(LibPD_Message);
			
			LibPD.NoteOn += new LibPDNoteOnHook(LibPD_NoteOn);
			LibPD.ControlChange += new LibPDControlChangeHook(LibPD_ControlChange);
			LibPD.ProgramChange += new LibPDProgramChangeHook(LibPD_ProgramChange);
			LibPD.Pitchbend += new LibPDPitchBendHook(LibPD_Pitchbend);
			LibPD.Aftertouch += new LibPDAftertouchHook(LibPD_Aftertouch);
			LibPD.PolyAftertouch += new LibPDPolyAftertouchHook(LibPD_PolyAftertouch);
			LibPD.MidiByte += new LibPDMidiByteHook(LibPD_MidiByte);
			
		}
		
		#region receive events
		
		void LibPD_Print(string recv)
		{
			Debug.Write(recv);
		}
		
		void LibPD_Bang(string recv)
		{
			Debug.WriteLine("Bang from PD: " + recv);
		}
		
		void LibPD_Float(string recv, float x)
		{
			Debug.WriteLine("Float from PD: {0} {1}", recv, x);
		}

		void LibPD_Symbol(string recv, string sym)
		{
			Debug.WriteLine("Symbol from PD: {0} {1}", recv, sym);
		}

		void LibPD_List(string recv, object[] args)
		{
			var msg = new LibPDList(args);
			Debug.WriteLine("List from PD: {0} {1}", recv, msg.ToString());
		}

		void LibPD_Message(string recv, string type, object[] args)
		{
			var msg = new LibPDMessage(type, args);
			Debug.WriteLine("Message from PD: {0} {1}", recv, msg.ToString());
		}

		void LibPD_NoteOn(int channel, int pitch, int velocity)
		{
			Debug.WriteLine("NoteOn from PD: {0} {1} {2}", channel, pitch, velocity);
		}
		
		void LibPD_ControlChange(int channel, int controller, int value)
		{
			Debug.WriteLine("ControlChange from PD: {0} {1} {2}", channel, controller, value);
		}
		
		void LibPD_ProgramChange(int channel, int value)
		{
			Debug.WriteLine("ProgramChange from PD: {0} {1}", channel, value);
		}
		
		void LibPD_Pitchbend(int channel, int value)
		{
			Debug.WriteLine("Pitchbend from PD: {0} {1}", channel, value);
		}

		void LibPD_Aftertouch(int channel, int value)
		{
			Debug.WriteLine("Aftertouch from PD: {0} {1}", channel, value);
		}

		void LibPD_PolyAftertouch(int channel, int pitch, int value)
		{
			Debug.WriteLine("PolyAftertouch from PD: {0} {1} {2}", channel, pitch, value);
		}

		void LibPD_MidiByte(int port, int byt)
		{
			Debug.WriteLine("MidiByte from PD: {0} {1}", port, byt);
		}
	
		#endregion receive events

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
