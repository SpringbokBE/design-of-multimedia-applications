# Design of Multimedia Applications
E017920A - Design of Multimedia Applications, Assignments - Faculty of Engineering and Architecture, UGent

## About
The assignments cover applications that make use of the open source [Gstreamer multimedia framework](https://gstreamer.freedesktop.org/).

## Assignment 1

## Assignment 2

### Assignment 2.1

Write a command line pipeline performing a visualization (autovideosink) and a mix of two video streams after which one effect is applied on the mixed video. After the effect has been added, the video must be displayed (autovideosink) and stored as an H.264/AVC compressed video stream inside an MKV container. Audio can be ignored. Use the following videos and store them in the Downloads folder.
  
Video 1: http://users.datasciencelab.ugent.be/MM/sintel_SD.mp4  
Video 2: http://users.datasciencelab.ugent.be/MM/sita_SD.mp4  
  
Files: `DMA_2_1.sh`

### Assignment 2.2

Write a python GUI application in which two video’s are visualized and mixed. The transparency (alpha) of each video stream can be set using one horizontal slider on the screen. The output after mixing must be both displayed in the GUI window and stored to disk simultaneously. The path to the video files can be hard coded and video seeking capabilities should not be present because a live stream is supposed to enter the system.
  
Files: `DMA_2_2.py`

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
