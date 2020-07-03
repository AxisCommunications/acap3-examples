 *Copyright (C) 2020, Axis Communications AB, Lund, Sweden. All Rights Reserved.*

# A vdo stream based ACAP3 application on an edge device
This README file explains how to build an ACAP3 application that uses the vdostream API. It is achieved by using the containerized Axis API and toolchain images.

Together with this README file, you should be able to find a directory called app. That directory contains the "vdoencodeclient" application source code which can easily
be compiled and run with the help of the tools and step by step below.

This example illustrates how to continuously capture frames from the vdo service, access the received buffer contents as well as the frame metadata. Captured frames are logged in the Application log.

## Getting started
These instructions will guide you on how to execute the code. Below is the structure and scripts used in the example:

```bash
vdostream
├── app
│   ├── LICENSE
│   ├── Makefile
│   ├── vdoencodeclient.c
├── Dockerfile
└── README.md
```

* **Dockerfile** - Docker file with the specified Axis toolchain and API container to build the example specified.
* **app/LICENSE** - Text file which lists all open source licensed source code distributed with the application.
* **app/Makefile** - Makefile containing the build and link instructions for building the ACAP3 application.
* **app/vdoencodeclient.c** - Application to capture the frames using vdo service in C.
* **README.md** - Step by step instructions on how to run the example.

### Limitations
* The example is done for the armv7hf architecture, but it is possible to update to aarch64 architecture.
* Supported video compression formats for an Axis video device are found in the data-sheet of the device.

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

<APP_IMAGE> is the name to tag the image with, e.g., vdoencodeclient:1.0

Copy the result from the container image to a local directory build:

```bash
docker cp $(docker create <APP_IMAGE>):/opt/app ./build
```

The working dir now contains a build folder with the following files:

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

* **build/package.conf** - Defines the application and its configuration.
* **build/package.conf.orig** - Defines the application and its configuration, original file.
* **build/param.conf** - File containing application parameters.
* **build/vdoencodeclient*** - Application executable binary file.
* **build/vdoencodeclient_1_0_0_armv7hf.eap** - Application package .eap file.
* **build/vdoencodeclient_1_0_0_LICENSE.txt** - Copy of LICENSE file.

#### Install your application
Installing your application on an Axis video device is as simple as:

Browse to the following page (replace <axis_device_ip> with the IP number of your Axis video device)

```bash
http://<axis_device_ip>/#settings/apps
```

*Goto your device web page above > Click on the tab **App** in the device GUI > Add **(+)** sign and browse to
the newly built **vdoencodeclient_1_0_0_armv7hf.eap** > Click **Install** > Run the application by enabling the **Start** switch*

Application will run with default video compression format h264.

#### The expected output
Application log can be found directly at:

```
http://<axis_device_ip>/axis-cgi/admin/systemlog.cgi?appname=vdoencodeclient
```

or by clicking on the "**App log**" link in the device GUI or by using extracting the logs using following commands
in the terminal.
> [!IMPORTANT]
*> Please make sure SSH is enabled on the device to run the
following commands.*

```
ssh root@<axis_device_ip>
cd /var/log/
head -50 info.log
```

```
----- Contents of SYSTEM_LOG for 'vdoencodeclient' -----

2020-06-22T11:00:03.509+02:00 axis-accc8efc9c84 [ INFO    ] vdoencodeclient[17690]: frame = 35729, type = P, size = 81
2020-06-22T11:00:03.532+02:00 axis-accc8efc9c84 [ INFO    ] vdoencodeclient[17690]: frame = 35730, type = I, size = 78433
2020-06-22T11:00:03.572+02:00 axis-accc8efc9c84 [ INFO    ] vdoencodeclient[17690]: frame = 35731, type = P, size = 567
2020-06-22T11:00:03.605+02:00 axis-accc8efc9c84 [ INFO    ] vdoencodeclient[17690]: frame = 35732, type = P, size = 82
2020-06-22T11:00:03.638+02:00 axis-accc8efc9c84 [ INFO    ] vdoencodeclient[17690]: frame = 35733, type = P, size = 74
2020-06-22T11:00:03.672+02:00 axis-accc8efc9c84 [ INFO    ] vdoencodeclient[17690]: frame = 35734, type = P, size = 450
2020-06-22T11:00:03.706+02:00 axis-accc8efc9c84 [ INFO    ] vdoencodeclient[17690]: frame = 35735, type = P, size = 111
2020-06-22T11:00:03.738+02:00 axis-accc8efc9c84 [ INFO    ] vdoencodeclient[17690]: frame = 35736, type = P, size = 76
2020-06-22T11:00:03.772+02:00 axis-accc8efc9c84 [ INFO    ] vdoencodeclient[17690]: frame = 35737, type = P, size = 74
2020-06-22T11:00:03.805+02:00 axis-accc8efc9c84 [ INFO    ] vdoencodeclient[17690]: frame = 35738, type = P, size = 78
2020-06-22T11:00:03.838+02:00 axis-accc8efc9c84 [ INFO    ] vdoencodeclient[17690]: frame = 35739, type = P, size = 78
2020-06-22T11:00:03.872+02:00 axis-accc8efc9c84 [ INFO    ] vdoencodeclient[17690]: frame = 35740, type = P, size = 86
2020-06-22T11:00:03.905+02:00 axis-accc8efc9c84 [ INFO    ] vdoencodeclient[17690]: frame = 35741, type = P, size = 79
2020-06-22T11:00:03.938+02:00 axis-accc8efc9c84 [ INFO    ] vdoencodeclient[17690]: frame = 35742, type = P, size = 78
2020-06-22T11:00:03.972+02:00 axis-accc8efc9c84 [ INFO    ] vdoencodeclient[17690]: frame = 35743, type = P, size = 77
2020-06-22T11:00:04.005+02:00 axis-accc8efc9c84 [ INFO    ] vdoencodeclient[17690]: frame = 35744, type = P, size = 71
2020-06-22T11:00:04.038+02:00 axis-accc8efc9c84 [ INFO    ] vdoencodeclient[17690]: frame = 35745, type = P, size = 82
2020-06-22T11:00:04.072+02:00 axis-accc8efc9c84 [ INFO    ] vdoencodeclient[17690]: frame = 35746, type = P, size = 75
2020-06-22T11:00:04.105+02:00 axis-accc8efc9c84 [ INFO    ] vdoencodeclient[17690]: frame = 35747, type = P, size = 77
2020-06-22T11:00:04.138+02:00 axis-accc8efc9c84 [ INFO    ] vdoencodeclient[17690]: frame = 35748, type = P, size = 80
```

## License
**[Apache License 2.0](../LICENSE)**
