/*
 * Created by SharpDevelop.
 * User: Tebjan Halm
 * Date: 08.04.2012
 * Time: 20:17
 *
 */
 
using System;
using System.IO;
using System.Diagnostics;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;
using NAudio.Wave;

namespace LibPDBinding
{
	/// <summary>
	/// Description of MainForm.
	/// </summary>
	public partial class MainForm : Form
	{
		private LibPDWaveProvider FLibPDReader;
        private AsioOut FAsioOut;
        private int FLibPDPatch;

		public MainForm()
		{
			// The InitializeComponent() call is required for Windows Forms designer support.
			InitializeComponent();			
            
			//Asio setup
			this.FLibPDReader = new LibPDWaveProvider();
			
			foreach(var device in AsioOut.GetDriverNames())
			{
				this.comboBoxAsioDevice.Items.Add(device);
			}
			
			if (this.comboBoxAsioDevice.Items.Count > 1)
			{
				this.comboBoxAsioDevice.SelectedIndex = 0;
			}
			else
			{
				this.comboBoxAsioDevice.Items.Add("No ASIO!? -> go download ASIO4All");
			}
			
            SetButtonStates();
            
            LibPD.WriteMessageToDebug = true;
            LibPD.Print += new LibPDPrint(LibPD_Print);
            Debug.WriteLine("start");
            
		}

		void LibPD_Print(string text)
		{
			Debug.WriteLine(text);
		}
		
        //open file
        private void buttonSelectFile_Click(object sender, EventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Filter = "PD files|*.pd";
            if (ofd.ShowDialog() == DialogResult.OK)
            {
            	FLibPDPatch = LibPD.OpenPatch(ofd.FileName);
            }
            
        }

        //play
        private void buttonPlay_Click(object sender, EventArgs args)
        {
            try
            {
                Play();
            }
            catch (Exception e)
            {
                MessageBox.Show(e.Message);
            }
        }

        private void Play()
        {
        	LibPD.OpenAudio(2, 2, 44100);
        	LibPD.ComputeAudio(true);
            this.FAsioOut.Play();
            SetButtonStates();
        }

        //stop
        private void buttonStop_Click(object sender, EventArgs e)
        {
        	Stop();
        }
        
        private void Stop()
        {
        	this.FAsioOut.Stop();
        	LibPD.ComputeAudio(false);
        	SetButtonStates();
        }
        
        //gui
        private void SetButtonStates()
        {
        	buttonPlay.Enabled = FAsioOut != null && FAsioOut.PlaybackState != PlaybackState.Playing;
        	buttonStop.Enabled = FAsioOut != null && FAsioOut.PlaybackState == PlaybackState.Playing;
        }
		
		//ASIO stuff---------------------------------------------------------------------
		
		void ButtonAsioPanelClick(object sender, EventArgs e)
		{
			FAsioOut.ShowControlPanel();
		}
		
		void ComboBoxAsioDeviceSelectedIndexChanged(object sender, EventArgs e)
		{
			// allow change device
			if (this.FAsioOut != null &&
			    (this.FAsioOut.DriverName != comboBoxAsioDevice.Text ||
			     this.FAsioOut.ChannelOffset != GetUserSpecifiedChannelOffset()))
			{
				this.FAsioOut.Dispose();
				this.FAsioOut = null;
			}
			
			CreateAsio();
		}
		
		//init driver
        private void CreateAsio()
        {
        	//recreate device if necessary
        	if (this.FAsioOut != null)
        	{
        		Cleanup();
        	}
        	
        	this.FAsioOut = new AsioOut(comboBoxAsioDevice.Text);
        	this.FAsioOut.ChannelOffset = GetUserSpecifiedChannelOffset();
        	this.FAsioOut.Init(FLibPDReader);
        }
        
        //channel offset
        private int GetUserSpecifiedChannelOffset()
        {
        	int channelOffset = 0;
        	int.TryParse(textBoxChannelOffset.Text, out channelOffset);
        	return channelOffset;
        }
        
		//close
		void MainFormFormClosed(object sender, FormClosedEventArgs e)
		{
			Cleanup();
		}
		
		//close ASIO
		private void Cleanup()
		{
			if (this.FAsioOut != null)
			{
				this.FAsioOut.Dispose();
				this.FAsioOut = null;
			}
		}
		
		void ButtonPatchClick(object sender, EventArgs e)
		{
		}
		
		void ButtonPDClick(object sender, EventArgs e)
		{
		}
		
		void ButtonCustomClick(object sender, EventArgs e)
		{
		}
		
		void ButtonMidiClick(object sender, EventArgs e)
		{
			LibPD.SendNoteOn(0, 56, 78);
			LibPD.SendControlChange(0, 56, 78);
			LibPD.SendProgramChange(0, 56);
			LibPD.SendPitchbend(0, 56);
			LibPD.SendAftertouch(0, 56);
			LibPD.SendPolyAftertouch(0, 56, 78);
			LibPD.SendMidiByte(0, 56);
			LibPD.SendSysex(0, 56);
			LibPD.SendSysRealtime(0, 56);
		}
		
		void ButtonMessagesClick(object sender, EventArgs e)
		{
			var recv = this.textBoxReceiver.Text;
			LibPD.SendBang(recv);
			LibPD.SendFloat(recv, 123.45f);
			LibPD.SendSymbol(recv, "a string");
		}
		
		void ButtonArrayClick(object sender, EventArgs e)
		{
			var arr = new float[3];
			
			LibPD.ReadArray(arr, "array1", 0, 3);

			Debug.WriteLine("Array: " + string.Concat(arr));
		
			arr[0] = 10;
			arr[1] = 20;
			arr[2] = 30;
			
			LibPD.WriteArray("array1", 0, arr, 3);
			
			Debug.WriteLine("Got array size of: " + LibPD.ArraySize("array1"));
			
		}
	}
}
