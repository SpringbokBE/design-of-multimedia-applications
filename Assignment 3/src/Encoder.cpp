/**
 *  @file    Encoder.cpp
 *  @author  Unknown
 *  @email   Unknown
 *  @date    24/04/2020
 *  @version 1.0
 *
 *  @brief   E017920A, Assignment, video encoding
 *
 *  @section DESCRIPTION
 *
 *  Block-based encoder for (uncompressed) YUV video files.
 *
 *	Creates a pipeline comprising intra prediction, motion compensation, DCT transformation,
 *	quantisation and finally entropy coding.
 *	
 *	Usage of the encoder is <inputfile> <input_width> <input_height> <qp> <I-interval> <outputfile>.
 *
 *	<inputfile>			: Uncompressed YUV video file.
 *	<input_width>		: Width of a frame in pixels.
 *	<input_height>	: Height of a frame in pixels.
 *	<qp>						: Takes a value between 2 - 52, the higher the value, the coarser the quantisation.
 *	<I-interval>		: Indicates the number of frames in between two P-frames, in which motion
 *										compensation will be used w.r.t. the previous P-frame. Higher intervals
 *										usually provide better compression
 *	<outputfile>		: Name for the encoded output file.
 *
 */

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "Encoder.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "YUVFileInput.h"
#include "BitFileOutput.h"

#include "MotionCompensator.h"
#include "IntraPredictor.h"
#include "DCTTransform.h"
#include "Quantiser.h"
#include "EntropyCoder.h"

#include "IQuantiser.h"
#include "IDCTTransform.h"
#include "IMotionCompensator.h"
#include "IIntraPredictor.h"
#include "Clipper.h"

#include "Config.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

int Encoder::Encode(char *inputfile, int width, int height, int qp, int i_interval, char *outputfile, int cost)
{
	results.open(RES_NAME, std::ofstream::app);
	YUVFileInput in(inputfile, width/16, height/16);
	BitFileOutput out(outputfile);

	EntropyCoder entropy_coder(&out);
	DCTTransform dct;
	MotionCompensator mc(SEARCH_WIDTH, SEARCH_HEIGHT, cost);
	IntraPredictor ip;
	
	printf("File:\t%s\nWidth:\t%d\nHeight:\t%d\nQP:\t%d\nI-interval:\t%d\nSearch window: \t%dx%d\n\n", inputfile, width, height, qp, i_interval, SEARCH_WIDTH, SEARCH_HEIGHT);

	// Write header
	entropy_coder.CodeUInt(width/16);
	entropy_coder.CodeUInt(height/16);
	out.WriteBit(DO_INTRA);
	out.WriteBit(DO_MC);
	out.WriteBit(DO_DCT);
	out.WriteBit(DO_QT);

	if (DO_QT) entropy_coder.CodeUInt(qp);
	if (DO_MC) entropy_coder.CodeUInt(i_interval);

	// Decoder
	IDCTTransform idct;
	IMotionCompensator imc;
	IIntraPredictor iip;

	// Coding loop
	Frame *current_frame = in.getNextFrame();
	int count = 0;

	while (current_frame != NULL)
	{
		bool i_frame = (count % i_interval == 0);
		bool p_frame = (count % i_interval != 0);

		printf("#%3d %c\n", count++, p_frame?'P':'I');

		long used_bits = 0;
		int num_mb = current_frame->getNumMB();

		ip.setCurrentFrame(current_frame);
		iip.setCurrentFrame(current_frame);

		// Counter variables for intra prediction mode statistics
		int mode0 = 0, mode1 = 0, mode2 = 0, mode3 = 0;

		for (int current_mb = 0; current_mb < num_mb; current_mb++)
		{
			Macroblock *mb = current_frame->getMacroblock(current_mb);		// Get YUV macroblock

			mb->qp = qp;

			int mode = -1; // Intra prediction mode

			if (DO_INTRA && i_frame)	mode = ip.predictIntra(current_mb, width/16, height/16); // Intra prediction
			if (DO_MC && p_frame)		mc.motionCompensate(mb);				// Motion Compensate
			if (DO_DCT)					dct.Transform(mb);						// DCT-transform
			if (DO_QT)					Quantiser::Quantise(mb, qp);			// Quantise
			
			used_bits += entropy_coder.EntropyCode(mb, p_frame, qp);		// EntropyCode

			if (DO_INTRA && i_frame)	
			{
				entropy_coder.CodeIntraMode(mode);
				used_bits += 2; // 2 bits needed to signal intra prediction mode
			}

			// Intra prediction mode statistics
			if (DO_INTRA && i_frame)
			{
				if (mode == 0) mode0++;
				else if (mode == 1) mode1++;
				else if (mode == 2) mode2++;
				else if (mode == 3) mode3++;
			}
			if (DO_MC && p_frame) {
				if (mb->partitions) mode0++;
				else mode1++;
			}
			// In order to have a valid reference picture, we need to decode the macroblock again.
			if (DO_QT)					IQuantiser::IQuantise(mb, qp);			// De-Quantise
			if (DO_DCT)					idct.ITransform(mb);					// Inverse Transform
			if (DO_INTRA && i_frame)	iip.predictIntra(current_mb, width/16, height/16, mode); // Intra prediction
			if (DO_MC && p_frame)		imc.iMotionCompensate(mb);				// Motion Compensate

			Clipper::Clip(mb);
		}

		printf("%8ld %4d %4d %4d %4d\n", used_bits, mode0, mode1, mode2, mode3);

		// Set reference frame to current frame
		delete mc.getReferenceFrame();
		mc.setReferenceFrame(current_frame);
		imc.setReferenceFrame(current_frame);

		// Get new frame
		current_frame = in.getNextFrame();
	}

	// Clean up
	delete mc.getReferenceFrame();

	printf("\nTotal: %8ld\n", entropy_coder.getTotalUsedBits());
	results << cost << ", " << entropy_coder.getTotalUsedBits() << "\n";
	results.close();

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	Encoder enc;

	//if (argc != 7)
	//{
	//	printf("\nUSAGE:   %s <inputfile> <input_width> <input_height> <qp> <I-interval> <outputfile>\n", argv[0]);
	//	printf("            <input_width> and <input_height> are expressed in pixels\n");
	//	return 1;
	//}
	//
	//enc.Encode(argv[1], atoi(argv[2]), atoi(argv[3]),atoi(argv[4]),atoi(argv[5]),argv[6]);
	for (int i = 1; i < 10; i++) {
		for(int qp = 2; qp < 52; qp += 2){
			enc.Encode("../data/foreman_50.yuv", 352, 288, qp, i, "../data/foreman_50.enc", 10000);
		}
	}
	//enc.Encode("../data/foreman_50.yuv", 352, 288, 40, 5, "../data/foreman_50.enc", 0);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////