#ifndef __UPA_GRPC_H__
#define __UPA_GRPC_H__

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

#ifdef BAZEL_BUILD
#include "protos/upa_grpc.grpc.pb.h"
#else
#include "upa_grpc.grpc.pb.h"
#endif

using UpaGrpcClientCallback = void (*)(int, void*);
using UpaGrpcServerCallback = int (*)(const void*, void*);

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
class UpaGrpcClientContext {
 public:
  grpc::ClientContext base;

  UpaGrpcClientContext(int wait_sec) {
    if (wait_sec > 0 && wait_sec <= 3600) {
      base.set_deadline(std::chrono::system_clock::now() +
                        std::chrono::seconds(wait_sec));
    }
  }
  ~UpaGrpcClientContext() {}

  int Wait();
  void Done(grpc::StatusCode result);

 private:
  std::mutex mu_;
  std::condition_variable cv_;
  bool done_ = false;
  grpc::StatusCode result_;
  };

class UpaGrpcClient {
 public:
  UpaGrpcClient(std::string ip, uint16_t port, upa_grpc::MsgType msgType)
      : target_(ip + ":" + std::to_string(port)), msg_type_(msgType) {}
  UpaGrpcClient(std::string target, upa_grpc::MsgType msgType)
      : target_(target), msg_type_(msgType) {}
  ~UpaGrpcClient() { Start(); }

  int Start();
  void Stop();
  int GetState();
  bool WaitForConnected(int wait_sec);

  int Send(UpaGrpcClientContext* context, upa_grpc::Message* request,
           upa_grpc::Message* response, UpaGrpcClientCallback callback);

 private:
  std::string target_;
  upa_grpc::MsgType msg_type_;
  std::shared_ptr<grpc::Channel> channel_ = nullptr;
  std::unique_ptr<upa_grpc::UpaGrpcService::Stub> stub_ = nullptr;
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
  void Start(); // thread를 생성하고 server run 실행 시킨다.
  void Stop(); // server를 shutdown한다. start에서 생성된 thread는 종료된다.

 private:
  std::string addr_;
  upa_grpc::MsgType msg_type_;
  UpaGrpcServerCallback callback_;
  std::unique_ptr<grpc::Server> server_ = nullptr;
};

#endif  // __UPA_GRPC_H__