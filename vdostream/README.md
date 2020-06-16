# An vdo stream based ACAP3 application on an edge device
This readme file explains how to build an ACAP3 that uses the vdostream API. The application is built by using the containerized Axis API and toolchain images.

Together with this README file you should be able to find a directory called app, that directory contains the "vdoencodeclient" application source code which can easily be compiled and run with the help of the tools and step by step below.

This example illustrates how to continuously capture frames from the vdo service, access the received buffer contents as well as the frame metadata. Captured frames are logged in the Application log.

## Getting started
These instructions below will guide you on how to execute the code. Below is the structure and scripts used in the example:

```bash
vdostream
├── app
│   ├── LICENSE
│   ├── Makefile
│   ├── vdoencodeclient.c
├── Dockerfile
└── README.md
```
* **Dockerfile**        - Docker file with the specified Axis toolchain and API container to build the example specified
* **LICENSE**           - Text file which lists all open source licensed source code distributed with the application.
* **Makefile**          - Used by the make tool to build the program.
* **README.md**         - Step by step instructions on how to run the example.
* **vdoencodeclient.c** - Application to capture the frames using vdo service in C.

### Limitations
* The example is done for the armv7hf architecture, but it is possible to update to aarch64 architecture.
* Supported video compression formats for an Axis video device are found in the datasheet of the device.

### How to run the code
Below is a step by step on the whole process.

#### Build the application
Standing in your working directory run the following commands:
```
docker build --tag <APP_IMAGE> .
```
<APP_IMAGE> is the name to tag the image with, e.g., axisecp/vdoencodeclient:1.0

Copy the build result from the container image to a local directory build
```
docker cp $(docker create <APP_IMAGE>):/opt/app ./build
```

Working dir now contains a build folder with the following files:
```bash
vdostream
├── app
│   ├── LICENSE
│   ├── Makefile
│   ├── vdoencodeclient.c
├── Dockerfile
└── README.md
├── build
│   ├── LICENSE
│   ├── Makefile
│   ├── package.conf
│   ├── package.conf.orig
│   ├── param.conf
│   ├── vdoencodeclient*
│   ├── vdoencodeclient_1_0_0_armv7hf.eap
│   ├── vdoencodeclient_1_0_0_LICENSE.txt
│   ├── vdoencodeclient.c
```
* **package.conf**                       - Defines the application and its configuration.
* **package.conf.orig**                  - Defines the application and its configuration, original file.
* **param.conf**                         - File containing application parameters.
* **vdoencodeclient***                   - Application executable binary file.
* **vdoencodeclient_1_0_0_armv7hf.eap**  - Application package .eap file.
* **vdoencodeclient_1_0_0_LICENSE.txt**  - Copy of LICENSE file.

#### Install your application.
Installing your application on an Axis video device is as simple as:

Browse to the following page (replace <axis_device_ip> with the IP number of your Axis video device):
```
http://<axis_device_ip>/#settings/apps
```

Click Add, the + sign and browse to the newly built vdoencodeclient_1_0_0_armv7hf.eap in build folder.

Click Install, vdoencodeclient is now available as an application on the device.

#### Run the application with default vdo parameters.

Run the application by clicking on the application icon and enable the Start switch.

Application log is found by:
```
http://<axis_device_ip>/axis-cgi/admin/systemlog.cgi?appname=vdoencodeclient
```
or by clicking on the "App log" link.

#### Run application with different vdo parameters

It is possible to run the application on the command line with different arguments, in the following order:
* format
  video compression format, h264 (default), h265 or jpeg
* frames
  number of frames
* output
  output filename

Enable ssh on camera by:
```
http://<axis_device_ip>/axis-cgi/admin/param.cgi?action=update&Network.SSH.Enabled=yes
```

Start terminal:
```
ssh root@<axis_device_ip>
```

Run application from the filesystem on device:
```
cd /usr/local/packages/vdoencodeclient
./vdoencodeclient --format h264 --frames 10 --output vdo.out
```

A file with the output (vdo.out) will be generated application folder, from where it can be extracted.

## License
**Apache License 2.0**
