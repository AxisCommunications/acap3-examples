# Specify the architecture at build time: armv7hf
# Should be used for getting API and toolchain image
# Currently, only armv7hf is supported
ARG http_proxy
ARG https_proxy
ARG ARCH=armv7hf
ARG VERSION=3.1
ARG UBUNTU_VERSION=19.10

FROM builder-yuv:1.0-ubuntu${UBUNTU_VERSION} as yuv
FROM axisecp/acap-api:${VERSION}-${ARCH}-ubuntu${UBUNTU_VERSION} as api
FROM axisecp/acap-toolchain:${VERSION}-${ARCH}-ubuntu${UBUNTU_VERSION} as toolchain

ARG PKG_ARCH=cortexa9hf-neon-poky-linux-gnueabi

# Copy the lib, include and .pc files from the API container
COPY --from=api /opt/axis/sdk/temp/sysroots/${PKG_ARCH}/usr/lib/ /opt/axis/sdk/sysroots/${PKG_ARCH}/usr/lib/
COPY --from=api /opt/axis/sdk/temp/sysroots/${PKG_ARCH}/usr/include/ /opt/axis/sdk/sysroots/${PKG_ARCH}/usr/include/
COPY --from=api /opt/axis/sdk/temp/sysroots/${PKG_ARCH}/usr/lib/pkgconfig/ /opt/axis/sdk/sysroots/${PKG_ARCH}/usr/lib/pkgconfig/

# Copy the tools and scripts from the toolchain container
RUN cp -r /opt/axis/sdk/temp/* /opt/axis/sdk/

# Copy needed files from the yuv container
COPY --from=yuv /yuv/lib /opt/axis/sdk/sysroots/${PKG_ARCH}/usr/lib/
COPY --from=yuv /yuv/include /opt/axis/sdk/sysroots/${PKG_ARCH}/usr/include/

# Install any non-axis package that your build process/ACAP needs
RUN apt-get update && apt-get install -y --no-install-recommends \
        curl

# The building of the particular ACAP
COPY ./app /opt/app/
WORKDIR /opt/app

# Creating package
RUN . /opt/axis/sdk/environment-setup* && create-package.sh
