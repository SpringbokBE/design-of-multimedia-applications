# Design of Multimedia Applications
E017920A - Design of Multimedia Applications, Assignments - Faculty of Engineering and Architecture, UGent

## About
The first two assignments cover applications that make use of the open source [Gstreamer multimedia framework](https://gstreamer.freedesktop.org/).
The third assignment involves the implementation of some essential steps in the encoding process of a MPEG-2 based video encoder for YUV video files.

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

## Assignment 3

Four functional blocks of a block-based video codec will be implemented: transformation, quantisation, intra-prediction and motion estimation. The full assignment can be found ![here](Assignment%203/data/Assignment.pdf).

## Requirements

The application is tested on Kubuntu 19.04 and Windows 10 using:

* `Python 3.7.3`
* `Gstreamer 1.15.90`
* `PyCairo 1.16.2`
* `PyGObject 3.32.0`
* `Microsoft Visual Studio 2019`

## Installation

##  Assignment 1 & 2

Run any of the `.py` or `.sh` files.

## Assignment 3

### Compilation
Open the _Developer Command Prompt for VS2019_ and navigate to the `Assignment 3` folder.
Create the build files using `cmake -B build`, then open `Encoder.sln` using Microsoft Visual Studio 2019. Under the solution properties, make sure `Encoder` is set as the startup project. The encoder can now be compiled using VS2019.

### Encoding
`Encoder.exe <inputfile> <input_width> <input_height> <qp> <I-interval> <outputfile>`
*	`<inputfile>`			: Uncompressed YUV video file.
*	`<input_width>`		: Width of a frame in pixels.
*	`<input_height>`	: Height of a frame in pixels.
*	`<qp>`						: Takes a value between 2 - 52, the higher the value, the coarser the quantisation.
*	`<I-interval>`		: Indicates the number of frames in between two P-frames, in which motion compensation will be used w.r.t. the previous P-frame. Higher intervals usually provide better compression.
*	`<outputfile>`		: Name for the encoded (compressed) output file.

### Decoding
`Decoder.exe <inputfile> <outputfile>`
*	`<inputfile>`			: Encoded YUV video file
*	`<outputfile>`		: Name for the decoded (uncompressed) output file.

### Viewing
`YUVViewer.exe <inputfile>`
*	`<inputfile>`			: Uncompressed YUV video file.

### Motion vector visualisation
Run `MVVisualiser.py`. This script no longer works since macroblock partitioning has been implemented. The original test data (without partitioning), however, still resides in the `data\*\` folders. Frames can easily be extracted from the uncompressed YUV video files using `ffmpeg -pixel_format yuv420p -video_size 352x288 -framerate 30 -i xxx.yuv -f image2 frame%3d.png`.

### PSNR calculation
`PSNR.exe <inputfile> <inputfile>`
*	`<inputfile>`			: Uncompressed YUV video file.

## Folder structure

```
Assignment 1 // Contains scripts involving assignment 1.
Assignment 2 // Contains scripts involving assignment 2.
Assignment 3 // Contains files involving assignment 3.
|- data/     // Contains test samples, frames and output spreadsheets.
|- include/  // Contains the relevant header files.
|- lib/      // Contains decoder libraries.
|- src/      // Contains the implementation files.
|- tools/    // Contains tools for decoding, testing and visualisation.
```

## Results

### Assignment 3
It is clear that increasing the value of QP has the most significant impact on the compressed size.
An increased number of I-frames is also beneficial for the compression.
<p style = "float:left;">
<img src = "Assignment 3/data/screens/SizeFlower.png" width = "290px" alt = "Compressed size vs. QP, flower_50.yuv">
<img src = "Assignment 3/data/screens/SizeFootball.png" width = "290px" alt = "Compressed size vs. QP, football_50.yuv">
<img src = "Assignment 3/data/screens/SizeForeman.png" width = "290px" alt = "Compressed size vs. QP, foreman_50.yuv">
</p>
The partitioning cost factor is determined empirically and varies between the datasets. A value of 10,000 has been chosen in this project.
<p style = "float:left;">
<img src = "Assignment 3/data/screens/CostFlower.png" width = "290px" alt = "Compressed size vs. Cost factor, flower_50.yuv">
<img src = "Assignment 3/data/screens/CostFootball.png" width = "290px" alt = "Compressed size vs. Cost factor, football_50.yuv">
<img src = "Assignment 3/data/screens/CostForeman.png" width = "290px" alt = "Compressed size vs. Cost factor, foreman_50.yuv">
</p>
Screenshot of output of the motion vector visualisation script.
<p style = "float:left;">
<img src = "Assignment 3/data/screens/MVVisualiser.png" width = "580px" alt = "Motion vector visualisation">
</p>

## Authors

* **Gerbrand De Laender** (gerbrand.delaender@ugent.be)
* **Damon Verbeyst** (damon.verbeyst@ugent.be)
