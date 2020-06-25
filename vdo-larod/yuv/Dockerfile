ARG http_proxy=http://wwwproxy.se.axis.com:3128
ARG https_proxy=http://wwwproxy.se.axis.com:3128
ARG UBUNTU_VERSION

FROM arm32v7/ubuntu:${UBUNTU_VERSION}

ARG libyuv_version=5b6042fa0d211ebbd8b477c7f3855977c7973048

RUN apt-get update && apt-get install -y --no-install-recommends \
    git \
    make \
    g++

WORKDIR /opt/build

# TODO: Investigate why server certs can't be verified
RUN GIT_SSL_NO_VERIFY=1 git clone -n https://chromium.googlesource.com/libyuv/libyuv

RUN cd libyuv && \
    git checkout ${libyuv_version}

WORKDIR /opt/build/libyuv

COPY *.patch .

RUN git apply *.patch

RUN CXXFLAGS=' -O2 -mthumb -mfpu=neon -mfloat-abi=hard -mcpu=cortex-a9 -fomit-frame-pointer' \
    make -f linux.mk

RUN strip --strip-unneeded libyuv.so*

RUN mkdir -p /yuv/lib &&\
    mkdir -p /yuv/include &&\
    cp libyuv.so* /yuv/lib &&\
    cp -a include/. /yuv/include &&\
    cd /yuv/lib &&\
    ln -s libyuv.so.1 libyuv.so &&\
    ln -s libyuv.so.1 libyuv.so.1.0







