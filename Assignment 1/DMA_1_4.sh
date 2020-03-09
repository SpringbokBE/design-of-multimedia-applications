#!/bin/bash

# File name:  DMA_2_1.sh
# Author:     Gerbrand De Laender, Damon Verbeyst
# Date:       16/02/2020
# Email:      gerbrand.delaender@ugent.be, damon.verbeyst@ugent.be
# Brief:      E017920A, Design of Multimedia Applications, Assignment
# About:      Introduction to GStreamer.

gst-launch-1.0 -e \
souphttpsrc location=http://users.datasciencelab.ugent.be/MM/sintel-1024-surround.mp4 ! qtdemux name=d \
d.video_0 ! queue ! h264parse ! avdec_h264 ! gdkpixbufoverlay location=logo.png offset-x=20 offset-y=20 ! autovideosink \
d.audio_0 ! queue ! aacparse ! faad ! audioconvert ! autoaudiosink

gst-launch-1.0 -e \
souphttpsrc location=http://users.datasciencelab.ugent.be/MM/tears_of_steel_720p.mp4 ! qtdemux name=d \
d.video_0 ! queue ! h264parse ! avdec_h264 ! gdkpixbufoverlay location=logo.png offset-x=20 offset-y=20 ! autovideosink \
d.audio_0 ! queue ! mpegaudioparse ! mpg123audiodec ! audioconvert ! autoaudiosink