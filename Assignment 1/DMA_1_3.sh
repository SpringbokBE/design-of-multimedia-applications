#!/bin/bash

# File name:  DMA_2_1.sh
# Author:     Gerbrand De Laender, Damon Verbeyst
# Date:       16/02/2020
# Email:      gerbrand.delaender@ugent.be, damon.verbeyst@ugent.be
# Brief:      E017920A, Design of Multimedia Applications, Assignment
# About:      Introduction to GStreamer.

gst-launch-1.0 -e \
souphttpsrc location=http://users.datasciencelab.ugent.be/MM/sintel-1024-surround.mp4 ! qtdemux name=d \
d.video_0 ! queue ! h264parse ! avdec_h264 ! videoscale ! video/x-raw,width=512,height=218 ! x264enc rc-lookahead=5 ! m. \
d.audio_0 ! queue ! aacparse ! m. \
mp4mux name=m ! filesink location=sintel-1024-surround_out.mp4

gst-launch-1.0 -e \
souphttpsrc location=http://users.datasciencelab.ugent.be/MM/tears_of_steel_720p.mp4 ! qtdemux name=d \
d.video_0 ! queue ! h264parse ! avdec_h264 ! videoscale ! video/x-raw,width=512,height=214 ! x264enc rc-lookahead=5 ! m. \
d.audio_0 ! queue ! mpegaudioparse ! m. \
mp4mux name=m ! filesink location=tears_of_steel_720p_out.mp4