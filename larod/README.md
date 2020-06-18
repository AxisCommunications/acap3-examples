### An Larod based ACAP3 application running inference on edge device

This readme file explains how to build an ACAP3 that uses the [larod API](../FAQs.md#WhatisLarod?). It is achived by using the containerized Axis API bundle and toolchain.

Together with this file you should be able to find a directory called app, that directory contains the "larod-simple-app.c" application which can easily
be compiled and run with the help of the tools and step by step below.

## Getting started
These instructions below will guide you on how to execute the code. Below is the structure and scripts used in the example:

```bash
larod
├── Dockerfile
│── app
│   ├── larod_simple_app.c
│   ├── LICENSE
|   ├── Makefile
|   ├── package.conf
│   └── input
│       └── veiltail-11457_640_RGB_224x224.bin
└── README.md
```

* **larod_simple_app.c** - Example application to load a model and run inference on it.
* **Dockerfile** - Docker file with the specified Axis toolchain and API container to build the example specified.
* **README.md** - Step by step instructions on how to run the example.
* **app/LICENSE** - File containing the license conditions.
* **app/Makefile** - Makefile containing the build and link instructions for building the ACAP.
* **app/package.conf** - Configuration file containing parameters needed for proper ACAP3 packaging.
* **input/veiltail-11457_640_RGB_224x224.bin** - 224x224 raw bitmap image of a goldfish to run inference on.


### Limitations

ARTPEC-7 based product is required
In order to change the binary name it has to be done in the Makefile

### How to run the code
Below is a step by step on the whole process. So basically starting with the generation of the .eap file to running it on a device:

#### Build and run the application
Standing in your working directory run the following commands:

> [!IMPORTANT]
> *Depending on the network you are connected to,
The file that needs those settings is: *~/.docker/config.json.* 
For reference please see: https://docs.docker.com/network/proxy/ and a
[script for Axis device here](../FAQs.md#HowcanIset-upnetworkproxysettingsontheAxisdevice??).*

```bash
docker build --tag larod-simple-app:1.0 .
```

Copy the result from the container image to a local directory build:

```bash
docker cp $(docker create larod-simple-app:1.0):/opt/app ./build
```

The working dir now contains a build folder with the following files:
```bash
larod
├── Dockerfile
│── app
│   ├── larod_simple_app.c
│   ├── LICENSE
|   ├── Makefile
|   ├── package.conf
|   ├── param.conf
│   └── input
│       └── veiltail-11457_640_RGB_224x224.bin
│── README.md
└── build
	├── input
	│   └── veiltail-11457_640_RGB_224x224.bin
	├── larod_simple_app
	├── larod-simple-app_1_0_0_<ARCHITECTURE>.eap
	├── larod-simple-app_1_0_0_LICENSE.txt
	├── larod_simple_app.c
	├── LICENSE
	├── Makefile
	├── model
	│	├── labels_mobilenet_quant_v1_224.txt
	│	├── mobilenet_v1_1.0_224_quant.larod
	│	├── mobilenet_v1_1.0_224_quant.tflite
	│	├── mobilenet_v1_tflite.zip
	│	└── __MACOSX
	│── package.conf
	├── package.conf.orig
	├── param.conf
	└── README.md
```

#### Install your application on an Axis video product
Installing your application on an Axis video product is as simple as:

Browse to the following page (replace <axis_device_ip> with the IP number of your Axis video product)

```bash
http://<axis_device_ip>/#settings/apps
```

*Goto your device web page above > Click on the tab App in the device GUI > Add **(+)** sign and browse to 
the newly larod_simple_app_1_0_0_armv7hf.eap > Click Install > Run the application by enabling the Start switch*

#### The expected output:

 In order to see the output, please copy the file with the output (veiltail-11457_640_RGB_224x224.bin.out) from device into your host
 application directory. You need to enable SSH on the device before executing the following command. 

```bash
scp root@<axis_device_ip>:/usr/local/packages/larod_simple_app/input/veiltail-11457_640_RGB_224x224.bin.out . 
```
Run the following command in the same folder where you copied the output file (veiltail-11457_640_RGB_224x224.bin.out):

```bash
od -A d -t u1 -v -w1 <name of output file> | sort -n -k 2
```
The highest matched classes will be on the bottom of the list that's printed.
You can match these classes with:

```bash
/usr/local/packages/larod_simple_app/model/labels_mobilenet_quant_v1_224.txt
```
to see that indeed a goldfish was recognized.

> [!NOTE]
> *This app only supports models with one input and one output
tensor, whereas larod itself supports any number of either.*

## License

**Apache 2.0**