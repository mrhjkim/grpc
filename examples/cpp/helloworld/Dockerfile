# Stage 1: Builder
# This stage compiles your gRPC C++ application.
FROM ubuntu_grpc_dev:22.04 AS builder
ENV MY_INSTALL_DIR=/usr/local

# Build the example
RUN cd examples/cpp/helloworld && \
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

# Copy the compiled application binary from the builder stage
# Replace 'your_grpc_server_executable' with the actual name of your executable
COPY --from=builder /go/src/grpc/examples/cpp/helloworld/cmake/build/greeter_server .
COPY --from=builder /go/src/grpc/examples/cpp/helloworld/cmake/build/greeter_client .
COPY --from=builder /go/src/grpc/examples/cpp/helloworld/cmake/build/greeter_async_client .
COPY --from=builder /go/src/grpc/examples/cpp/helloworld/cmake/build/greeter_async_client2 .
COPY --from=builder /go/src/grpc/examples/cpp/helloworld/cmake/build/greeter_async_server .
COPY --from=builder /go/src/grpc/examples/cpp/helloworld/cmake/build/greeter_callback_client .
COPY --from=builder /go/src/grpc/examples/cpp/helloworld/cmake/build/greeter_callback_server .

# Command to run your gRPC server
# ENTRYPOINT ["./greeter_server"]