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
	/// A LibPDPatch from a file
	/// </summary>
	public class LibPDPatch
	{
		public IntPtr PatchHandle
		{
			get;
			private set;
		}
		
		public LibPDPatch(string fileName)
		{
			FileName = fileName;
			Name = Path.GetFileName(fileName);
		}
		
		/// <summary>
		/// File path on disk
		/// </summary>
		public string FileName
		{
			get;
			private set;
		}
		
		/// <summary>
		/// PDs internal name
		/// </summary>
		public string Name
		{
			get;
			private set;
		}
		
		/// <summary>
		/// indicates whether the patch is loaded in PD
		/// </summary>
		public bool IsLoaded
		{
			get;
			protected set;
		}
		
		/// <summary>
		/// Actually load the patch into PD
		/// </summary>
		public virtual void Load()
		{
			if(IsLoaded) Close();
			
			PatchHandle = LibPD.Openfile(this.FileName);
			Debug.WriteLine("PD File Handle: " + PatchHandle);
			IsLoaded = true;
		}
		
		/// <summary>
		/// Remove the patch from PD
		/// </summary>
		public virtual void Close()
		{
			if (IsLoaded) LibPD.CloseFile(PatchHandle);
			IsLoaded = false;
		}
		
		/// <summary>
		/// Send a message to this patch
		/// </summary>
		/// <param name="message">Message to be sent</param>
		public void SendMessage(LibPDMessage message)
		{
			message.SendTo("pd-" + this.Name);
		}

	}
	
	/// <summary>
	/// A dynamic LibPDPatch.
	/// </summary>
	public class LibPDDynamicPatch : LibPDPatch
	{
		public LibPDDynamicPatch(string name)
			: base(Path.Combine(Path.GetTempPath(), name + ".pd"))
		{
		}
		
		/// <summary>
		/// Actually load the patch into PD
		/// </summary>
		public override void Load()
		{
        	using (StreamWriter myWriter = File.CreateText(this.FileName))
        	{
        		//setup empty canvas
        		myWriter.WriteLine(@"#N canvas 0 0 450 300 10");
        	}
        	
        	base.Load();
        	
        	//delete temp file
        	File.Delete(this.FileName);
		}
		

		
	}
}
