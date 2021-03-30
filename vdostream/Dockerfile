# Specify the architecture at build time: armv7hf/aarch64
# Should be used for getting API image
# Currently, only armv7hf is supported
ARG ARCH=armv7hf
ARG VERSION=3.3_beta1
ARG UBUNTU_VERSION=20.04
ARG REPO=axisecp

FROM ${REPO}/acap-sdk:${VERSION}-${ARCH}-ubuntu${UBUNTU_VERSION}

# Building the ACAP application
COPY ./app /opt/app/
WORKDIR /opt/app
RUN . /opt/axis/acapsdk/environment-setup* && acap-build .
