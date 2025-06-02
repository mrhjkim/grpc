# Stage 1: Builder
# This stage compiles your gRPC C++ application.
FROM ubuntu_grpc_dev:22.04 AS builder
ENV MY_INSTALL_DIR=/usr/local

# Build the example
RUN cd examples/cpp/route_guide && \
    mkdir -p cmake/build && \
    cd cmake/build && \
    cmake -DCMAKE_PREFIX_PATH=$MY_INSTALL_DIR ../.. && \
    make -j 4 && \
    cd /go/src/grpc

# Stage 2: Runtime
# This stage creates the final, smaller image with only the runtime necessities.
FROM ubuntu:22.04 AS runtime

# Install only runtime dependencies for gRPC
# libssl is for secure connections, libprotobuf for protobuf messages, libgrpc++ for gRPC itself.
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    libssl3 \
    libprotobuf23 \
    libgrpc++1 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /go/src/grpc/examples/cpp/route_guide/cmake/build/route_guide_server .
COPY --from=builder /go/src/grpc/examples/cpp/route_guide/cmake/build/route_guide_client .
COPY --from=builder /go/src/grpc/examples/cpp/route_guide/cmake/build/route_guide_callback_server .
COPY --from=builder /go/src/grpc/examples/cpp/route_guide/cmake/build/route_guide_callback_client .
COPY --from=builder /go/src/grpc/examples/cpp/route_guide/route_guide_db.json .