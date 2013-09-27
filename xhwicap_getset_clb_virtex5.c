/**
* Replacement for the XHwIcap_GetClbBits and XHwIcap_SetClbBits functions for the Virtex 5.
**/

#ifndef XHWICAP_GETSET_CLB_H_
#define XHWICAP_GETSET_CLB_H_

#include "xhwicap_getset_clb_virtex5.h"
#include "xhwicap_multiframe.h"
#include <assert.h>


void XHwIcap_Custom_GetConfigurationCoordinates(XHwIcap *InstancePtr, long clb_row, long clb_col,
        u8 *bottom_ntop_p, int *clock_row_p, u32 *major_frame_address_p, u32 *word_offset_p) {
    u32 num_clock_rows = InstancePtr->HClkRows;
    u32 num_clb_rows = InstancePtr->Rows;
    u32 num_clb_rows_per_clock_row = num_clb_rows / num_clock_rows;
    u32 num_clb_cols = InstancePtr->Cols;

    assert(clb_row >= 0 && clb_row < num_clb_rows);
    assert(clb_col >= 0 && clb_col < num_clb_cols);
    
    u8 bottom_ntop;
    int clock_row = (clb_row - 1) / num_clb_rows_per_clock_row;
    if(clock_row > num_clock_rows / 2) {
        bottom_ntop = 1;
        clock_row -= num_clock_rows / 2;
    } else {
        bottom_ntop = 0;
        clock_row = num_clock_rows / 2 - clock_row - 1;
    }
   
    u32 major_frame_address = clb_col;
    u32 i;
    for (i = 0; major_frame_address > InstancePtr->SkipCols[i] ; i++);
    major_frame_address += i;
    
    u32 words_per_clb_row = InstancePtr->WordsPerFrame / num_clb_rows_per_clock_row;
    u32 word_offset = num_clb_rows_per_clock_row - 1 - (clb_row - 1) % num_clb_rows_per_clock_row;
    word_offset *= words_per_clb_row;
    if(word_offset >= InstancePtr->WordsPerFrame / 2)
    	word_offset++;
    	
    *bottom_ntop_p = bottom_ntop;
    *clock_row_p = clock_row;
    *major_frame_address_p = major_frame_address;
    *word_offset_p = word_offset;
}

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
		const u8 Resource[][2], const u8 Value[], long NumBits) {
    u8 bottom_ntop;
    int clock_row;
    u32 major_frame_address;
    u32 word_offset;
    XHwIcap_Custom_GetConfigurationCoordinates(InstancePtr, clb_row, clb_col,
        &bottom_ntop, &clock_row, &major_frame_address, &word_offset);
    
    u32 frame_num = 4;
    u32 frame_number_offset;
    if(Resource[0][1] < 32)
        frame_number_offset = 26;
    else
        frame_number_offset = 32;

	u32 buffer[InstancePtr->WordsPerFrame * (frame_num + 1) + 1];
	int Status = XHwIcap_DeviceReadFrames(InstancePtr, bottom_ntop, XHI_FAR_CLB_BLOCK,
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
        u8 word_nr = frame_number_relative * InstancePtr->WordsPerFrame + word_offset;
        if(bit_nr >= 32) {
        	word_nr++;
            bit_nr -= 32;
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
    
    Status = XHwIcap_DeviceWriteFrames(InstancePtr, bottom_ntop, XHI_FAR_CLB_BLOCK,
				clock_row, major_frame_address, frame_number_offset, frame_num, buffer);
    if (Status != XST_SUCCESS)
        return XST_FAILURE;

	return XST_SUCCESS;
}


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
        const u8 Resource[][2], u8 Value[], long NumBits) {
    u8 bottom_ntop;
    int clock_row;
    u32 major_frame_address;
    u32 word_offset;
    XHwIcap_Custom_GetConfigurationCoordinates(InstancePtr, clb_row, clb_col,
        &bottom_ntop, &clock_row, &major_frame_address, &word_offset);
    
    u32 frame_num = 4;
    u32 frame_number_offset;
    if(Resource[0][1] < 32)
        frame_number_offset = 26;
    else
        frame_number_offset = 32;
        
	u32 buffer[InstancePtr->WordsPerFrame * (frame_num + 1) + 1];
	int Status = XHwIcap_DeviceReadFrames(InstancePtr, bottom_ntop, XHI_FAR_CLB_BLOCK,
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
        u8 word_nr = frame_number_relative * InstancePtr->WordsPerFrame + word_offset;
        if(bit_nr >= 32) {
        	word_nr++;
            bit_nr -= 32;
        }
        assert(bit_nr < 32);
    	u32 word = configuration[word_nr];
    	u8 bit = word >> bit_nr & 1;
    	Value[i] = bit;
    }
	return XST_SUCCESS;
}

#endif
