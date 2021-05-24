# What is AXIS Camera Application Platform?
[AXIS Camera Application Platform (ACAP)](https://www.axis.com/support/developer-support/axis-camera-application-platform) is an open application platform that enables members of [Axis Application Development Partner](https://www.axis.com/partners/adp-partner-program) (ADP) Program to develop applications that can be downloaded and installed on Axis network cameras and video encoders. ACAP makes it possible to develop applications for a wide range of use cases:
* Security applications that improve surveillance systems and facilitate investigation.
* Business intelligence applications that improve business efficiency.
* Camera feature plug-ins that add value beyond the Axis product's core functionality

# Prerequisites for ACAP development
ACAP is Axis own open platform for applications that run on-board an Axis product. If you are new to ACAP, start with learning more about the platform, [prerequisites](https://www.axis.com/developer-community/acap-fundamentals), [compatible architectures](https://www.axis.com/developer-community/acap-sdk) and [SDK user manual](https://www.axis.com/products/online-manual/s00004).

## Getting started with the repo
This repository contains a set of application examples which aims to enrich the
developers analytics experience. All examples are using Docker framework and has a
README file in its directory which shows overview, example directory structure and
step-by-step instructions on how to run applications on the camera.

## Example applications
Below is the list of examples available in the repository.

* [axevent](./axevent/)
  * The example code is written in C which illustrates both how to subscribe to different events and how to send an event.
* [axoverlay](./axoverlay/)
  * The example code is written in C which illustrates how to draw plain boxes and text as overlays in a stream.
* [hello-world](./hello-world/)
  * The example code is written in C and shows how to build a simple hello world application.
* [larod](./larod/)
  * The example code is written in C which connects to [larod](./FAQs.md#WhatisLarod?) and loads a model, runs inference on it and then finally deletes the loaded model from [larod](./FAQs.md#WhatisLarod?).
* [licensekey](./licensekey/)
  * The example code is written in C which illustrates how to check the licensekey status.
* [tensorflow-to-larod](./tensorflow-to-larod/)
  * This example covers model conversion, model quantization, image formats and custom models in
greater depth than the [larod](./larod) example.
* [using-opencv](./using-opencv/)
  * This example covers how to build, bundle and use OpenCV with ACAP3.

### DockerHub Image
The ACAP SDK image can be used as a basis for custom built images to run your application or as a developer environment inside the container. The image is public and free to use for anyone.

* [ACAP SDK](https://hub.docker.com/repository/docker/axisecp/acap-sdk) This image is based on Ubuntu and contains the environment needed for building an AXIS Camera Application Platform (ACAP) application. This includes all tools for building and packaging an ACAP 3 application as well as API components (header and library files) needed for accessing different parts of the camera firmware.

# Frequently asked questions
Please visit [FAQs page](FAQs.md) for frequently asked questions.

# License
[Apache 2.0](LICENSE)

