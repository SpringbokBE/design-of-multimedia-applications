/**
 *  @file    Quantiser.cpp
 *  @author  Unknown, Gerbrand De Laender, Damon Verbeyst
 *  @email   gerbrand.delaender@ugent.be, damon.verbeyst@ugent.be
 *  @date    24/04/2020
 *  @version 1.0
 *
 *  @brief   E017920A, Assignment, video encoding
 *
 *  @section DESCRIPTION
 *
 *  Implementation of the quantisation step.
 *
 *	In this step, all three channels (Y, Cb, Cr) of the macroblock are quantised. The amount of
 *  quantisation is determined by the parameter <qp> which is specified when running the encoder.
 *
 */

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "Quantiser.h"

#include <math.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T> int sgn(T val)
{
	return (T(0) < val) - (val < T(0));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void Quantiser::Quantise(Macroblock* mb, int qp)
{
  mb->state = QT;
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			mb->luma[i][j] = static_cast<pixel>(sgn(mb->luma[i][j]) * round(abs(mb->luma[i][j]) / qp));
      if (i % 2 || j % 2) continue; // Cb and Cr are subsampled by a factor of 2.
			mb->cb[i / 2][j / 2] = static_cast<pixel>(sgn(mb->cb[i / 2][j / 2]) * round(abs(mb->cb[i / 2][j / 2]) / qp));
			mb->cr[i / 2][j / 2] = static_cast<pixel>(sgn(mb->cr[i / 2][j / 2]) * round(abs(mb->cr[i / 2][j / 2]) / qp));
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////