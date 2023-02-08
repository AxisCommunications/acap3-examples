# What is AXIS Camera Application Platform?

[AXIS Camera Application Platform (ACAP)](https://www.axis.com/support/developer-support/axis-camera-application-platform) is an open application platform that enables members of [Axis Application Development Partner](https://www.axis.com/partners/adp-partner-program) (ADP) Program to develop applications that can be downloaded and installed on Axis network cameras and video encoders. ACAP makes it possible to develop applications for a wide range of use cases:

- Security applications that improve surveillance systems and facilitate investigation.
- Business intelligence applications that improve business efficiency.
- Camera feature plug-ins that add value beyond the Axis product's core functionality

# Prerequisites for ACAP development

ACAP is Axis own open platform for applications that run on-board an Axis product. If you are new to ACAP, start with learning more about the platform, [prerequisites](https://www.axis.com/developer-community/acap-fundamentals), [compatible architectures](https://www.axis.com/developer-community/acap-sdk) and [SDK user manual](https://help.axis.com/acap-3-developer-guide).

# Getting started with the repository

This repository contains a set of application examples which aims to enrich the
developers analytics experience. All examples are using Docker framework and
has a README file in its directory which shows overview, example directory
structure and step-by-step instructions on how to run applications on the
camera.

## Example applications

Below is the list of examples available in the repository.

- [axevent](./axevent/)
  - The example code is written in C which illustrates both how to subscribe to different events and how to send an event.
- [axoverlay](./axoverlay/)
  - The example code is written in C which illustrates how to draw plain boxes and text as overlays in a stream.
- [hello-world](./hello-world/)
  - The example code is written in C and shows how to build a simple hello world application.
- [larod](./larod/)
  - The example code is written in C which connects to [larod](./FAQs.md#WhatisLarod?) and loads a model, runs inference on it and then finally deletes the loaded model from [larod](./FAQs.md#WhatisLarod?).
- [licensekey](./licensekey/)
  - The example code is written in C which illustrates how to check the licensekey status.
- [object-detection](./object-detection/)
  - The example code focus on object detection, cropping and saving detected objects into jpeg files.
- [object-detection-cv25](./object-detection-cv25/)
  - This example is very similar to object-detection, but is designed for AXIS CV25 devices.
- [reproducible-package](./reproducible-package/)
  - An example of how to create a reproducible application package.
- [tensorflow-to-larod](./tensorflow-to-larod/)
  - This example covers model conversion, model quantization, image formats and custom models in
greater depth than the [larod](./larod) and [vdo-larod](./vdo-larod) examples.
- [tensorflow-to-larod-artpec8](./tensorflow-to-larod-artpec8/)
  - This example is very similar to tensorflow-to-larod, but is designed for AXIS ARTPEC-8 devices.
- [tensorflow-to-larod-cv25](./tensorflow-to-larod-cv25/)
  - This example is very similar to tensorflow-to-larod, but is designed for AXIS CV25 devices.
- [using-opencv](./using-opencv/)
  - This example covers how to build, bundle and use OpenCV with ACAP.
- [utility-libraries](./utility-libraries/)
  - These examples covers how to build, bundle and use external libraries with ACAP.
- [vdo-larod](./vdo-larod/)
  - The example code is written in C and loads an image classification model to [larod](./FAQs.md#WhatisLarod?) and then uses vdo to fetch frames of size WIDTH x HEIGHT in yuv format which are converted to interleaved rgb format and then sent to larod for inference on MODEL.
- [vdo-larod-preprocessing](./vdo-larod-preprocessing/)
  - The example code is written in C and loads an image classification model to [larod](./FAQs.md#WhatisLarod?) and then uses vdo to fetch frames of size WIDTH x HEIGHT in yuv format which are sent to larod for preprocessing and inference on MODEL.
- [vdo-opencl-filtering](./vdo-opencl-filtering/)
  - This example illustrates how to capture frames from the vdo service, access the received buffer, and finally perform a GPU accelerated Sobel filtering with OpenCL.
- [vdostream](./vdostream/)
  - The example code is written in C which starts a vdo stream and then illustrates how to continuously capture frames from the vdo service, access the received buffer contents as well as the frame metadata.

## Docker Hub image

The ACAP SDK image can be used as a basis for custom built images to run your
application or as a developer environment inside the container. The image is
public and free to use for anyone.

- [ACAP SDK](https://hub.docker.com/r/axisecp/acap-sdk) This image is based on
  Ubuntu and contains the environment needed for building an AXIS Camera
Application Platform (ACAP) application. This includes all tools for building
and packaging an ACAP 3 application as well as API components (header and
library files) needed for accessing different parts of the camera firmware.

# Long term support (LTS)

Examples for older versions of the ACAP SDK can been found here:

- [Examples for ACAP SDK v3.1](https://github.com/AxisCommunications/acap3-examples/tree/3.1)

# Frequently asked questions

Please visit [FAQs page](FAQs.md) for frequently asked questions.

# License

[Apache 2.0](LICENSE)
