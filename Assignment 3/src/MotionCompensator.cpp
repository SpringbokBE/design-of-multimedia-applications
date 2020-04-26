/**
 *  @file    MotionCompensator.cpp
 *  @author  Unknown, Gerbrand De Laender, Damon Verbeyst
 *  @email   gerbrand.delaender@ugent.be, damon.verbeyst@ugent.be
 *  @date    24/04/2020
 *  @version 1.0
 *
 *  @brief   E017920A, Assignment, video encoding
 *
 *  @section DESCRIPTION
 *
 *  Implementation of the motion compensation step.
 *
 *	In this step, macroblocks of an I-frame are matched against parts of the reference P-frame.
 *	A suboptimal match is found using a fast search algorithm, which aims at reducing the intensive
 *	process of exhaustively searching the P-frame for the best match. The best match is
 *	characterised by the lowest Sum of Squared Errors (SSE). The vectors indicating the relative
 *	position of the best match are called the motion vectors. Instead of encoding the full block,
 *	now only the motion vector and the residual signal have to be encoded, yielding better
 *	compression rates.
 *
 *	A fast search is performed on the macroblock level (using 16x16 blocks), however a basic form
 *	of partitioning is also supported. In the latter case, a macroblock is split into 4 8x8 blocks,
 *	which are separately matched against the reference P-frame. If the cost to send the four
 *	partitions (including the increased number of motion vectors themselves) is lower than that of
 *	the unpartitioned version, the former is used. The cost factor can be empirically determined.
 *
 */

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "MotionCompensator.h"

#include <algorithm>
#include <iostream>
#include <limits>

#include "Config.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// Used to construct the loop variables and bounds when using partitioning.
const int MotionCompensator::_i[5] = {0, 8, 0, 8, 0};
const int MotionCompensator::_j[5] = {0, 0, 8, 8, 0};
const int MotionCompensator::_i_max[5] = {8, 16, 8, 16, 16};
const int MotionCompensator::_j_max[5] = {8, 8, 16, 16, 16};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//
//	Allocates a search buffer containing all pixel values within the search window, specified by
//	search_width and search_height.
//

MotionCompensator::MotionCompensator(int search_width, int search_height)
	: search_width(search_width), search_height(search_height), reference_frame(0), ref_height(0), ref_width(0)
{
	search_width += 16;
	search_height += 16;

	search_buffer = new pixel*[search_width];

	for (int i = 0; i < search_width; i++) {
		search_buffer[i] = new pixel[search_height];
	}

	if(MV_TO_CSV) out.open(CSV_NAME, std::ofstream::trunc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//
//	Deallocates the search buffer.
//

MotionCompensator::~MotionCompensator()
{
	for (int i = 0; i < this->search_width + 16; i++) {
		delete[] search_buffer[i];
	}
	delete[] search_buffer;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//
//	Sets the reference frame and the number of macroblocks along the width and height of the frame.
//

void MotionCompensator::setReferenceFrame(Frame *frame)
{
	reference_frame = frame;
	ref_width = frame->getWidth();
	ref_height = frame->getHeight();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//
//	Get the current reference frame.
//

Frame *MotionCompensator::getReferenceFrame()
{
	return reference_frame;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//
//	Apply the motion compensation on the current macroblock. The optimal block is picked based on
//	the lowest SSE, where only the luma channel is considered.
//

void MotionCompensator::motionCompensate(Macroblock* mb)
{
	mb->state = MC;
	int x = 16 * mb->getXPos(), y = 16 * mb->getYPos(); // Coordinates of top-left corner of macroblock within the frame.
	int dx = this->search_width / 2, dy = this->search_height / 2;

	// Fill the search buffer with the luma of the pixels in the search window around the current macroblock.
	for (int i = -dx, ii = 0; i < dx + 16; i++, ii++) {
		for (int j = -dy, jj = 0; j < dy + 16; j++, jj++) {
			search_buffer[ii][jj] = getRefPixelLuma(std::min(std::max(0, x + i), 16 * ref_width - 1), std::min(std::max(0, y + j), 16 * ref_height - 1));
		}
	}

	long min_sse = fastSearch16x16(mb);
	int temp_x = mb->mv[0].x, temp_y = mb->mv[0].y;
	long sum_min_sse = fastSearch8x8(mb);
	
	// Cost is determined empirically.
	if (sum_min_sse + 10000 < min_sse) {
		mb->partitions = true;
	}
	else {
		mb->partitions = false;
		mb->mv[0].x = mb->mv[1].x = mb->mv[2].x = mb->mv[3].x = temp_x;
		mb->mv[0].y = mb->mv[1].y = mb->mv[2].y = mb->mv[3].y = temp_y;
	}

	for (int part = 0; part < 4; part++) {
		int xr = dx + mb->mv[part].x; // Coordinates of top-left corner of optimal solution within the search window. (=relative)
		int yr = dy + mb->mv[part].y;
		int xa = std::min(std::max(0, x + mb->mv[part].x), 16 * ref_width - 16); // Coordinates of top-left corner of optimal solution within the frame. (=absolute)
		int ya = std::min(std::max(0, y + mb->mv[part].y), 16 * ref_height - 16);

		for (int i = _i[part]; i < _i_max[part]; i++) {
			for (int j = _j[part]; j < _j_max[part]; j++) {
				mb->luma[j][i] -= search_buffer[xr + i][yr + j];
				if (i % 2 || j % 2) continue;
				mb->cb[j / 2][i / 2] -= getRefPixelCb(xa + i, ya + j);
				mb->cr[j / 2][i / 2] -= getRefPixelCr(xa + i, ya + j);
			}
		}
	}

	if (MV_TO_CSV) out << mb->mv[0].x << ", " << mb->mv[0].y << "\n";
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//
//	Returns the luma value of the pixel located at coordinates (x, y) in the reference frame.
//

pixel MotionCompensator::getRefPixelLuma(int x, int y)
{
	int mb_num = (y / 16) * ref_width + x / 16;
	int mb_x = x % 16;
	int mb_y = y % 16;

	return reference_frame->getMacroblock(mb_num)->luma[mb_y][mb_x];
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//
//	Returns the Cb value of the pixel located at coordinates (x, y) in the reference frame.
//

pixel MotionCompensator::getRefPixelCb(int x, int y)
{
	int mb_num = (y / 16) * ref_width + x / 16;
	int mb_x = (x % 16) / 2;
	int mb_y = (y % 16) / 2;
	
	return reference_frame->getMacroblock(mb_num)->cb[mb_y][mb_x];
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//
//	Returns the Cr value of the pixel located at coordinates (x, y) in the reference frame.
//

pixel MotionCompensator::getRefPixelCr(int x, int y)
{
	int mb_num = (y / 16) * ref_width + x / 16;
	int mb_x = (x % 16) / 2;
	int mb_y = (y % 16) / 2;

	return reference_frame->getMacroblock(mb_num)->cr[mb_y][mb_x];
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//
//	Returns the Sum of Squared Errors (SSE) of the macroblock shifted by mv_x, mv_y relative to
//	the reference frame. If part < 4, only a 8x8 window will be considered, if part = 4, the full
//	16x16 widow is considered.
//	The partition numbering is the following: [0|1]
//																						[2|3]
//

long MotionCompensator::getSSE(Macroblock* mb, int mv_x, int mv_y, int part = 4)
{
	int x = search_width / 2 + mv_x, y = search_height / 2 + mv_y; // Coordinates of top-left corner of macroblock within the search window. 
	if (x < 0 || x > search_width || y < 0 || y > search_height) return LONG_MAX;
	
	long sse = 0;
	
	for (int i = _i[part]; i < _i_max[part]; i++) {
		for (int j = _j[part]; j < _j_max[part]; j++) {
			sse += CALC_DIFF(mb->luma[j][i], search_buffer[x + i][y + j]);
		}
	}

	return sse;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//
//	Looks for the best possible motion vectors of the given macroblock using partitioning.
//	Applies a fast-search algorithm 4 times on each partition. The 4 resulting (sub)optimal vectors
//	are stored in the macroblock itself as mv[0], mv[1], mv[2] and mv[3].
//

long MotionCompensator::fastSearch8x8(Macroblock* mb)
{
	long sum_min_sse = 0;

	for (int part = 0; part < 4; part++) {
		long min_sse = LONG_MAX;
		int min_mv_x = 0, min_mv_y = 0; // Local equivalent since the macroblock's motion vector is used in the loops.
		mb->mv[part].x = 0, mb->mv[part].y = 0; // Initialise the motion vector to 0.

		for (int it = 0; it < 3; it++) {
			int dx = search_width / (2 << it), dy = search_height / (2 << it);
			int inc_x = (it != 2 ? dx : 1), inc_y = (it != 2 ? dy : 1);
			for (int mv_x = mb->mv[part].x - dx; mv_x <= mb->mv[part].x + dx; mv_x += inc_x) {
				for (int mv_y = mb->mv[part].y - dy; mv_y <= mb->mv[part].y + dy; mv_y += inc_y) {
					if(it && mv_x == mb->mv[part].x && mv_y == mb->mv[part].y) continue; // SSE of center has already been computed in the previous iteration.
					long sse = getSSE(mb, mv_x, mv_y, part);
					if (sse < min_sse) {
						min_sse = sse, min_mv_x = mv_x, min_mv_y = mv_y;
					}
				}
			}

			// Now it is safe to update the macroblock's motion vector with the local values.
			mb->mv[part].x = min_mv_x, mb->mv[part].y = min_mv_y;
		}
		sum_min_sse += min_sse;
	}

	return sum_min_sse;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//
//	Looks for the best possible motion vector of the given macroblock. Applies a fast-search
//	algorithm. Theresulting (sub)optimal vector is stored in the macroblock itself, as mv[0].
//

long MotionCompensator::fastSearch16x16(Macroblock* mb)
{
	long min_sse = LONG_MAX;
	int min_mv_x = 0, min_mv_y = 0; // Local equivalent since the macroblock's motion vector is used in the loops.
	mb->mv[0].x = 0, mb->mv[0].y = 0; // Initialise the motion vector to 0.
	
	for (int it = 0; it < 3; it++) {
		int dx = search_width / (2 << it), dy = search_height / (2 << it);
		int inc_x = (it != 2 ? dx : 1), inc_y = (it != 2 ? dy : 1);
		for (int mv_x = mb->mv[0].x - dx; mv_x <= mb->mv[0].x + dx; mv_x += inc_x) {
			for (int mv_y = mb->mv[0].y - dy; mv_y <= mb->mv[0].y + dy; mv_y += inc_y) {
				if(it && mv_x == mb->mv[0].x && mv_y == mb->mv[0].y) continue; // SSE of center has already been computed in the previous iteration.
				long sse = getSSE(mb, mv_x, mv_y);
				if (sse < min_sse) {
					min_sse = sse, min_mv_x = mv_x, min_mv_y = mv_y;
				}
			}
		}

		// Now it is safe to update the macroblock's motion vector with the local values.
		mb->mv[0].x = min_mv_x, mb->mv[0].y = min_mv_y;
	}

	return min_sse;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////