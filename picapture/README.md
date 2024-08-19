# Introduction

This is a minimal example of how to read images from the Raspberry Pi camera into OpenCV.


# Prerequisites

You must install:
 - `libopencv-dev`
 - `gstreamer1.0-tools` 
 - `gstreamer1.0-plugins-base-apps`

Run this command on your Raspberry Pi:

```
$ sudo apt install libopencv-dev gstreamer1.0-tools gstreamer1.0-plugins-base-apps
```

## Test the camera and gstreamer installation

Run this command:

```
$ gst-launch-1.0 libcamerasrc ! video/x-raw, width=800, height=600, framerate=15/1 ! videoconvert ! ximagesink
```

# Build and run

Make a `build` directory, and run `cmake` and then `make`.

```
$ mkdir build
$ cd build
$ cmake ..
$ make 
$ ./PiCapture
```

A direct HDMI video output is recommended. Remote access over VNC or X forwarding will be a bit slow and laggy.
