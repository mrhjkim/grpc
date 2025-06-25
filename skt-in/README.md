# gRPC SKT-IN Interface Library

SCPAS/SRG 등 외부 노드와 OCS 내부 Pod 간의 통신에  gRPC 방식을 사용할 것으로 보입니다.
기존 프로세스는 대부분 C언어로 개발되었으나 gRPC는 Go, C++ 등의 언어를 지원하지만
C언어는 지원하지 않기 때문에 직접적으로 gRPC 기능 개발이 불가능한 상황입니다.
이에 C언어에서 호환 가능한 Library를 구현했습니다.

## C++ 를 기반으로 gRPC API 구현

- Async Callback 방식으로 Server / Client Class 구성
- 메시지 receive/send 부분은 기 구현되어있고 메시지 처리 부분을 app에서 callback 형태로 전달하여 직접 처리하도록 구조 설계했습니다.

## C 코드에서도 호환 될 수 있도록 C API Wrapper 추가 구현

- Class 를 void* 형태의 handler로 type cast하여 C언어 기반 app에서 관리하도록 합니다.
- function, field 를 wrapping한 함수를 제공하여 C언어 기반 app에서 호출하여 필드를 핸들링하고 함수를 호출할 수 있도록 구조 설계했습니다.

## 테스트 방법

### build
cd cmake/build
cmake -DCMAKE_PREFIX_PATH=$MY_INSTALL_DIR ../..
make

### test server 실행
C++ 버전: ./test_server_c++ [--ip=IP] [--port=PORT]
C 버전  : ./test_server_c [-i IP] [-p PORT]

### test client 실행
C++ 버전 : ./test_client_c++ [--target=IP:PORT]
C 버전   : ./test_client_c [-i IP] [-p PORT]
