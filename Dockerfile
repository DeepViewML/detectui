ARG BUILD_TYPE=Release

FROM --platform=linux/arm64/v8 maivin/gstreamer:bullseye AS source

RUN curl https://apt.kitware.com/keys/kitware-archive-latest.asc | gpg --dearmor - > /usr/share/keyrings/kitware-archive-keyring.gpg
RUN echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ bionic main' > /etc/apt/sources.list.d/kitware.list

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git-core \
    ninja-build \
    pkg-config \
    libxkbcommon-dev \
    libgstreamer1.0-dev \
    libsoup2.4-dev \
    libgstreamer-plugins-base1.0-dev \
    libdrm-dev \
    libdeepview-rt-dev \
    libvaal-dev \
    libvideostream-dev

WORKDIR /src
COPY . .
RUN mkdir copy_mount


# Build Base vpkui library common to all applications.
# FROM --platform=linux/arm64/v8 source AS vpkui
RUN cmake -Bbuild -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
RUN cmake --build build --parallel 8 --target facedetectgl
# COPY --from=vpkui /src/build/lib/* ./build/lib

ENTRYPOINT bash

# # Facedetect Cleanup Application
# # This is the bare minimum code cleanned up without the Nuklear noise.
# FROM --platform=linux/arm64/v8 vpkui AS facedetectgl
# RUN cmake --build build --parallel 8 --target facedetectgl
# FROM --platform=linux/arm64/v8 vpkui AS facedetectgl_headless
# RUN cmake --build build --parallel 8 --target facedetectgl_headless
# FROM --platform=linux/arm64/v8 maivin/gstreamer:bullseye AS facedetect
# WORKDIR /app
# COPY --from=facedetectgl_headless /src/build/samples/facedetectgl_headless .
# COPY --from=facedetectgl_headless /usr/lib/aarch64-linux-gnu/libvaal.so /usr/lib/aarch64-linux-gnu/
# ENTRYPOINT ["/app/facedetectgl_headless", "-d", "/dev/video0", "--mirror", "--mirror_v"]

# # Bodypose Cleanup Application
# FROM --platform=linux/arm64/v8 vpkui AS bodyposegl
# RUN cmake --build build --parallel 8 --target bodyposegl
# FROM --platform=linux/arm64/v8 vpkui AS bodyposegl_headless
# RUN cmake --build build --parallel 8 --target bodyposegl_headless
# FROM --platform=linux/arm64/v8 maivin/gstreamer:bullseye AS bodypose
# WORKDIR /app
# COPY --from=bodyposegl_headless /src/build/samples/bodyposegl_headless .
# COPY --from=bodyposegl_headless /usr/lib/aarch64-linux-gnu/libvaal.so /usr/lib/aarch64-linux-gnu/
# ENTRYPOINT ["/app/bodyposegl_headless", "-d", "/dev/video0", "--mirror", "--mirror_v"]

# # Detection Cleanup Application
# FROM --platform=linux/arm64/v8 vpkui AS detectiongl
# RUN cmake --build build --parallel 8 --target detectiongl
# FROM --platform=linux/arm64/v8 vpkui AS detectiongl_headless
# RUN cmake --build build --parallel 8 --target detectiongl_headless
# FROM --platform=linux/arm64/v8 maivin/gstreamer:bullseye AS detection
# WORKDIR /app
# COPY --from=detectiongl_headless /src/build/samples/detectiongl_headless .
# COPY --from=detectiongl_headless /usr/lib/aarch64-linux-gnu/libvaal.so /usr/lib/aarch64-linux-gnu/
# ENTRYPOINT ["/app/detectiongl_headless", "-d", "/dev/video0", "--mirror", "--mirror_v"]

# # Faceblur Cleanup Application
# # This example shows how we can combine headless and wayland in the same code.
# FROM --platform=linux/arm64/v8 vpkui AS faceblurgl
# RUN cmake --build build --parallel 8 --target faceblurgl
# FROM --platform=linux/arm64/v8 vpkui AS faceblurgl_headless
# RUN cmake --build build --parallel 8 --target faceblurgl_headless
# FROM --platform=linux/arm64/v8 maivin/gstreamer:bullseye AS faceblur
# WORKDIR /app
# COPY --from=faceblurgl_headless /src/build/samples/faceblurgl_headless .
# COPY --from=faceblurgl_headless /usr/lib/aarch64-linux-gnu/libvaal.so /usr/lib/aarch64-linux-gnu/
# ENTRYPOINT ["/app/faceblurgl_headless", "-d", "/dev/video0", "--mirror", "--mirror_v"]

# # headpose Cleanup Application
# FROM --platform=linux/arm64/v8 vpkui AS headposegl
# RUN cmake --build build --parallel 8 --target headposegl
# FROM --platform=linux/arm64/v8 vpkui AS headposegl_headless
# RUN cmake --build build --parallel 8 --target headposegl_headless
# FROM --platform=linux/arm64/v8 maivin/gstreamer:bullseye AS headpose
# WORKDIR /app
# COPY --from=headposegl_headless /src/build/samples/headposegl_headless .
# COPY --from=headposegl_headless /usr/lib/aarch64-linux-gnu/libvaal.so /usr/lib/aarch64-linux-gnu/
# ENTRYPOINT ["/app/headposegl_headless", "-d", "/dev/video0", "--mirror", "--mirror_v"]

# # segmentation Cleanup Application
# FROM --platform=linux/arm64/v8 vpkui AS segmentationgl
# RUN cmake --build build --parallel 8 --target segmentationgl
# FROM --platform=linux/arm64/v8 vpkui AS segmentationgl_headless
# RUN cmake --build build --parallel 8 --target segmentationgl_headless
# FROM --platform=linux/arm64/v8 maivin/gstreamer:bullseye AS segmentation
# WORKDIR /app
# COPY --from=segmentationgl_headless /src/build/samples/segmentationgl_headless .
# COPY --from=segmentationgl_headless /usr/lib/aarch64-linux-gnu/libvaal.so /usr/lib/aarch64-linux-gnu/
# ENTRYPOINT ["/app/segmentationgl_headless", "-d", "/dev/video0", "--mirror", "--mirror_v"]