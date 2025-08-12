#ifndef __UPA_GRPC_H__
#define __UPA_GRPC_H__

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/support/server_callback.h>

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

#ifdef BAZEL_BUILD
#include "protos/upa_grpc.grpc.pb.h"
#else
#include "upa_grpc.grpc.pb.h"
#endif

using UpaGrpcClientCallback = void (*)(void*); // read_msg
using UpaGrpcServerCallback = int (*)(void*, const void*, void*); // context, read_msg, reactor
using UpaGrpcClientReactorClass = grpc::ClientBidiReactor<upa_grpc::Message, upa_grpc::Message>;
using UpaGrpcServerReactorClass = grpc::ServerBidiReactor<upa_grpc::Message, upa_grpc::Message>;

// API of Message struct
const char* MsgTypeStr(int type);
int GetMsgType(const upa_grpc::Message* msg);
void SetMsgType(upa_grpc::Message* msg, int val);
const char* GetSrcId(const upa_grpc::Message* msg);
void SetSrcId(upa_grpc::Message* msg, const char* val);
const char* GetDstId(const upa_grpc::Message* msg);
void SetDstId(upa_grpc::Message* msg, const char* val);
const char* GetRouteKey(const upa_grpc::Message* msg);
void SetRouteKey(upa_grpc::Message* msg, const char* val);
const char* GetSessionKey(const upa_grpc::Message* msg); 
void SetSessionKey(upa_grpc::Message* msg, const char* val);
const char* GetData(const upa_grpc::Message* msg, size_t* size); 
void SetData(upa_grpc::Message* msg, const char* val, size_t size); 
std::string SprintMessage(const upa_grpc::Message* msg, bool is_send);
void PrintMessage(const upa_grpc::Message* msg, bool is_send);

// UpaGrpcClient
class UpaGrpcClient {
 public:
  UpaGrpcClient(std::string ip, uint16_t port, upa_grpc::MsgType msgType,
                UpaGrpcClientCallback callback)
      : target_(ip + ":" + std::to_string(port)),
        msg_type_(msgType),
        callback_(callback) {}
  UpaGrpcClient(std::string target, upa_grpc::MsgType msgType,
                UpaGrpcClientCallback callback)
      : target_(target), msg_type_(msgType), callback_(callback) {}
  ~UpaGrpcClient() { Start(); }

  int Start(); // start client channel and reactor
  void Stop(); // stop client channel and reactor
  int StartReactor(); // start client reactor
  void StopReactor(bool sendDoneFlag); // stop client reactor
  int GetState(); // check channel status
  bool WaitForConnected(int wait_sec);
  void SetReconnectBackoff(int min, int max);

  int Send(upa_grpc::Message* msg);

 private:
  std::string target_;
  upa_grpc::MsgType msg_type_;
  UpaGrpcClientCallback callback_;
  int min_backoff_;  // min reconnect backoff time (msec)
  int max_backoff_;  // max reconnect backoff time (msec)
  std::shared_ptr<grpc::Channel> channel_ = nullptr;
  std::unique_ptr<upa_grpc::UpaGrpcService::Stub> stub_ = nullptr;
  UpaGrpcClientReactorClass* reactor_ = nullptr;
};

// UpaGrpcServer
class UpaGrpcServer {
 public:
  UpaGrpcServer(std::string ip, uint16_t port, upa_grpc::MsgType msgType,
                UpaGrpcServerCallback callback)
      : addr_(ip + ":" + std::to_string(port)),
        msg_type_(msgType),
        callback_(callback) {}
  UpaGrpcServer(std::string addr, upa_grpc::MsgType msgType,
                UpaGrpcServerCallback callback)
      : addr_(addr), msg_type_(msgType), callback_(callback) {}
  ~UpaGrpcServer() { Stop(); }

  int Run();
  void Start();  // thread를 생성하고 server run 실행 시킨다.
  void Stop();   // server를 shutdown한다. start에서 생성된 thread는 종료된다.

 private:
  std::string addr_;
  upa_grpc::MsgType msg_type_;
  UpaGrpcServerCallback callback_;
  std::unique_ptr<grpc::Server> server_ = nullptr;
};

int UpaGrpcServerSend(UpaGrpcServerReactorClass* reactor,
                      upa_grpc::Message* msg);
// TODO : UpaGrpcServer class에서 reactor를 client channel과 같이 관리하고 peer
// key를 통해 조회할 수 있도록 구조개선해야겠다.

#endif  // __UPA_GRPC_H__