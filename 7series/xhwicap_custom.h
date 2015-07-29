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
*Created by Ghent University.
*/
#ifndef XHWICAP_CUSTOM_H_ /* prevent circular inclusions */
#define XHWICAP_CUSTOM_H_ /* by using protection macros */

#include "xhwicap_i.h"
#include "xhwicap_custom_i.h"
#include "xhwicap_getset_clb_7series.h"

#if (XHI_FAMILY != XHI_DEV_FAMILY_7SERIES)
    #error You are using the wrong xhwicap_custom driver files. This file is specific to 7 series FPGAs only!!
#endif

/* Custom function for initializing the HWICAP, customized for Zynq and Kintex 7 fabric*/
int XHwIcap_custom_CfgInitialize(XHwIcap *InstancePtr, XHwIcap_Config *ConfigPtr, u32 EffectiveAddr);

#endif
