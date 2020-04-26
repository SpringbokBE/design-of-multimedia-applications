/**
 *  @file    IntraPredictor.cpp
 *  @author  Unknown, Gerbrand De Laender, Damon Verbeyst
 *  @email   gerbrand.delaender@ugent.be, damon.verbeyst@ugent.be
 *  @date    24/04/2020
 *  @version 1.0
 *
 *  @brief   E017920A, Assignment, video encoding
 *
 *  @section DESCRIPTION
 *
 *  Implementation of the intra prediction step.
 *
 *	In this step, redundant information is removed by predicting the content of the current
 *	macroblock based on the edge pixels of the macroblocks to its left, top and top-left.
 *	The prediction is then subtracted from the macroblock, such that only its residue and mode
 *	need to be encoded. Currently, four modes of prediction are supported: DC, vertical,
 *	horizontal and diagonal. The one with the lower Sum of Squared Errors (SSE) w.r.t. the
 *	current macroblock is picked.
 *
 */

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "IntraPredictor.h"

#include <algorithm>

#include "Config.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//
//	Initialise the attributes and allocate storage for both neigbouring pixels as well as for
//	buffering the results of the prediction before the optimal solution is known.
//

IntraPredictor::IntraPredictor()
 : pixel_up_left_luma(0), pixel_up_left_cb(0), pixel_up_left_cr(0), current_frame(nullptr)
{
	pixels_up_luma = new pixel[16], pixels_up_cb = new pixel[8], pixels_up_cr = new pixel[8];
	pixels_left_luma = new pixel[16], pixels_left_cb = new pixel[8], pixels_left_cr = new pixel[8];

	// Initialise the temporary result buffers.
	luma_for_mode = new pixel**[4](), cb_for_mode = new pixel**[4](), cr_for_mode = new pixel**[4]();

	for (int m = 0; m < 4; m++) {
		luma_for_mode[m] = new pixel*[16](), cb_for_mode[m] = new pixel*[8](), cr_for_mode[m] = new pixel*[8]();
		
		for (int i = 0; i < 16; i++) {
			luma_for_mode[m][i] = new pixel[16]();
			if (i % 2) continue; // Cb and Cr are subsampled by a factor of 2.
			cb_for_mode[m][i / 2] = new pixel[8](), cr_for_mode[m][i / 2] = new pixel[8]();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//
//	Deallocate the attributes and temporary buffers.
//

IntraPredictor::~IntraPredictor()
{
	delete pixels_up_luma, delete pixels_up_cb, delete pixels_up_cr;
	delete pixels_left_luma, delete pixels_left_cb, delete pixels_left_cr;

	// Temporary result buffers.
	for (int m = 0; m < 4; m++)
	{
		for (int i = 0; i < 16; i++) {
			delete[] luma_for_mode[m][i];
			if (i % 2) continue; // Cb and Cr are subsampled by a factor of 2.
			delete[] cb_for_mode[m][i / 2], delete[] cr_for_mode[m][i / 2];
		}
		delete[] luma_for_mode[m], delete[] cb_for_mode[m], delete[] cr_for_mode[m];
	}
	delete[] luma_for_mode, delete[] cb_for_mode, delete[] cr_for_mode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//
//	Sets the current frame.
//

void IntraPredictor::setCurrentFrame(Frame* frame)
{
	current_frame = frame;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//
//	Applies the intra prediction to the given macroblock. The best prediction is the one with the
//	lowest SSE.
//

int IntraPredictor::predictIntra(int current_mb, int width, int height)
{
	Macroblock* mb = current_frame->getMacroblock(current_mb);
	mb->state = YUV;
	initialiseEdgePixels(current_mb, width, height, 128);
	
	// The four modes are calculated, the one with the lowest SSE is picked.
	// Intermediate results are stored such that there is no need to recalculate the optimal solution.
	predictDC(luma_for_mode[0], cb_for_mode[0], cr_for_mode[0]);
	predictVertical(luma_for_mode[1], cb_for_mode[1], cr_for_mode[1]);
	predictHorizontal(luma_for_mode[2], cb_for_mode[2], cr_for_mode[2]);
	predictDiagonal(luma_for_mode[3], cb_for_mode[3], cr_for_mode[3]);

	// Calculate the SSE and use the mode with the lowest error.
	long sse_for_mode[4] = {};
	for (int m = 0; m < 4; m++) {
		for (int i = 0; i < 16; i++) {
			for (int j = 0; j < 16; j++) {
				sse_for_mode[m] += CALC_DIFF(mb->luma[i][j], luma_for_mode[m][i][j]);
			}
		}
	}

	int best_mode = std::distance(sse_for_mode, std::min_element(sse_for_mode, sse_for_mode + 4));
	
	// Subtract the prediction and leave behind the residual.
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			mb->luma[i][j] -= luma_for_mode[best_mode][i][j];
			if (i % 2 || j % 2) continue; // Cb and Cr are subsampled by a factor of 2.
			mb->cb[i / 2][j / 2] -= cb_for_mode[best_mode][i / 2][j / 2];
			mb->cr[i / 2][j / 2] -= cr_for_mode[best_mode][i / 2][j / 2];
		}
	}

	return best_mode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//
//	Copies the neighbouring pixels of the neightbouring macroblocks on which the prediction
//	will be based.
//

void IntraPredictor::initialiseEdgePixels(int current_mb, int width, int height, pixel default_value)
{
	// This macroblock is at the leftmost edge of the image.
	if (current_mb % width == 0) {
		for (int i = 0; i < 16; i++) {
			this->pixels_left_luma[i] = default_value;
			if (i % 2) continue; // Cb and Cr are subsampled by a factor of 2.
			this->pixels_left_cb[i / 2] = this->pixels_left_cr[i / 2] = default_value;
		}
		this->pixel_up_left_luma = this->pixel_up_left_cb = this->pixel_up_left_cr = default_value;
	}

	// This macroblock is not at the leftmost edge of the image.
	else {
		Macroblock* left = current_frame->getMacroblock(current_mb - 1);
		for (int i = 0; i < 16; i++) {
			this->pixels_left_luma[i] = left->luma[i][15];
			if (i % 2) continue; // Cb and Cr are subsampled by a factor of 2.
			this->pixels_left_cb[i / 2] = left->cb[i / 2][7];
			this->pixels_left_cr[i / 2] = left->cr[i / 2][7];
		}
	}

	// This macroblock is at the topmost edge of the image.
	if (current_mb < width) {
		for (int i = 0; i < 16; i++) {
			this->pixels_up_luma[i] = default_value;
			if (i % 2) continue; // Cb and Cr are subsampled by a factor of 2.
			this->pixels_up_cb[i / 2] = this->pixels_up_cr[i / 2] = default_value;
		}
		this->pixel_up_left_luma = this->pixel_up_left_cb = this->pixel_up_left_cr = default_value;
	}

	// This macroblock is not at the topmost edge of the image.
	else {
		Macroblock* up = current_frame->getMacroblock(current_mb - width);
		for (int i = 0; i < 16; i++) {
			this->pixels_up_luma[i] = up->luma[15][i];
			if (i % 2) continue; // Cb and Cr are subsampled by a factor of 2.
			this->pixels_up_cb[i / 2] = up->cb[7][i / 2];
			this->pixels_up_cr[i / 2] = up->cr[7][i / 2];
		}
	}

	// This macroblock is not in the top-left corner of the image.
	if (current_mb > width && current_mb % width) {
		Macroblock* up_left = current_frame->getMacroblock(current_mb - width - 1);
		this->pixel_up_left_luma = up_left->luma[15][15];
		this->pixel_up_left_cb = up_left->cb[7][7];
		this->pixel_up_left_cr = up_left->cr[7][7];
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//
//	Do the DC prediction, in which the block is filled with the average of the left/top neighbours.
//

void IntraPredictor::predictDC(pixel** luma, pixel** cb, pixel** cr)
{
	double luma_avg = 0, cb_avg = 0, cr_avg = 0;

	for (int i = 0; i < 16; i++) {
		luma_avg += this->pixels_left_luma[i];
		luma_avg += this->pixels_up_luma[i];
		if (i % 2) continue; // Cb and Cr are subsampled by a factor of 2.
		cb_avg += this->pixels_left_cb[i / 2];
		cb_avg += this->pixels_up_cb[i / 2];
		cr_avg += this->pixels_left_cr[i / 2];
		cr_avg += this->pixels_up_cr[i / 2];
	}

	luma_avg = static_cast<pixel>(round(luma_avg / 32));
	cb_avg = static_cast<pixel>(round(cb_avg / 16));
	cr_avg = static_cast<pixel>(round(cr_avg / 16));

	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			luma[i][j] = luma_avg;
			if (i % 2 || j % 2) continue; // Cb and Cr are subsampled by a factor of 2.
			cb[i / 2][j / 2] = cb_avg;
			cr[i / 2][j / 2] = cr_avg;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//
//	Do the vertical prediction, in which the block is filled by copying the top neighbours along
//	the y-axis.
//

void IntraPredictor::predictVertical(pixel** luma, pixel** cb, pixel** cr)
{
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			luma[i][j] = this->pixels_up_luma[j];
			if (i % 2 || j % 2) continue; // Cb and Cr are subsampled by a factor of 2.
			cb[i / 2][j / 2] = this->pixels_up_cb[j / 2];
			cr[i / 2][j / 2] = this->pixels_up_cr[j / 2];
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//
//	Do the horizontal prediction, in which the block is filled by copying the left neighbours along
//	the x-axis.
//

void IntraPredictor::predictHorizontal(pixel** luma, pixel** cb, pixel** cr)
{
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			luma[i][j] = this->pixels_left_luma[i];
			if (i % 2 || j % 2) continue; // Cb and Cr are subsampled by a factor of 2.
			cb[i / 2][j / 2] = this->pixels_left_cb[i / 2];
			cr[i / 2][j / 2] = this->pixels_left_cr[i / 2];
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//
//	Do the diagonal prediction, in which the block is filled by extrapolating the neighbouring
//	pixels diagonally from top-left to bottom-right. The first row and column need special attention
//	because of the specialised weighting. The rest is (diagonally) copied from the first row/column.
//

void IntraPredictor::predictDiagonal(pixel** luma, pixel** cb, pixel** cr)
{
	// Fill the upper 3 pixels for all three channels.
	luma[0][0] = static_cast<pixel>(round((this->pixels_left_luma[0] + 2.0 * this->pixel_up_left_luma + this->pixels_up_luma[0]) / 4.0));
	luma[1][0] = static_cast<pixel>(round((this->pixel_up_left_luma + 2.0 * this->pixels_left_luma[0] + this->pixels_left_luma[1]) / 4.0));
	luma[0][1] = static_cast<pixel>(round((this->pixel_up_left_luma + 2.0 * this->pixels_up_luma[0] + this->pixels_up_luma[1]) / 4.0));

	cb[0][0] = static_cast<pixel>(round((this->pixels_left_cb[0] + 2.0 * this->pixel_up_left_cb + this->pixels_up_cb[0]) / 4.0));
	cb[1][0] = static_cast<pixel>(round((this->pixel_up_left_cb + 2.0 * this->pixels_left_cb[0] + this->pixels_left_cb[1]) / 4.0));
	cb[0][1] = static_cast<pixel>(round((this->pixel_up_left_cb + 2.0 * this->pixels_up_cb[0] + this->pixels_up_cb[1]) / 4.0));

	cr[0][0] = static_cast<pixel>(round((this->pixels_left_cr[0] + 2.0 * this->pixel_up_left_cr + this->pixels_up_cr[0]) / 4.0));
	cr[1][0] = static_cast<pixel>(round((this->pixel_up_left_cr + 2.0 * this->pixels_left_cr[0] + this->pixels_left_cr[1]) / 4.0));
	cr[0][1] = static_cast<pixel>(round((this->pixel_up_left_cr + 2.0 * this->pixels_up_cr[0] + this->pixels_up_cr[1]) / 4.0));

	// Fill the rest of the first column / row for all three channels.
	for (int i = 2; i < 16; i++) {
		luma[i][0] = static_cast<pixel>(round((this->pixels_left_luma[i - 2] + 2.0 * this->pixels_left_luma[i - 1] + this->pixels_left_luma[i]) / 4.0));
		luma[0][i] = static_cast<pixel>(round((this->pixels_up_luma[i - 2] + 2.0 * this->pixels_up_luma[i - 1] + this->pixels_up_luma[i]) / 4.0));
	}
	for (int i = 2; i < 8; i++) {
		cb[i][0] = static_cast<pixel>(round((this->pixels_left_cb[i - 2] + 2.0 * this->pixels_left_cb[i - 1] + this->pixels_left_cb[i]) / 4.0));
		cr[i][0] = static_cast<pixel>(round((this->pixels_left_cr[i - 2] + 2.0 * this->pixels_left_cr[i - 1] + this->pixels_left_cr[i]) / 4.0));
		cb[0][i] = static_cast<pixel>(round((this->pixels_up_cb[i - 2] + 2.0 * this->pixels_up_cb[i - 1] + this->pixels_up_cb[i]) / 4.0));
		cr[0][i] = static_cast<pixel>(round((this->pixels_up_cr[i - 2] + 2.0 * this->pixels_up_cr[i - 1] + this->pixels_up_cr[i]) / 4.0));
	}

	// Fill the remaining 15x15 pixels (Y) or 7x7 pixels (Cb, Cr).
	for (int i = 1; i < 16; i++) {
		for (int j = 1; j < 16; j++) {
			luma[i][j] = luma[i - 1][j - 1];
			if (i % 2 || j % 2) continue; // Cb and Cr are subsampled by a factor of 2.
			cb[i / 2][j / 2] = cb[i / 2 - 1][j / 2 - 1];
			cr[i / 2][j / 2] = cr[i / 2 - 1][j / 2 - 1];
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////