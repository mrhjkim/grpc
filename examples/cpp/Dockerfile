# Stage 1: Builder
# This stage compiles your gRPC C++ application.
FROM ubuntu:22.04
ENV MY_INSTALL_DIR=/usr/local

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    autoconf \
    libtool \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /go/src/grpc
COPY . .
# Install gRPC and its dependencies
RUN mkdir -p cmake/build && \
    cd cmake/build && \
    cmake -DgRPC_INSTALL=ON -DgRPC_BUILD_TESTS=OFF -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX=$MY_INSTALL_DIR ../.. && \
    make -j 4 && \
    make install && \
    cd /go/src/grpc
