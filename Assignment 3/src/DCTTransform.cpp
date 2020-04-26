/**
 *  @file    DCTTransform.cpp
 *  @author  Unknown, Gerbrand De Laender, Damon Verbeyst
 *  @email   gerbrand.delaender@ugent.be, damon.verbeyst@ugent.be
 *  @date    24/04/2020
 *  @version 1.0
 *
 *  @brief   E017920A, Assignment, video encoding
 *
 *  @section DESCRIPTION
 *
 *  Implementation of the Discrete Cosine Transform (DCT) step.
 *
 */

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "DCTTransform.h"

#define _USE_MATH_DEFINES
#include <math.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//
// Allocate and initialise the DCT coefficients matrix.
//

DCTTransform::DCTTransform()
{
	A = new double* [8]();
	for (int i = 0; i < 8; i++) {
		A[i] = new double[8]();
	}

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			double cj = (j == 0 ? sqrt(0.125) : sqrt(0.250));
			A[j][i] = cj * cos((2.0 * i + 1) * j * M_PI / 16.0);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//
// Deallocate the DCT coefficients matrix.
//

DCTTransform::~DCTTransform()
{
	for (int i = 0; i < 8; i++) {
		delete[] A[i];
	}
	delete[] A;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//
//	Applies the 8x8 Discrete Cosine Transform (DCT) on all channels (Y, Cb, Cr) of the macroblock.
//

void DCTTransform::Transform(Macroblock* mb)
{
	mb->state = DCT;

	// Transform the luma channel (16x16)
	Transform_8x8(mb->luma, 0, 0);
	Transform_8x8(mb->luma, 8, 0);
	Transform_8x8(mb->luma, 0, 8);
	Transform_8x8(mb->luma, 8, 8);

	// Transform the chroma channels (8x8)
	Transform_8x8(mb->cb, 0, 0);
	Transform_8x8(mb->cr, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//
//	Applies the 8x8 Discrete Cosine Transform (DCT) on the block.
//

void DCTTransform::Transform_8x8(pixel** block, int offset_x, int offset_y)
{
	double temp[8][8] = {};

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			double sum = 0;
			for (int k = 0; k < 8; k++) {
				sum += A[i][k] * block[k + offset_y][j + offset_x];
			}
			temp[i][j] = sum;
		}
	}

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			double sum = 0;
			for (int k = 0; k < 8; k++) {
				sum += temp[i][k] * A[j][k];
			}
			block[i + offset_y][j + offset_x] = (pixel) sum;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////