
# What is Axis Camera Application Platform?
[AXIS Camera Application Platform (ACAP)](https://www.axis.com/support/developer-support/axis-camera-application-platform) is an open application platform that enables members of [Axis Application Development Partner](https://www.axis.com/partners/adp-partner-program) (ADP) Program to develop applications that can be downloaded and installed on Axis network cameras and video encoders. ACAP makes it possible to develop applications for a wide range of applications:
* Security applications that improve surveillance systems and facilitate investigation.
* Business intelligence applications that improve business efficiency.
* Camera feature plug-ins that add value beyond the Axis product's core functionality


# Getting started
This repository contains a set of application examples which aims to enrich the
developers analytics experience. All examples are using Docker framework and has a
README file in its directory which shows overview, example directory structure and
step-by-step instructions on how to run applications on the camera.

## Requirements


## Supported architectures

## Example applications for video analytics
Below is the list of examples available in the respository.

* [Larod](./larod/)
  * The example code is written in C which starts a vdo stream and then illustrates how to continuously capture frames from the vdo service, access the received buffer contents as well as the frame metadata.
* [vdostream](./vdostream/)
  * The example code is written in C which connects to larod and loads a model, runs inference on it and then finally deletes the loaded model from larod.

### DockerHub Images
There are two types of Docker images here: the ToolChain (SDK), and the API 3.1. These images can be used as the basis for custom built images for running your applications. The images needed are specified in the docker-compose files. All images are public and free to use for anyone.
* [Toolchain](https://hub.docker.com/repository/docker/axisecp/acap-toolchain) -  An Ubuntu-based Toolchain bundle with all tools for building and packaging an ACAP 3.1 application included.
* [API 3.1](https://hub.docker.com/repository/docker/axisecp/acap-api) - An Ubuntu-based API bundle with all API components (header and library files) included.


# How to work with Github repository
You can help to make this repo a better one using the following commands.

1. Fork it (git checkout ..)
2. Create your feature branch: git checkout -b <contr/my-new-feature>
3. Commit your changes: git commit -a
4. Push to the branch: git push origin <contr/my-new-feature>
5. Submit a pull request


# License
[Apache 2.0](LICENSE)

