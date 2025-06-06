# gRPC Basics: C++ sample code

The files in this folder are the samples used in [gRPC Basics: C++][],
a detailed tutorial for using gRPC in C++.

[gRPC Basics: C++]:https://grpc.io/docs/languages/cpp/basics

## Configuration

The RouteGuide proto definition is available [here](../../protos/route_guide.proto).
The server takes the following command-line argument -
* db_path - Path to json file containing database. Defaults to `examples/cpp/route_guide/route_guide_db.json` on bazel builds, and `route_guide_db.json` for non-bazel builds.

## Running the example

To run the server -

```
$ tools/bazel run examples/cpp/route_guide:route_guide_callback_server
```

To run the client -

```
$ tools/bazel run examples/cpp/route_guide:route_guide_callback_client
```


### Docker base image build from grpc root directory
(먼저 ./examples/cpp 디렉토리의 README 파일을 참고해서 base image를 만들어야 한다.)
docker build -t routeguide:latest -f ./examples/cpp/route_guide/Dockerfile .

### create docker network
docker network create routeguide_network
### server run
docker run --rm --net routeguide_network --name routeguide_server -p50051:50051 routeguide:latest ./route_guide_callback_server
### client run
docker run --rm --net routeguide_network --name routeguide_client routeguide:latest ./route_guide_callback_client --target=routeguide_server:50051