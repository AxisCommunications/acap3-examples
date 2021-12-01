 *Copyright (C) 2021, Axis Communications AB, Lund, Sweden. All Rights Reserved.*

# A guide to building and running openssl and libcurl in an ACAP application

This README file explains how to build openssl and libcurl from source and bundle them for use in an ACAP. The example application uses the openssl and libcurl
libraries to fetch https content from the URL and store the data in the application directory on the Camera.

Together with this README file, you should be able to find a directory called app. That directory contains the "openssl_example" application source code which can easily
be compiled and run with the help of the tools.

APIs specification is available on https://curl.se/libcurl/c, https://www.openssl.org/docs/man1.1.1/man7/

## Getting started

These instructions will guide you on how to execute the code. Below is the structure and scripts used in the example:

```bash
openssl_example
├── app
│   ├── LICENSE
│   ├── Makefile
│   ├── manifest.json
│   └── openssl_example.c
├── Dockerfile
└── README.md
```

* **app/LICENSE**           - File containing the license conditions.
* **app/Makefile**          - Makefile containing the build and link instructions for building the ACAP application.
* **app/manifest.json**     - Defines the application and its configuration.
* **app/openssl_example.c** - Example application.
* **Dockerfile**            - Docker file with the specified Axis toolchain and API container to build the example specified.
* **README.md**             - Step by step instructions on how to run the example.

### How to run the code

Below is the step by step instructions on how to execute the program. So basically starting with the generation of the .eap file to running it on a device:

#### Build the application

Standing in your working directory run the following commands:

> [!IMPORTANT]
> *Depending on the network you are connected to, you may need to add proxy settings.
The file that needs those settings is:* ~/.docker/config.json. *For
reference please see: https://docs.docker.com/network/proxy/ and a
[script for Axis device here](../../FAQs.md#HowcanIset-upnetworkproxysettingsontheAxisdevice?).*

```bash
docker build --tag <APP_IMAGE> .
```

<APP_IMAGE> is the name to tag the image with, e.g., openssl_example:1.0

Default architecture is **armv7hf**. To build for **aarch64** it's possible to
update the *ARCH* variable in the Dockerfile or to set it in the docker build
command via build argument:

```bash
docker build --build-arg ARCH=aarch64 --tag <APP_IMAGE> .
```

If the device is inside a network with a proxy, then it can be passed on as a build argument:

```bash
docker build --build-arg OPENSSL_PROXY=<my_proxy> --tag <APP_IMAGE> .
```

Copy the result from the container image to a local directory build:

```bash
docker cp $(docker create <APP_IMAGE>):/opt/app ./build
```

The working dir now contains a build folder with the following files:

```bash
├── build
│   ├── LICENSE
│   ├── Makefile
│   ├── manifest.json
│   ├── package.conf
│   ├── package.conf.orig
│   ├── param.conf
│   ├── openssl_example*
│   ├── lib
│   │   ├── libssl.so@
│   │   ├── libssl.so.1.1*
│   │   ├── libcrypto.so@
│   │   ├── libcrypto.so.1.1*
│   │   ├── libcurl.so@
│   │   ├── libcurl.so.4@
│   │   └── libcurl.so.4.7.0*
│   ├── openssl_example_1_0_0_armv7hf.eap
│   ├── openssl_example_1_0_0_LICENSE.txt
│   └── openssl_example.c

```

* **build/package.conf** - Defines the application and its configuration.
* **build/package.conf.orig** - Defines the application and its configuration, original file.
* **build/param.conf** - File containing application parameters.
* **build/openssl_example** - Application executable binary file.
* **build/lib** - Folder containing compiled library files for openssl and libcurl.
* **build/openssl_example_1_0_0_armv7hf.eap** - Application package .eap file.
* **build/openssl_example_1_0_0_LICENSE.txt** - Copy of LICENSE file.

#### Install your application

Installing your application on an Axis device is as simple as:

Browse to the following page (replace <axis_device_ip> with the IP number of your Axis device)

```bash
http://<axis_device_ip>/#settings/apps
```

*Goto your device web page above > Click on the tab **App** in the device GUI > Click Add **(+)** sign and browse to
the newly built **openssl_example_1_0_0_armv7hf.eap** > Click **Install** > Run the application by enabling the **Start** switch*

Application openssl_example is now available as an application on the device.

### libopenssl application

The application will use the included CA certificate to fetch the HTTPS content from "https://www.example.com/" and store the content at /usr/local/packages/openssl_example/localdata/https.txt on the camera.

#### The expected output

In this example, when the application is run the content of the URL will be downloaded using HTTPS.

>[!IMPORTANT]
*> Please make sure SSH is enabled on the device to run the following commands.*

The copied URL content can be verified by checking the content of https.txt file.

```bash
ssh root@<axis_device_ip>
cat /usr/local/packages/openssl_example/localdata/https.text
```

Expected output : HTTPS content of the URL("https://www.example.com/") shall be printed.

## License

**[Apache License 2.0](../../LICENSE)**
