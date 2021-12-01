ARG ARCH=armv7hf
ARG VERSION=3.4.2
ARG UBUNTU_VERSION=20.04
ARG REPO=axisecp
ARG SDK=acap-sdk

FROM ${REPO}/${SDK}:${VERSION}-${ARCH}-ubuntu${UBUNTU_VERSION}

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    autoconf \
    libtool \
    automake && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /opt/build
RUN git clone https://github.com/curl/curl.git

# Curl lib generate
# - Source the SDK environment script to get links to cross compiler through
# - CONFIGURE_FLAGS environment variable
WORKDIR /opt/build/curl
# CONFIGURE_FLAGS need to be split
# hadolint ignore=SC2086
RUN . /opt/axis/acapsdk/environment-setup* && \
    autoreconf -fi && \
    ./configure --prefix="${PWD}"/build ${CONFIGURE_FLAGS} --with-openssl && \
    make && \
    make install

# Delete manual directory from curl built result
RUN rm -rf /opt/build/curl/build/share/man

# Switch to build directory
WORKDIR /opt/build

# Clone openssl and extract source code
RUN curl -O https://www.openssl.org/source/openssl-1.1.1l.tar.gz && \
    tar xzvf openssl-1.1.1l.tar.gz

# Openssl lib generate
# - Source the SDK environment script to get links to cross compiler through
# - CONFIGURE_FLAGS environment variable
WORKDIR /opt/build/openssl-1.1.1l

RUN ./Configure linux-armv4 no-asm shared --prefix="${PWD}"/build --cross-compile-prefix=/usr/bin/ && \
    make && \
    make install

# Delete manual directory from openssl built result
RUN rm -rf /opt/build/openssl-1.1.1l/build/share/man

# Copy the library to application folder
ARG BUILDDIR_CURL=/opt/build/curl/build
ARG BUILDDIR_OPENSSL=/opt/build/openssl-1.1.1l/build
COPY ./app /opt/app/
WORKDIR /opt/app
RUN mkdir lib && cp -r ${BUILDDIR_CURL}/lib/libcurl.so* lib && \
    cp -r ${BUILDDIR_OPENSSL}/lib/libssl.so* lib && \
    cp -r ${BUILDDIR_OPENSSL}/lib/libcrypto.so* lib

# Proxy setting for curl
ARG OPENSSL_PROXY
ENV OPENSSL_PROXY ${OPENSSL_PROXY}

# Building the ACAP application
RUN . /opt/axis/acapsdk/environment-setup* && acap-build ./
