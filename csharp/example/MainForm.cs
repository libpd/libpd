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
		private LibPDManager FLibPDManager;
		private LibPDPatch FLibPDPatch;
		private LibPDWaveProvider FLibPDReader;
        private AsioOut FAsioOut;

		
		
		public MainForm()
		{
			// The InitializeComponent() call is required for Windows Forms designer support.
			InitializeComponent();			
			
			//libpd helper
			FLibPDManager = new LibPDManager();
        	FLibPDManager.InitAudio();
            
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

        }
		
        //open file
        private void buttonSelectFile_Click(object sender, EventArgs e)
        {
        	if(FLibPDPatch != null) FLibPDPatch.Close();
        		
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Filter = "PD files|*.pd";
            if (ofd.ShowDialog() == DialogResult.OK)
            {
            	FLibPDPatch = FLibPDManager.LoadPatch(ofd.FileName);
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
        	FLibPDManager.InitAudio();
        	FLibPDManager.EnableDSP();
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
        	FLibPDManager.DisableDSP();
        	SetButtonStates();
        }
        
        //gui
        private void SetButtonStates()
        {
        	buttonPlay.Enabled = FAsioOut != null && FAsioOut.PlaybackState != PlaybackState.Playing;
        	buttonStop.Enabled = FAsioOut != null && FAsioOut.PlaybackState == PlaybackState.Playing;
        }

        //setup a patch on the fly
        void ButtonCreatePatchClick(object sender, EventArgs e)
        {
        	
        	if (FLibPDPatch != null) FLibPDPatch.Close();
        	
        	FLibPDPatch = FLibPDManager.NewPatch();
        	
        	FLibPDPatch.SendMessage(new LibPDObjMessage(10, 150, "dac~"));
        	FLibPDPatch.SendMessage(new LibPDObjMessage(10, 100, "*~", 0.2));
        	FLibPDPatch.SendMessage(new LibPDObjMessage(10, 70, "osc~", 440));
        	FLibPDPatch.SendMessage(new LibPDConnectMessage(1, 0, 0, 0));
        	FLibPDPatch.SendMessage(new LibPDConnectMessage(1, 0, 0, 1));
        	FLibPDPatch.SendMessage(new LibPDConnectMessage(2, 0, 1, 0));
        	
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

		private LibPDMessage ParseMessage()
		{
			return LibPDMessage.ParseTypedMessage(this.textBoxMessage.Text);
		}
		
		void ButtonPatchClick(object sender, EventArgs e)
		{
			FLibPDPatch.SendMessage(ParseMessage());
		}
		
		void ButtonPDClick(object sender, EventArgs e)
		{
			FLibPDManager.SendMessage(ParseMessage());
		}
		
		void ButtonCustomClick(object sender, EventArgs e)
		{
			ParseMessage().SendTo(this.textBoxReceiver.Text);
		}
	}
}
