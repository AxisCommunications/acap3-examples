# A combined vdo stream and larod based ACAP3 application running inference on edge device
This readme file explains how to build an application that uses:
- vdo to fetch frames from e.g. a camera
- a library called libyuv to do some image preprocessing
- [larod API](../FAQs.md#WhatisLarod?) to load a graph model and run classification inferences on it

The application is built by using the containerized Axis API and toolchain images.

Together with this README file you should be able to find a directory called app, that directory contains the "vdo_larod" application source code, which can easily be compiled and run with the help of the tools and step by step below.

## Detailed outline of example application
This application opens a client to vdo and starts fetching frames (in a new thread) in the yuv format. It tries to match twice the WIDTH and HEIGHT that is required by the neural network. The thread fetching frames is written so that it always tries to provide a frame as new as possible even if not all previous frames have been processed by libyuv and larod. The implementation of the vdo specific parts of the app can be found in file "imgprovider.c".

The image preprocessing is then started by libyuv in "imgconverter.c". The code will perform the following three steps:
1. Convert the full size NV12 image delivered by vdo to interleaved BGRA color format since the bulk of libyuv functions works on that format.
2. Crop out a part of the full size image. The crop will be taken from the center of the vdo image and be as big as possible while still maintaining the WIDTH x HEIGHT aspect ratio. This crop will then be scaled to be of the same size as what the neural network requires (WIDTH x HEIGHT).
3. Lastly the scaled image will be converted from BGRA to interleaved RGB which is a common format for e.g. Mobilenet CNNs.

Finally larod will load a neural network model and start processing. It simply takes the images produced by vdo and libyuv and makes synchronous inferences calls to the neural network that was loaded. These function calls return when inferences are finished upon which the application parses the output tensor provided to print the top result to syslog/application log. The larod related code is found in "vdo_larod.c".

## Getting started
These instructions below will guide you on how to execute the code. Below is the structure and scripts used in the example:

```bash
vdo-larod
├── app
│   ├── model
|   │   ├── mobilenet_v2_1.9_224_quant_edgetpu.larod
|   │   ├── mobilenet_v2_1.9_224_quant.larod
│   ├── argparse.c
│   ├── argparse.h
│   ├── imgconverter.c
│   ├── imgconverter.h
│   ├── imgprovider.c
│   ├── imgprovider.h
│   ├── LICENSE
│   ├── Makefile
│   ├── package.conf.cpu
│   ├── package.conf.edgetpu
│   ├── vdo_larod.c
├── yuv
├── build.sh
├── Dockerfile
└── README.md
```
* **argparse.c/h** - Implementation of argument parser, written in C.
* **build.sh** - Builds and tags the image of vdo_larod image e.g., axisecp/vdo_larod:1.0 and the .eap file.
* **Dockerfile** - Docker file with the specified Axis toolchain and API container to build the example specified.
* **imageconverter.c/h** - Implementation of libyuv parts, written in C.
* **imageprovider.c/h** - Implementation of vdo parts, written in C.
* **LICENSE** - Text file which lists all open source licensed source code distributed with the application.
* **Makefile** - Used by the make tool to build the program.
* **mobilenet_v2_1.9_224_quant_edgetpu.larod** - Model used for Google TPU.
* **mobilenet_v2_1.9_224_quant.larod** - Model used for CPU with TensorFlow Lite.
* **package.conf.cpu** - Defines the application and its configuration when building for CUP with TensorFlow Lite.
* **package.conf.edgetpu** - Defines the application and its configuration when building chip and model for Google TPU.
* **README.md** - Step by step instructions on how to run the example.
* **vdo-larod.c** - Application using larod, written in C.
* **yuv** - Folder containing files for building libyuv.

Below is the structure and script used when building libyuv:

```bash
vdo-larod
├── yuv
│   ├── 0001-Create-a-shared-library.patch
│   ├── build.sh
│   ├── Dockerfile
│   ├── README.md
```
* **build.sh** - Builds and tags the image of builder-yuv e.g., builder-yuv.
* **0001-Create-a-shared-library.patch** - Patch for building the image of builder-yuv e.g., builder-yuv.
* **Dockerfile** - Docker file for building libyuv.
* **README.md** - Step by step instructions on how to build libyuv.

### Limitations
* ARTPEC-7 based product.
* This application was not written to optimize performance.

### How to run the code
Below is a step by step on the whole process.

#### Build the application
> [!IMPORTANT]
> *Depending on the network you are connected to,
The file that needs those settings is: *~/.docker/config.json.*
For reference please see: https://docs.docker.com/network/proxy/ and a
[script for Axis device here](../FAQs.md#HowcanIset-upnetworkproxysettingsontheAxisdevice??).*

Depending on selected chip, different model can be used for running larod. This is configured through attributes in package.conf:
- APPOPTS, which contains the application command line options.
- OTHERFILES, shows files to be included in the package e.g. model. Files listed here are copied to the application directory during installation.
- PACKAGENAME, a user friendly package name which is also part of the .eap file name.

Different devices support different chips and models.

##### Alternative Chip 2 - CPU with TensorFlow Lite
Standing in your working directory, run the following command:
```
cp app/package.conf.cpu app/package.conf
```
##### Alternative Chip 4 - Google TPU
Standing in your working directory, run the following command:
```
cp app/package.conf.edgetpu app/package.conf
```
##### Build script
Standing in your working directory, run the following build command:
```
./build.sh <APP_IMAGE> <UBUNTU_VERSION> .
```
<APP_IMAGE> is the name to tag the image with, e.g., axisecp/vdo_larod:1.0
<UBUNTU_VERSION> is the Ubuntu version to be used, e.g., 19.10. This is used when building libyuv.

This script will also copy the build result from the container image to a local directory build.

Working dir now contains a build folder with the following files:
```bash
vdo-larod
├── build
│   ├── argparse.c
│   ├── argparse.h
│   ├── imgconverter.c
│   ├── imgconverter.h
│   ├── imgprovider.c
│   ├── imgprovider.h
│   ├── include
│   ├── lib
│   ├── LICENSE
│   ├── Makefile
│   ├── model
│   ├── package.conf
│   ├── package.conf.cpu
│   ├── package.conf.edgetpu
│   ├── package.conf.orig
│   ├── param.conf
│   ├── vdo_larod*
│   ├── vdo_larod.c
│   ├── vdo_larod_cpu_1_0_0_armv7hf.eap / vdo_larod_edgetpu_1_0_0_armv7hf.eap
│   ├── vdo_larod_1_0_0_LICENSE.txt

```
* **include** - Folder containing include files for libyuv.
* **lib** - Folder containing compiled library files for libyuv.
* **model** - Folder containing models used in this application.
* **package.conf** - Defines the application and its configuration.
* **package.conf.orig** - Defines the application and its configuration, original file.
* **param.conf** - File containing application parameters.
* **vdo_larod** - Application executable binary file.
* **vdo_larod_cpu_1_0_0_armv7hf.eap** - Application package .eap file,
  if alternative chip 2 has been built.
* **vdo_larod_edgetpu_1_0_0_armv7hf.eap** - Application package .eap file,
  if alternative chip 4 has been built.
* **vdo_larod_1_0_0_LICENSE.txt** - Copy of LICENSE file.

#### Install your application
Installing your application on an Axis video device is as simple as:

Browse to the following page (replace <axis_device_ip> with the IP number of your Axis video device):
```
http://<axis_device_ip>/#settings/apps
```

Click Add, the + sign and browse to the newly built "vdo_larod_cpu_1_0_0_armv7hf.eap" or "vdo_larod_edgetpu_1_0_0_armv7hf.eap" in build folder.

Click Install, vdo_larod is now available as an application on the device,
using the friendly name "vdo_larod_cpu" or "vdo_larod_edgetpu".

#### Run the application
Run the application by clicking on the application icon and enable the Start switch.

Application log is found by:
```
http://<axis_device_ip>/axis-cgi/admin/systemlog.cgi?appname=vdo_larod
```
or by clicking on the "App log" link.

#### The expected output

##### Alternative Chip 2 - CPU with TensorFlow Lite
vdo_larod[27779]: Creating VDO image provider and creating stream 320 x 240
vdo_larod[27779]: Dump of vdo stream settings map =====
vdo_larod[27779]: chooseStreamResolution: We select stream w/h=320 x 240 based on VDO channel info.
vdo_larod[27779]: Setting up larod connection with chip 2 and model /usr/local/packages/vdo_larod/model/mobilenet_v2_1.0_224_quant.larod
vdo_larod[27779]: Creating temporary files and memmaps for inference input and output tensors
vdo_larod[27779]: Start fetching video frames from VDO
vdo_larod[27779]: createAndMapTmpFile: Setting up a temp fd with pattern /tmp/larod.in.test-XXXXXX and size 150528
vdo_larod[27779]: createAndMapTmpFile: Setting up a temp fd with pattern /tmp/larod.out.test-XXXXXX and size 1001
vdo_larod[27779]: Converted image in 4 ms
vdo_larod[27779]: Ran inference for 373 ms
vdo_larod[27779]: Top result: index 695 with probability 51.60%
vdo_larod[27779]: Converted image in 3 ms
vdo_larod[27779]: Ran inference for 324 ms
vdo_larod[27779]: Top result: index 695 with probability 52.40%

##### Alternative Chip 4 - Google TPU
vdo_larod[32250]: Creating VDO image provider and creating stream 320 x 240
vdo_larod[32250]: Dump of vdo stream settings map =====
vdo_larod[32250]: chooseStreamResolution: We select stream w/h=320 x 240 based on VDO channel info.
vdo_larod[32250]: Setting up larod connection with chip 4 and model /usr/local/packages/vdo_larod/model/mobilenet_v2_1.0_224_quant_edgetpu.larod
vdo_larod[32250]: Converted image in 4 ms
vdo_larod[32250]: Creating temporary files and memmaps for inference input and output tensors
vdo_larod[32250]: Start fetching video frames from VDO
vdo_larod[32250]: createAndMapTmpFile: Setting up a temp fd with pattern /tmp/larod.in.test-XXXXXX and size 150528
vdo_larod[32250]: createAndMapTmpFile: Setting up a temp fd with pattern /tmp/larod.out.test-XXXXXX and size 1001
vdo_larod[32250]: Converted image in 4 ms
vdo_larod[32250]: Ran inference for 27 ms
vdo_larod[32250]: Top result: index 535 with probability 46.40%
vdo_larod[32250]: Converted image in 3 ms
vdo_larod[32250]: Ran inference for 7 ms
vdo_larod[32250]: Top result: index 535 with probability 45.60%
vdo_larod[32250]: Converted image in 3 ms
vdo_larod[32250]: Ran inference for 5 ms
vdo_larod[32250]: Top result: index 535 with probability 45.20%
vdo_larod[32250]: Ran inference for 5 ms

##### Conclusion
- This is an example of test data, which is dependant on selected device and chip.
- Running inference is much faster on chip Google TPU than CPU with TensorFlow Lite.
- Converting images takes almost the same time on both chips.

## License
**Apache License 2.0**
