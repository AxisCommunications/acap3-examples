 *Copyright (C) 2020, Axis Communications AB, Lund, Sweden. All Rights Reserved.*

# A larod based ACAP3 application running inference on an edge device
This README file explains how to build an ACAP3 application that uses the [larod API](../FAQs.md#WhatisLarod?). It is achieved by using the containerized Axis API and toolchain images.

Together with this README file, you should be able to find a directory called app. That directory contains the "larod_simple_app" application source code which can easily
be compiled and run with the help of the tools and step by step below.

## Getting started
These instructions will guide you on how to execute the code. Below is the structure and scripts used in the example:

```bash
larod
├── app
│   ├── input
│   │   └── veiltail-11457_640_RGB_224x224.bin
│   ├── larod_simple_app.c
│   ├── LICENSE
│   ├── Makefile
│   └── package.conf
├── Dockerfile
├── extract_analyze_output.sh
└── README.md
```

* **Dockerfile** - Docker file with the specified Axis toolchain and API container to build the example specified.
* **app/larod_simple_app.c** - Example application to load a model and run inference on it.
* **app/LICENSE** - Text file which lists all open source licensed source code distributed with the application.
* **app/Makefile** - Makefile containing the build and link instructions for building the ACAP3 application.
* **app/package.conf** - Configuration file containing parameters needed for proper ACAP3 packaging.
* **app/input/veiltail-11457_640_RGB_224x224.bin** - 224x224 raw bitmap image of a goldfish to run inference on.
* **extract_analyze_output.sh** - The script shows the matched class in the output.
* **README.md** - Step by step instructions on how to run the example.

### Limitations
* ARTPEC-7 based device with video support is required
* In order to change the binary name, it has to be done in the Makefile.

### How to run the code
Below is the step by step instructions on how to execute the program. So basically starting with the generation of the .eap file to running it on a device:

#### Build the application
Standing in your working directory run the following commands:

> [!IMPORTANT]
> *Depending on the network you are connected to,
The file that needs those settings is: *~/.docker/config.json.*
For reference please see: https://docs.docker.com/network/proxy/ and a
[script for Axis device here](../FAQs.md#HowcanIset-upnetworkproxysettingsontheAxisdevice?).*

```bash
docker build --tag <APP_IMAGE> .
```

<APP_IMAGE> is the name to tag the image with, e.g., larod-simple-app:1.0

Copy the result from the container image to a local directory build:

```bash
docker cp $(docker create <APP_IMAGE>):/opt/app ./build
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

#### Install your application
Installing your application on an Axis video device is as simple as:

Browse to the following page (replace <axis_device_ip> with the IP number of your Axis video device)

```bash
http://<axis_device_ip>/#settings/apps
```

*Goto your device web page above > Click on the tab **App** in the device GUI > Add **(+)** sign and browse to
the newly built **larod_simple_app_1_0_0_armv7hf.eap** > Click **Install** > Run the application by enabling the **Start** switch*

#### The expected output
In order to see the output, please copy the file with the output (veiltail-11457_640_RGB_224x224.bin.out) from the device into your host
application directory.
> [!IMPORTANT]
*> Please make sure SSH is enabled on the device to run the
following commands.*

```bash
scp root@<axis_device_ip>:/usr/local/packages/larod_simple_app/input/veiltail-11457_640_RGB_224x224.bin.out .
```

Run the script **extract_analyze_output.sh** in the same folder where you copied the output file (veiltail-11457_640_RGB_224x224.bin.out) to see the output.
The script will prompt the password required to access the device.

```bash
extract_analyze_output.sh <axis_device_ip> <Path_to_labels_mobilenet_quant_v1_224.txt>
```

The model class PATH for objects matching is located in the
*/home/user/Workspace/acap3-examples/larod/build/model/labels_mobilenet_quant_v1_224.txt*

The matched class will be printed below:

**Output:** The model has found that with a probability of 100 % the picture represents a goldfish.

> [!NOTE]
> *This app only supports models with one input and one output
tensor, whereas larod itself supports any number of either.*

## License
**[Apache License 2.0](../LICENSE)**
