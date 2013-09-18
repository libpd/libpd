#ifndef _PDXBEE
#define _PDXBEE

// MAX_XBEE_PACKET_LENGTH is around 80
#define MAX_XBEE_PACKET_LENGTH 128

#define LENGTH_MSB_INDEX 1 /* offset in x_outbuf */
#define LENGTH_LSB_INDEX 2 /* offset in x_outbuf */
#define FRAME_TYPE_INDEX 3 /* offset in x_outbuf */
#define FRAME_ID_INDEX 4 /* offset in x_outbuf */
#define AT_COMMAND_INDEX 5 /* offset in x_outbuf */
#define AT_PARAMETER_INDEX 6 /* offset in x_outbuf */

/* API Frame Names and Values */

#define AT_Command                                  0x08
#define AT_Command_Queue_Parameter_Value            0x09
#define ZigBee_Transmit_Request                     0x10
#define Explicit_Addressing_ZigBee_Command_Frame    0x11
#define Remote_Command_Request                      0x17
#define Create_Source_Route                         0x21
#define AT_Command_Response                         0x88
#define Modem_Status                                0x8A
#define ZigBee_Transmit_Status                      0x8B
#define ZigBee_Receive_Packet                       0x90
#define ZigBee_Explicit_Rx_Indicator                0x91
#define ZigBee_IO_Data_Sample_Rx_Indicator          0x92
#define XBee_Sensor_Read_Indicator                  0x94
#define Node_Identification_Indicator               0x95
#define Remote_Command_Response                     0x97
#define Over_the_Air_Firmware_Update_Status         0xA0
#define Route_Record_Indicator                      0xA1
#define Many_to_One_Route_Request_Indicator         0xA3

/* if API mode is 2 all characters after the first are escaped if they are one of */
#define XFRAME 0x7E /* Frame Delimiter */
#define XSCAPE 0x7D /* Escape */
#define XON 0x11 /* XON */
#define XOFF 0x13 /* XOFF */
/* to escape the character prefix it with XSCAPE and XOR it with 0x20 */
#endif /* _PDXBEE */
