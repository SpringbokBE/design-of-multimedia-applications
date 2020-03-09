# Design of Multimedia Applications
E017920A - Design of Multimedia Applications, Assignments - Faculty of Engineering and Architecture, UGent

## About
The assignments cover applications that make use of the open source [Gstreamer multimedia framework](https://gstreamer.freedesktop.org/).

## Assignment 1

### Assignment 1.1
Provide a command line for playback of the following video files, without using autopluggers like the playbin, playsink, decodebin, and uridecodebin elements.
  
Video 1: http://users.datasciencelab.ugent.be/MM/sintel-1024-surround.mp4  
Video 2: http://users.datasciencelab.ugent.be/MM/tears_of_steel_720p.mp4  
  
To not overload the network, you can test the decoding and playback using the local file in the Downloads folder of the virtual machine.
  
Files: `DMA_1_1.sh`

### Assignment 1.2
Extend the previous pipeline such that the autovideosink and autoaudiosink autoplugger can be replaced as well.
  
Files: `DMA_1_2.sh`

### Assignment 1.3
Provide for each video a command line to decode them, downscale height and width by a factor 2 (keep the aspect ratio), and encode them without changing the type of container (mp4) and codec. The audio stream needs to remain untouched (stream copy instead of decode encode).
  
Files: `DMA_1_3.sh`

### Assignment 1.4
Provide for each video a playout pipeline using autopluggers and put a logo in the upper left corner of the video. Find a decent logo and choose the size appropriately.
  
Files: `DMA_1_4.sh`

## Assignment 2

### Assignment 2.1

Write a command line pipeline performing a visualization (autovideosink) and a mix of two video streams after which one effect is applied on the mixed video. After the effect has been added, the video must be displayed (autovideosink) and stored as an H.264/AVC compressed video stream inside an MKV container. Audio can be ignored. Use the following videos and store them in the Downloads folder.
  
Video 1: http://users.datasciencelab.ugent.be/MM/sintel_SD.mp4  
Video 2: http://users.datasciencelab.ugent.be/MM/sita_SD.mp4  
  
Files: `DMA_2_1.sh`

### Assignment 2.2

Write a python GUI application in which two video’s are visualized and mixed. The transparency (alpha) of each video stream can be set using one horizontal slider on the screen. The output after mixing must be both displayed in the GUI window and stored to disk simultaneously. The path to the video files can be hard coded and video seeking capabilities should not be present because a live stream is supposed to enter the system.
  
Files: `DMA_2_2.py`

## Assignment 2.3

Add some buttons on the GUI such that different effects can be applied to the resulting video (at least 2). One of the effects should be able to put your channel’s logo in the upper left corner of the video. Take care the user interface is convincing as a proof of concept towards the outside "non-engineering" world.

Files `DMA_2_3.py`, `logo.png`

## Requirements

The application is tested on Kubuntu 19.04 using:

* `Python 3.7.3`
* `Gstreamer 1.15.90`
* `PyCairo 1.16.2`
* `PyGObject 3.32.0`

## Installation

Run any of the `.py` or `.sh` files.

## Folder structure

```
Assignment 1 // Contains scripts involving assignment 1.
Assignment 2 // Contains scripts involving assignment 2.
```

## Authors

* **Gerbrand De Laender** (gerbrand.delaender@ugent.be)
* **Damon Verbeyst** (damon.verbeyst@ugent.be)
