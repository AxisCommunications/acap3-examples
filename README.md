# What is Axis Camera Application Platform?
[AXIS Camera Application Platform (ACAP)](https://www.axis.com/support/developer-support/axis-camera-application-platform) is an open application platform that enables members of [Axis Application Development Partner](https://www.axis.com/partners/adp-partner-program) (ADP) Program to develop applications that can be downloaded and installed on Axis network cameras and video encoders. ACAP makes it possible to develop applications for a wide range of use cases:
* Security applications that improve surveillance systems and facilitate investigation.
* Business intelligence applications that improve business efficiency.
* Camera feature plug-ins that add value beyond the Axis product's core functionality

# Prerequisites for ACAP development
ACAP is Axis own open platform for applications that run on-board an Axis product. If you are new to ACAP, start with learning more about the platform, [prerequisites](https://www.axis.com/developer-community/acap-fundamentals), [compatible architectures](https://www.axis.com/developer-community/acap-sdk) and [SDK user manual](https://www.axis.com/products/online-manual/s00001#t10152931).

## Getting started with the repo
This repository contains a set of application examples which aims to enrich the
developers analytics experience. All examples are using Docker framework and has a
README file in its directory which shows overview, example directory structure and
step-by-step instructions on how to run applications on the camera.

## Example applications
Below is the list of examples available in the repository.

* [axevent](./axevent/)
  * The example code is written in C which illustrates both how to subscribe to different events and how to send an event.
* [Larod](./larod/)
  * The example code is written in C which connects to [larod](./FAQs.md#WhatisLarod?) and loads a model, runs inference on it and then finally deletes the loaded model from [larod](./FAQs.md#WhatisLarod?).
* [licensekey](./licensekey/)
  * The example code is written in C which illustrates how to check the licensekey status.
* [vdostream](./vdostream/)
  * The example code is written in C which starts a vdo stream and then illustrates how to continuously capture frames from the vdo service, access the received buffer contents as well as the frame metadata.
* [vdo-larod](./vdo-larod/)
  * The example code is written in C and loads an image classification model to [larod](./FAQs.md#WhatisLarod?) and then uses vdo to fetch frames of size WIDTH x HEIGHT in yuv format which are converted to interleaved rgb format and then sent to larod for inference on MODEL.
* [Tensorflow_Larod](./tensorflow-to-larod/)
  * This example covers model conversion, model quantization, image formats and models with multiple output tensors in
greater depth than the [larod](https://github.com/AxisCommunications/acap3-examples/tree/master/larod)
and [vdo-larod](https://github.com/AxisCommunications/acap3-examples/tree/master/vdo-larod) examples.

### DockerHub Images
There are two types of Docker images here: the toolchain (SDK), and the API. These images can be used as the basis for custom built images for running your applications. The images needed are specified in the docker-compose files. All images are public and free to use for anyone.
* [Toolchain](https://hub.docker.com/repository/docker/axisecp/acap-toolchain) -  An Ubuntu-based toolchain bundle with all tools for building and packaging an ACAP 3 application included.
* [API](https://hub.docker.com/repository/docker/axisecp/acap-api) - An Ubuntu-based API bundle with all API components (header and library files) included.

 # Frequently asked questions
Please visit [FAQs page](FAQs.md)  for frequently asked questions.

# License
[Apache 2.0](LICENSE)

