#ifndef INTRAPREDICTOR_H
#define INTRAPREDICTOR_H

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "Frame.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

class IntraPredictor
{
	public:
		IntraPredictor();
		~IntraPredictor();

		void setCurrentFrame(Frame* frame);
		int predictIntra(int current_mb, int width, int height); // Width and height in number of macroblocks.

	protected:
		Frame* current_frame;
		
		pixel* pixels_up_luma, * pixels_up_cb, * pixels_up_cr;
		pixel* pixels_left_luma, * pixels_left_cb, * pixels_left_cr;
		pixel	pixel_up_left_luma, pixel_up_left_cb, pixel_up_left_cr;

	private:
		// Buffers to keep the results of all intra prediction modes, such that the best mode doesn't need
		// to be recalculated once the SSE results are complete.
		pixel*** luma_for_mode, *** cb_for_mode, *** cr_for_mode;
		
		void initialiseEdgePixels(int current_mb, int width, int height, pixel default_value);
		void predictDC(pixel** luma, pixel** cb, pixel** cr);
		void predictVertical(pixel** luma, pixel** cb, pixel** cr);
		void predictHorizontal(pixel** luma, pixel** cb, pixel** cr);
		void predictDiagonal(pixel** luma, pixel** cb, pixel** cr);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#endif