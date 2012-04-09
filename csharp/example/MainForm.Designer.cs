/*
 * Created by SharpDevelop.
 * User: TF
 * Date: 08.04.2012
 * Time: 20:17
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
namespace LibPDBinding
{
	partial class MainForm
	{
		/// <summary>
		/// Designer variable used to keep track of non-visual components.
		/// </summary>
		private System.ComponentModel.IContainer components = null;
		
		/// <summary>
		/// Disposes resources used by the form.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing) {
				if (components != null) {
					components.Dispose();
				}
			}
			base.Dispose(disposing);
		}
		
		/// <summary>
		/// This method is required for Windows Forms designer support.
		/// Do not change the method contents inside the source code editor. The Forms designer might
		/// not be able to load this method if it was changed manually.
		/// </summary>
		private void InitializeComponent()
		{
			this.comboBoxAsioDevice = new System.Windows.Forms.ComboBox();
			this.textBoxChannelOffset = new System.Windows.Forms.TextBox();
			this.buttonPlay = new System.Windows.Forms.Button();
			this.buttonStop = new System.Windows.Forms.Button();
			this.buttonSelectFile = new System.Windows.Forms.Button();
			this.buttonAsioPanel = new System.Windows.Forms.Button();
			this.buttonCreatePatch = new System.Windows.Forms.Button();
			this.label1 = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			this.SuspendLayout();
			// 
			// comboBoxAsioDevice
			// 
			this.comboBoxAsioDevice.FormattingEnabled = true;
			this.comboBoxAsioDevice.Location = new System.Drawing.Point(12, 12);
			this.comboBoxAsioDevice.Name = "comboBoxAsioDevice";
			this.comboBoxAsioDevice.Size = new System.Drawing.Size(226, 21);
			this.comboBoxAsioDevice.TabIndex = 0;
			this.comboBoxAsioDevice.SelectedIndexChanged += new System.EventHandler(this.ComboBoxAsioDeviceSelectedIndexChanged);
			// 
			// textBoxChannelOffset
			// 
			this.textBoxChannelOffset.Location = new System.Drawing.Point(12, 39);
			this.textBoxChannelOffset.Name = "textBoxChannelOffset";
			this.textBoxChannelOffset.Size = new System.Drawing.Size(23, 20);
			this.textBoxChannelOffset.TabIndex = 1;
			this.textBoxChannelOffset.Text = "0";
			// 
			// buttonPlay
			// 
			this.buttonPlay.Location = new System.Drawing.Point(12, 131);
			this.buttonPlay.Name = "buttonPlay";
			this.buttonPlay.Size = new System.Drawing.Size(101, 63);
			this.buttonPlay.TabIndex = 2;
			this.buttonPlay.Text = "Start";
			this.buttonPlay.UseVisualStyleBackColor = true;
			this.buttonPlay.Click += new System.EventHandler(this.buttonPlay_Click);
			// 
			// buttonStop
			// 
			this.buttonStop.Location = new System.Drawing.Point(135, 131);
			this.buttonStop.Name = "buttonStop";
			this.buttonStop.Size = new System.Drawing.Size(103, 63);
			this.buttonStop.TabIndex = 3;
			this.buttonStop.Text = "Stop";
			this.buttonStop.UseVisualStyleBackColor = true;
			this.buttonStop.Click += new System.EventHandler(this.buttonStop_Click);
			// 
			// buttonSelectFile
			// 
			this.buttonSelectFile.Location = new System.Drawing.Point(12, 93);
			this.buttonSelectFile.Name = "buttonSelectFile";
			this.buttonSelectFile.Size = new System.Drawing.Size(101, 23);
			this.buttonSelectFile.TabIndex = 4;
			this.buttonSelectFile.Text = "Open PD File";
			this.buttonSelectFile.UseVisualStyleBackColor = true;
			this.buttonSelectFile.Click += new System.EventHandler(this.buttonSelectFile_Click);
			// 
			// buttonAsioPanel
			// 
			this.buttonAsioPanel.Location = new System.Drawing.Point(135, 39);
			this.buttonAsioPanel.Name = "buttonAsioPanel";
			this.buttonAsioPanel.Size = new System.Drawing.Size(103, 23);
			this.buttonAsioPanel.TabIndex = 5;
			this.buttonAsioPanel.Text = "ASIO Control";
			this.buttonAsioPanel.UseVisualStyleBackColor = true;
			this.buttonAsioPanel.Click += new System.EventHandler(this.ButtonAsioPanelClick);
			// 
			// buttonCreatePatch
			// 
			this.buttonCreatePatch.Location = new System.Drawing.Point(135, 93);
			this.buttonCreatePatch.Name = "buttonCreatePatch";
			this.buttonCreatePatch.Size = new System.Drawing.Size(103, 23);
			this.buttonCreatePatch.TabIndex = 6;
			this.buttonCreatePatch.Text = "Create Patch";
			this.buttonCreatePatch.UseVisualStyleBackColor = true;
			this.buttonCreatePatch.Click += new System.EventHandler(this.ButtonCreatePatchClick);
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(41, 42);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(77, 17);
			this.label1.TabIndex = 7;
			this.label1.Text = "Channel Offset";
			// 
			// label2
			// 
			this.label2.Location = new System.Drawing.Point(281, 273);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(74, 17);
			this.label2.TabIndex = 8;
			this.label2.Text = "Who cares?";
			// 
			// MainForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(251, 210);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.label1);
			this.Controls.Add(this.buttonCreatePatch);
			this.Controls.Add(this.buttonAsioPanel);
			this.Controls.Add(this.buttonSelectFile);
			this.Controls.Add(this.buttonStop);
			this.Controls.Add(this.buttonPlay);
			this.Controls.Add(this.textBoxChannelOffset);
			this.Controls.Add(this.comboBoxAsioDevice);
			this.Name = "MainForm";
			this.Text = "libpd demo";
			this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.MainFormFormClosed);
			this.ResumeLayout(false);
			this.PerformLayout();
		}
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Button buttonCreatePatch;
		private System.Windows.Forms.Button buttonAsioPanel;
		private System.Windows.Forms.Button buttonSelectFile;
		private System.Windows.Forms.Button buttonPlay;
		private System.Windows.Forms.Button buttonStop;
		private System.Windows.Forms.TextBox textBoxChannelOffset;
		private System.Windows.Forms.ComboBox comboBoxAsioDevice;
	}
}
