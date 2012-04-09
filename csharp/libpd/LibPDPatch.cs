/*
 * Created by SharpDevelop.
 * User: Tebjan Halm
 * Date: 09.04.2012
 * Time: 10:18
 * 
 * 
 */
using System;
using System.Diagnostics;
using System.IO;

namespace LibPDBinding
{
	/// <summary>
	/// Description of LibPDPatch.
	/// </summary>
	public class LibPDPatch
	{
		private IntPtr FPatchHandle;
		
		public LibPDPatch(string fileName)
		{
			FileName = fileName;
		}
		
		public string FileName
		{
			get;
			private set;
		}
		
		public bool IsLoaded
		{
			get;
			private set;
		}
		
		/// <summary>
		/// Actually load the patch into PD
		/// </summary>
		public void Load()
		{
			if(IsLoaded) Close();
			
			var path = Path.GetDirectoryName(FileName);
			var file = Path.GetFileName(FileName);
			FPatchHandle = LibPD.openfile(file, path);
			Debug.WriteLine("PD File Handle: " + FPatchHandle);
			IsLoaded = true;
		}
		
		/// <summary>
		/// Remove the patch from pd
		/// </summary>
		public void Close()
		{
			if (IsLoaded) LibPD.closefile(FPatchHandle);
			IsLoaded = false;
		}
		
		/// <summary>
		/// Send a message to this patch
		/// </summary>
		/// <param name="message">Message to be sent</param>
		public void SendMessage(LibPDMessage message)
		{
			message.SendTo("pd-" + Path.GetFileName(this.FileName));
		}
	}
}
