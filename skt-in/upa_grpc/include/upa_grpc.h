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

/**
 * defines
 */

#define MAX_UPA_GRPC_SERVER_REACTOR 128

/// @brief 메시지 수신 시 호출할 callback function type
/// @param msg read message (upa_grpc::Message*)
/// @param owner 서비스 객체 (server:UpaGrpcServer*, client:UpaGrpcClient*)
/// @param ract reactor 객체 (server:UpaGrpcServerReactorClass*, client:nullptr)
/// @return result code
using UpaGrpcOnReadCallback = int (*)(const void* msg, void* owner, void* ract);

/// @brief server service 객체에서 client 연결 시 호출할 callback function type
/// @param owner server 객체 (UpaGrpcServer*)
/// @param ract reactor 객체 (UpaGrpcServerReactorClass*)
/// @return result code
using UpaGrpcOnAcceptCallback = int (*)(void* owner, void* react);

/// @brief client service 객체에서 server 연결 시 호출할 callback function type
/// @param owner client 객체 (UpaGrpcClient*)
/// @return result code
using UpaGrpcOnConnectCallback = int (*)(void* owner, void*);

/// @brief 서비스 객체에서 peer 연결 해제 시 호출할 callback function type
/// @param owner 서비스 객체 (server:UpaGrpcServer*, client:UpaGrpcClient*)
/// @param ract reactor 객체 (server:UpaGrpcServerReactorClass*, client:nullptr)
/// @return result code
using UpaGrpcOnCloseCallback = int (*)(void* owner, void* ract);

/// @brief client service 객체에서 사용되는 reactor (bidirectional stream 방식)
using UpaGrpcClientReactor =
    grpc::ClientBidiReactor<upa_grpc::Message, upa_grpc::Message>;
class UpaGrpcClientReactorClass;

/// @brief server service 객체에서 사용되는 reactor (bidirectional stream 방식)
using UpaGrpcServerReactor =
    grpc::ServerBidiReactor<upa_grpc::Message, upa_grpc::Message>;
class UpaGrpcServerReactorClass;

/**
 * API of Message struct
 */

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

/**
 * UpaGrpcClient
 */

class UpaGrpcClient {
 public:
  UpaGrpcClient(std::string ip, uint16_t port, std::string name,
                upa_grpc::MsgType msgType, UpaGrpcOnReadCallback onRead)
      : target_(ip + ":" + std::to_string(port)),
        name_(name),
        msg_type_(msgType),
        callback_read_(onRead) {}
  UpaGrpcClient(std::string target, std::string name, upa_grpc::MsgType msgType,
                UpaGrpcOnReadCallback onRead)
      : target_(target),
        name_(name),
        msg_type_(msgType),
        callback_read_(onRead) {}
  ~UpaGrpcClient() { Start(); }

  // start client channel and reactor
  int Start();
  // stop client channel and reactor
  void Stop();
  // start client reactor
  int StartReactor();
  // stop client reactor
  void StopReactor();
  // restart(stop and start) client reactor
  void RestartReactor();
  // check channel status
  int GetState();                       
  // wait for the channel connected
  bool WaitForConnected(int wait_sec);

  const char* GetTarget();
  const char* GetName();
  upa_grpc::MsgType GetMsgType();
  UpaGrpcOnReadCallback GetOnRead();

  UpaGrpcOnConnectCallback GetOnConnect();
  void SetOnConnect(UpaGrpcOnConnectCallback onConnect);

  UpaGrpcOnCloseCallback GetOnClose();
  void SetOnClose(UpaGrpcOnCloseCallback onClose);

  void SetReconnectBackoff(int min, int max);

  void* GetUserData();
  void SetUserData(void* userdata);

  UpaGrpcClientReactorClass* GetReactor();
  int SetReactor(UpaGrpcClientReactorClass* reactor);
  int DeleteReactor();

  int Send(upa_grpc::Message* msg);

 private:
  std::string target_;
  std::string name_;
  upa_grpc::MsgType msg_type_;
  UpaGrpcOnReadCallback callback_read_;
  UpaGrpcOnConnectCallback callback_connect_ = nullptr;
  UpaGrpcOnCloseCallback callback_close_ = nullptr;
  int min_backoff_ = 0;  // min reconnect backoff time (msec)
  int max_backoff_ = 0;  // max reconnect backoff time (msec)
  void *userdata_ = nullptr;
  std::mutex mu_;
  UpaGrpcClientReactorClass* reactor_ = nullptr;
  std::atomic<int> last_channel_state_ = GRPC_CHANNEL_IDLE;
  std::atomic<bool> start_flag_ = false;

  int getState();   
  int deleteReactor();
};

/**
 * UpaGrpcServer
 */
class UpaGrpcServer {
 public:
  UpaGrpcServer(std::string ip, uint16_t port, std::string name,
                upa_grpc::MsgType msgType, UpaGrpcOnReadCallback callback)
      : addr_(ip + ":" + std::to_string(port)),
        name_(name),
        msg_type_(msgType),
        callback_read_(callback) {
    initReactorArray();
  }
  UpaGrpcServer(std::string addr, std::string name, upa_grpc::MsgType msgType,
                UpaGrpcOnReadCallback callback)
      : addr_(addr), name_(name), msg_type_(msgType), callback_read_(callback) {
    initReactorArray();
  }
  ~UpaGrpcServer() { Stop(); }

  // server service 객체를 실행합니다. Stop 매소드가 실행되어 객체가 정지해야 리턴합니다.
  int Run();
  // thread를 생성하고 server Run 실행합니다.
  void Start(); 
  // server를 shutdown 합니다. Start에서 생성된 thread는 종료됩니다.
  void Stop(); 
  // server에 연결된 peer(reactor)를 연결 해제합니다.
  void StopReactor(UpaGrpcServerReactorClass* reactor);
  void StopReactor(std::string reactor_name);
  void StopReactor(int reactor_idx);

  const char* GetAddr();
  const char* GetName();
  upa_grpc::MsgType GetMsgType();
  UpaGrpcOnReadCallback GetOnRead();

  UpaGrpcOnAcceptCallback GetOnAccept();
  void SetOnAccept(UpaGrpcOnAcceptCallback onAccept);

  UpaGrpcOnCloseCallback GetOnClose();
  void SetOnClose(UpaGrpcOnCloseCallback onClose);

  void* GetUserData();
  void SetUserData(void* userdata);

  UpaGrpcServerReactorClass* GetReactor(std::string name);
  UpaGrpcServerReactorClass* GetReactor(int idx);
  int SetReactor(UpaGrpcServerReactorClass* reactor, bool* changed);
  int DeleteReactor(UpaGrpcServerReactorClass* reactor);
  int DeleteReactor(std::string name);
  int DeleteReactor(int idx);
  const char* GetReactorName(UpaGrpcServerReactorClass* reactor);
  const char* GetReactorName(int idx);
  int GetReactorIdx(UpaGrpcServerReactorClass* reactor);
  int GetReactorCount();

  int Send(upa_grpc::Message* msg, UpaGrpcServerReactorClass* reactor);
  int Send(upa_grpc::Message* msg, std::string reactor_name);
  int Send(upa_grpc::Message* msg, int reactor_idx);

 private:
  std::string addr_;
  std::string name_;
  upa_grpc::MsgType msg_type_;
  UpaGrpcOnReadCallback callback_read_;
  UpaGrpcOnAcceptCallback callback_accept_ = nullptr;
  UpaGrpcOnCloseCallback callback_close_ = nullptr;
  void* userdata_ = nullptr;
  std::unique_ptr<grpc::Server> server_ = nullptr;
  std::mutex mu_;

  UpaGrpcServerReactorClass* reactor_array_[MAX_UPA_GRPC_SERVER_REACTOR];
  void initReactorArray();

  int getReactorIdx(UpaGrpcServerReactorClass* reactor);
  UpaGrpcServerReactorClass* getReactor(std::string name);
  UpaGrpcServerReactorClass* getReactor(int idx);
};


#endif  // __UPA_GRPC_H__