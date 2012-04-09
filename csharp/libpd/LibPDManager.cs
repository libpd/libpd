/*
 * Created by SharpDevelop.
 * User: Tebjan Halm
 * Date: 08.04.2012
 * Time: 21:33
 * 
 */
 
using System;
using System.Diagnostics;

namespace LibPDBinding
{
	/// <summary>
	/// Convenience Wrapper for native LibPD calls
	/// </summary>
	public class LibPDManager 
	{
		private t_libpd_printhook FPrint_Hook;
		
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

		
		public void InitAudio(int inChannels=2, int outChannels=2, int sampleRate=44100)
		{
			LibPD.init_audio(inChannels, outChannels, sampleRate);
		}
		
		//MESSAGING-------------------------------------------------------------------------------------
		
		public void EnableDSP()
		{
			var start = LibPD.start_message(3);
			LibPD.add_float(1);
			var end = LibPD.finish_message("pd", "dsp");
			
			Debug.WriteLine("Enable DSP Start: {0} End: {1}", start, end);
		}
		
		public void DisableDSP()
		{
			var start = LibPD.start_message(3);
			LibPD.add_float(0);
			var end = LibPD.finish_message("pd", "dsp");
			
			Debug.WriteLine("Disable DSP Start: {0} End: {1}", start, end);
		}
		
		public void SendCanavasMessage(float xPos, float yPos, float width, float height, float fonSize)
		{
			var start = LibPD.start_message(7);
			LibPD.add_symbol("canvas");
			LibPD.add_float(xPos);
			LibPD.add_float(yPos);
			LibPD.add_float(width);
			LibPD.add_float(height);
			LibPD.add_float(fonSize);
			var end = LibPD.finish_list("#N");
			
			Debug.WriteLine("Canvas Start: {0} End: {1}", start, end);
		}
		
		public void SendMessage(LibPDMessage message)
		{
			message.SendTo("pd");
		}
	}
}
