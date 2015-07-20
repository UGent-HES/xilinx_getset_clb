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
* Replacement for the XHwIcap_GetClbBits and XHwIcap_SetClbBits functions for the Zynq.
* Created by Ghent University.
**/

#ifndef XHWICAP_GETSET_CLB_7_SERIES_H
#define XHWICAP_GETSET_CLB_7_SERIES_H

#include <xhwicap.h>
#include "xhwicap_custom.h"

#if (XHI_FAMILY != XHI_DEV_FAMILY_7SERIES)
    #error You are using the wrong xhwicap_getset_clb driver files. This file is specific to 7 series FPGAs only!!
#endif

typedef struct {
	long slice_row;
	long slice_col;
	const u8 (*Resource)[2];
	const u8 *Value;
	long NumBits;
} LUT_config_type;


/**
* Returns true if the configuration of two slices are stored in the same set of frames.
**/
u8 XHwIcap_Custom_IsSameFrame(XHwIcap *InstancePtr, long slice_row0, long slice_col0, long slice_row1, long slice_col1);

/**
* Sets bits contained in multiple LUTs specified by the coordinates and data in the lut_configs array.
* For each LUT the slice_row, slice_col, Resource pointer, Value pointer and NumBits are stored cfr. the conventional XHwIcap_Custom_SetClbBits function.
* All the LUTs reconfigured with one call to XHwIcap_Custom_SetMultiClbBits should be stored in the same set of frames!
* This can be verified using the function XHwIcap_Custom_IsSameFrame.
*
* @return	XST_SUCCESS or XST_FAILURE.
**/
int XHwIcap_Custom_SetMultiClbBits(XHwIcap *InstancePtr, LUT_config_type  *lut_configs, u32 num_lut_configs);

/**
* Sets bits contained in a LUT specified by the CLB row and col
* coordinates.  The coordinate system lables the upper left CLB as (1,1).
*
* @param	InstancePtr is a pointer to the XHwIcap instance to be worked on.
* @param	Row is the CLB row. (1,1) is the upper left CLB.
* @param	Col is the CLB col. (1,1) is the upper left CLB.
* @param	Resource is the target bits data from XHI_CLB_LUT_replacement.
* @param	Value is the values to set each of the targets bits to.
*		The size of this array must be equal to NumBits.
* @param	NumBits is the number of Bits to change in this method.
*
* @return	XST_SUCCESS or XST_FAILURE.
**/
int XHwIcap_Custom_SetClbBits(XHwIcap *InstancePtr, long clb_row, long clb_col,
		const u8 Resource[][2], const u8 Value[], long NumBits);


/**
* Get bits contained in a LUT specified by the CLB row and col
* coordinates.  The coordinate system lables the upper left CLB as (1,1).
*
* @param	InstancePtr is a pointer to the XHwIcap instance to be worked on.
* @param	Row is the CLB row. (1,1) is the upper left CLB.
* @param	Col is the CLB col. (1,1) is the upper left CLB.
* @param	Resource is the target bits data from XHI_CLB_LUT_replacement.
* @param	Value is the values to set each of the targets bits to.
*		The size of this array must be equal to NumBits.
* @param	NumBits is the number of Bits to read in this method.
*
* @return	XST_SUCCESS or XST_FAILURE.
**/
int XHwIcap_Custom_GetClbBits(XHwIcap *InstancePtr, long clb_row, long clb_col,
        const u8 Resource[][2], u8 Value[], long NumBits);

#endif
