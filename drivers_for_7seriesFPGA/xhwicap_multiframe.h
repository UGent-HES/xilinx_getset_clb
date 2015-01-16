/******************************************************************************************************
DISCLAIMER NOTICE
*******************************************************************************************************
We are not affiliated, associated, authorized, endorsed by, 
or in any way officially connected with Xilinx, Inc. 
or any of its subsidiaries or its affiliates.  
In no event whatsoever shall Xilinx, Inc. 
or any of its subsidiaries or its affiliates have any warranty or support commitment for this software 
or liability for loss, injury or damage in connection with this software, 
including but not limited to the use or display thereof.
*******************************************************************************************************
*/
/**
* Functions for reading and writing multiple configuration frames.
* Created by Ghent University.
**/

#ifndef XHWICAP_MULTIFRAME_H_
#define XHWICAP_MULTIFRAME_H_


#include <xhwicap.h>
#include "xhwicap_custom.h"

/**
* Read multiple configuration frames.
* Based on the Xilinx source code to read one configuration frame.
**/
int XHwIcap_DeviceReadFrames(XHwIcap *InstancePtr, long Top, long Block,
				long HClkRow, long MajorFrame, long MinorFrame, u8 NumFrames,
				u32 *FrameBuffer);

/**
* Write multiple configuration frames.
* Based on the Xilinx source code to write one configuration frame.
**/
int XHwIcap_DeviceWriteFrames(XHwIcap *InstancePtr, long Top, long Block,
				long HClkRow, long MajorFrame, long MinorFrame, u8 NumFrames,
				u32 *FrameData);


#endif
