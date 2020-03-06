#!/bin/bash

# File name:  DMA_2_1.sh
# Author:     Gerbrand De Laender, Damon Verbeyst
# Date:       06/03/2020
# Email:      gerbrand.delaender@ugent.be, damon.verbeyst@ugent.be
# Brief:      E017920A, Design of Multimedia Applications, Assignment
# About:      Visualisation of GStreamer video mixing capabilities.

gst-launch-1.0 -e \
uridecodebin uri=file:///home/dma/Downloads/sintel_SD.mp4 ! videoscale ! mixer.sink_0 \
uridecodebin uri=file:///home/dma/Downloads/sita_SD.mp4 ! videoscale ! mixer.sink_1 \
videomixer name=mixer sink_0::alpha=0.5 sink_1::alpha=0.5 ! agingtv ! \
tee name=t ! queue ! videoconvert ! autovideosink \
t. ! queue ! videoconvert ! x264enc tune=zerolatency ! matroskamux ! filesink location=DMA_2_1.mkv