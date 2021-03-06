#ifndef MOTIONCOMPENSATOR_H
#define MOTIONCOMPENSATOR_H

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "Frame.h"
#include <stdio.h>
#include <fstream>

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

class MotionCompensator
{
public:
	MotionCompensator(int search_width, int search_height);
	~MotionCompensator();

	void setReferenceFrame(Frame* frame);
	Frame* getReferenceFrame();
	void motionCompensate(Macroblock* mb);

protected:
	Frame* reference_frame;
	int ref_width, ref_height;

	int search_width, search_height;

	pixel getRefPixelLuma(int x, int y);
	pixel getRefPixelCb(int x, int y);
	pixel getRefPixelCr(int x, int y);

	Plane search_buffer;

private:
	std::ofstream out;
	const static int _i[], _j[], _i_max[], _j_max[];
	
	long getSSE(Macroblock* mb, int mv_x, int mv_y, int part);
	long fastSearch8x8(Macroblock* mb);
	long fastSearch16x16(Macroblock* mb);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#endif