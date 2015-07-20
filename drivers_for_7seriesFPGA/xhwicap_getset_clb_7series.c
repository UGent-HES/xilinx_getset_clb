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
* Replacement for the XHwIcap_GetClbBits and XHwIcap_SetClbBits functions for the 7 series FPGA.
* Created by Ghent University.
**/

#ifndef XHWICAP_GETSET_CLB_H_
#define XHWICAP_GETSET_CLB_H_

#include <assert.h>
#include "xhwicap_getset_clb_7series.h"
#include "xhwicap_multiframe.h"
#include "xhwicap.h"
#include "xhwicap_custom.h"


/**
* Returns the word offset of the configuration of a CLB within a frame.
**/
u32 XHwIcap_Custom_GetWordOffset(XHwIcap *InstancePtr, long slice_row) {
    u32 num_clock_rows = InstancePtr->HClkRows;
    u32 num_clb_rows = InstancePtr->Rows;
    u32 num_clb_rows_per_clock_row = num_clb_rows / num_clock_rows;

    assert(slice_row >= 0 && slice_row < num_clb_rows);

  	/* Calculating word offset */
    u32 word_offset = 0;
	slice_row = slice_row % num_clb_rows_per_clock_row;

	if (slice_row >= num_clb_rows_per_clock_row/2){
		word_offset = slice_row * 2 + 1;
	}
	else {
		word_offset = slice_row * 2;
	}
	return word_offset;
}

/**
* Returns true if the configuration of two slices are stored in the same set of frames.
**/
u8 XHwIcap_Custom_IsSameFrame(XHwIcap *InstancePtr, long slice_row0, long slice_col0, long slice_row1, long slice_col1) {
    u32 num_clock_rows = InstancePtr->HClkRows;
    u32 num_clb_rows = InstancePtr->Rows;
    u32 num_clb_rows_per_clock_row = num_clb_rows / num_clock_rows;

    //u32 major_frame_address0 = slice_col0/2;
    //u32 major_frame_address1 = slice_col1/2;

    u8 slice_grp0 = slice_row0 / num_clb_rows_per_clock_row + 1;
    u8 slice_grp1 = slice_row1 / num_clb_rows_per_clock_row + 1;

    return (slice_col0 == slice_col1) && (slice_grp0 == slice_grp1);
}

/**
*Prepares the configuration of the Frame address for a given slice co-ordinates.
*
* @param	InstancePtr is a pointer to the XHwIcap instance to be worked on.
* @param	slice_row is row number of the Slice (Y co-ordinate).
* @param	slice_col is column number of the Slice (X co-ordinate).
* @param	*bottom_ntop_p pointer to a variable where top/bottom bit is to be stored.
* @param	*clock_row_p pointer to a variable where row address is to be stored.
* @param	*major_frame_address_p pointer to a variable where CLB column/major address is to be stored.
*
**/
void XHwIcap_Custom_GetConfigurationCoordinates(XHwIcap *InstancePtr, long slice_row, long slice_col,
        u8 *bottom_ntop_p, int *clock_row_p, u32 *major_frame_address_p) {

    u32 num_clock_rows = InstancePtr->HClkRows;
    u32 num_clb_rows = InstancePtr->Rows;
    u32 num_clb_rows_per_clock_row = num_clb_rows / num_clock_rows;
    u32 num_clb_cols = InstancePtr->Cols;

    assert(slice_row >= 0 && slice_row < num_clb_rows);
    assert(slice_col >= 0 && slice_col < 2*num_clb_cols);

	/* Translation of Slice X-coordinate to CLB X-coordinate */
    u32 major_frame_address = slice_col/2;
    u32 i;
    for (i = 0; major_frame_address >= InstancePtr->SkipCols[i] ; i++);
    major_frame_address += i;

	/* Top or Bottom bit and the row address*/
    u8 mid_line;
    u8 bottom_ntop;
    u8 slice_grp = slice_row / num_clb_rows_per_clock_row + 1;
    int clock_row = 0;

    if(num_clock_rows % 2 == 0)
#if (XHI_FPGA_FAMILY == XHI_DEV_FAMILY_ZYNQ)
    	mid_line = num_clock_rows / 2;
#elif (XHI_FPGA_FAMILY == XHI_DEV_FAMILY_K7)
		mid_line = num_clock_rows / 2 + 1;
#endif
    else
#if (XHI_FPGA_FAMILY == XHI_DEV_FAMILY_ZYNQ)
    	mid_line = num_clock_rows / 2 + 1;
#elif (XHI_FPGA_FAMILY == XHI_DEV_FAMILY_K7)
		mid_line = num_clock_rows / 2;
#endif

	if(slice_grp > mid_line){
		bottom_ntop = 0;
		clock_row = slice_grp - mid_line - 1;
	}
	else {
		clock_row = mid_line - slice_grp;
		bottom_ntop = 1;
	}

    *bottom_ntop_p = bottom_ntop;
    *clock_row_p = clock_row;
    *major_frame_address_p = major_frame_address;
}

void XHwIcap_Custom_SetClbBitsInConfig(XHwIcap *InstancePtr, u32 *configuration, u32 word_offset, u32 frame_num, u32 frame_number_offset, const u8 Resource[][2], const u8 Value[], long NumBits) {
	u32 i;
	for(i = 0; i < NumBits; i++) {
		u32 frame_number = Resource[i][1];
		u32 frame_number_relative = frame_number - frame_number_offset;
		assert(frame_number >= frame_number_offset && frame_number_relative < frame_num);

		u8 bit_nr = Resource[i][0];
		u16 word_nr = frame_number_relative * InstancePtr->WordsPerFrame + word_offset;

		if(bit_nr >= 32) {
			bit_nr -= 32;
		} else {
			word_nr++;
		}

		assert(bit_nr < 32);
		u32 word = configuration[word_nr];
		u32 bit = 1 << bit_nr;
		if(Value[i])
			word |= bit;
		else
			word &= ~bit;
		configuration[word_nr] = word;
	}
}

/**
* Sets bits contained in multiple LUTs specified by the coordinates and data in the lut_configs array.
* For each LUT the slice_row, slice_col, Resource pointer, Value pointer and NumBits are stored cfr. the conventional XHwIcap_Custom_SetClbBits function.
* All the LUTs reconfigured with one call to XHwIcap_Custom_SetMultiClbBits should be stored in the same set of frames!
* This can be verified using the function XHwIcap_Custom_IsSameFrame.
*
* @return	XST_SUCCESS or XST_FAILURE.
**/
int XHwIcap_Custom_SetMultiClbBits(XHwIcap *InstancePtr, LUT_config_type  *lut_configs, u32 num_lut_configs) {
	u8 bottom_ntop;
	int Status;
	int clock_row;
	u32 major_frame_address;

	assert(num_lut_configs>0);

	long slice_row = lut_configs[0].slice_row;
	long slice_col = lut_configs[0].slice_col;

	// Check if all the lutconfigs are indeed part of the same set of frames
	u32 i;
	for(i = 1; i<num_lut_configs; i++) {
		if(!XHwIcap_Custom_IsSameFrame(InstancePtr, slice_row, slice_col, lut_configs[i].slice_row, lut_configs[i].slice_col))
			return XST_FAILURE;
	}

	XHwIcap_Custom_GetConfigurationCoordinates(InstancePtr, slice_row, slice_col,
		&bottom_ntop, &clock_row, &major_frame_address);

	u32 frame_num = 4;
	u32 frame_number_offset;
	if(lut_configs[0].Resource[0][1] < 32)
		frame_number_offset = 26;
	else
		frame_number_offset = 32;

	u32 buffer[InstancePtr->WordsPerFrame * (frame_num + 1) + 1];

	Status = XHwIcap_DeviceReadFrames(InstancePtr, bottom_ntop, XHI_FAR_CLB_BLOCK,
			clock_row, major_frame_address, frame_number_offset, frame_num, buffer);

	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	u32 *configuration = buffer + InstancePtr->WordsPerFrame + 1;

	for(i = 0; i< num_lut_configs; i++) {
		u32 word_offset = XHwIcap_Custom_GetWordOffset(InstancePtr, lut_configs[i].slice_row);
	    XHwIcap_Custom_SetClbBitsInConfig(InstancePtr, configuration, word_offset,
	    		frame_num, frame_number_offset, lut_configs[i].Resource, lut_configs[i].Value, lut_configs[i].NumBits);
	}

	Status = XHwIcap_DeviceWriteFrames(InstancePtr, bottom_ntop, XHI_FAR_CLB_BLOCK,
				clock_row, major_frame_address, frame_number_offset, frame_num, buffer);

	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	return XST_SUCCESS;
}

/**
* Sets bits contained in a LUT specified by the Slice row and col
* coordinates.
*
* @param	InstancePtr is a pointer to the XHwIcap instance to be worked on.
* @param	slice_row is row number of the Slice (Y co-ordinate).
* @param	slice_col is column number of the Slice (X co-ordinate).
* @param	Resource is the target bits data from XHI_CLB_LUT_replacement.
* @param	Value is the values to set each of the targets bits to.
*			The size of this array must be equal to NumBits.
* @param	NumBits is the number of Bits to change in this method.
*
* @return	XST_SUCCESS or XST_FAILURE.
**/
int XHwIcap_Custom_SetClbBits(XHwIcap *InstancePtr, long slice_row, long slice_col,
		const u8 Resource[][2], const u8 Value[], long NumBits) {
    u8 bottom_ntop;
    int Status;
    int clock_row;
    u32 major_frame_address;
    u32 word_offset = XHwIcap_Custom_GetWordOffset(InstancePtr, slice_row);

    XHwIcap_Custom_GetConfigurationCoordinates(InstancePtr, slice_row, slice_col,
        &bottom_ntop, &clock_row, &major_frame_address);
    
    u32 frame_num = 4;
    u32 frame_number_offset;
    if(Resource[0][1] < 32)
        frame_number_offset = 26;
    else
        frame_number_offset = 32;

	u32 buffer[InstancePtr->WordsPerFrame * (frame_num + 1) + 1];

	Status = XHwIcap_DeviceReadFrames(InstancePtr, bottom_ntop, XHI_FAR_CLB_BLOCK,
			clock_row, major_frame_address, frame_number_offset, frame_num, buffer);

    if (Status != XST_SUCCESS)
        return XST_FAILURE;

    u32 *configuration = buffer + InstancePtr->WordsPerFrame + 1;

    XHwIcap_Custom_SetClbBitsInConfig(InstancePtr, configuration, word_offset,
    		frame_num, frame_number_offset, Resource, Value, NumBits);

    Status = XHwIcap_DeviceWriteFrames(InstancePtr, bottom_ntop, XHI_FAR_CLB_BLOCK,
				clock_row, major_frame_address, frame_number_offset, frame_num, buffer);

    if (Status != XST_SUCCESS)
        return XST_FAILURE;

	return XST_SUCCESS;
}


/**
* Get bits contained in a LUT specified by the Slice row and col
* coordinates.
*
* @param	InstancePtr is a pointer to the XHwIcap instance to be worked on.
* @param	slice_row is row number of the Slice (Y co-ordinate).
* @param	slice_col is column number of the Slice (X co-ordinate).
* @param	Resource is the target bits data from XHI_CLB_LUT_replacement.
* @param	Value is the values that are read each of the targets bits to.
*		    The size of this array must be equal to NumBits.
* @param	NumBits is the number of Bits to read in this method.
*
* @return	XST_SUCCESS or XST_FAILURE.
**/
int XHwIcap_Custom_GetClbBits(XHwIcap *InstancePtr, long slice_row, long slice_col,
        const u8 Resource[][2], u8 Value[], long NumBits) {
    u8 bottom_ntop;
    int Status;
    int clock_row;
    u32 major_frame_address;
    u32 word_offset = XHwIcap_Custom_GetWordOffset(InstancePtr, slice_row);

    XHwIcap_Custom_GetConfigurationCoordinates(InstancePtr, slice_row, slice_col,
        &bottom_ntop, &clock_row, &major_frame_address);
    
    u32 frame_num = 4;
    u32 frame_number_offset;
    if(Resource[0][1] < 32)
        frame_number_offset = 26;
    else
        frame_number_offset = 32;
        
	u32 buffer[InstancePtr->WordsPerFrame * (frame_num + 1) + 1];

	Status = XHwIcap_DeviceReadFrames(InstancePtr, bottom_ntop, XHI_FAR_CLB_BLOCK,
				clock_row, major_frame_address, frame_number_offset, frame_num, buffer);

	if (Status != XST_SUCCESS)
        return XST_FAILURE;

    u32 *configuration = buffer + InstancePtr->WordsPerFrame + 1;

    u32 i;
    for(i = 0; i < NumBits; i++) {
    	u32 frame_number = Resource[i][1];
    	u32 frame_number_relative = frame_number - frame_number_offset;
    	assert(frame_number >= frame_number_offset && frame_number_relative < frame_num);

        u8 bit_nr = Resource[i][0];
        u16 word_nr = frame_number_relative * InstancePtr->WordsPerFrame + word_offset;

        if(bit_nr >= 32) {
            bit_nr -= 32;
        }else {
        	word_nr++;
        }
        assert(bit_nr < 32);
    	u32 word = configuration[word_nr];
    	u8 bit = word >> bit_nr & 1;
    	Value[i] = bit;
    }
	return XST_SUCCESS;
}

#endif
