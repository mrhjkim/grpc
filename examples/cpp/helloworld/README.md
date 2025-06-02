# gRPC C++ Hello World Example

You can find a complete set of instructions for building gRPC and running the
Hello World app in the [C++ Quick Start][].

[C++ Quick Start]: https://grpc.io/docs/languages/cpp/quickstart

### Docker image build from grpc root directory
(먼저 ./examples/cpp 디렉토리의 README 파일을 참고해서 base image를 만들어야 한다.)
docker build -t helloworld:latest -f ./examples/cpp/helloworld/Dockerfile .

### create docker network
docker network create helloworld_network
### server run
docker run --rm --net helloworld_network --name helloworld_server helloworld:latest ./greeter_server
### client run
docker run --rm --net helloworld_network --name helloworld_client helloworld:latest ./greeter_client --target=helloworld_server:50051
