#include "../include/upa_grpc.h"

#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <thread>

using namespace upa_grpc;

std::string makeHexWithAscii(const void* data, size_t size,
                             std::string* hex_data) {
  char tmp[128];
  const uint8_t* bytePtr = reinterpret_cast<const uint8_t*>(data);

  *hex_data = "";
  for (size_t i = 0; i < size; i += 16) {
    sprintf(tmp, "%04x: ", static_cast<unsigned int>(i));
    hex_data->append(tmp);

    // Hex 출력
    for (size_t j = 0; j < 16; ++j) {
      if (i + j < size) {
        sprintf(tmp, "%02x ", static_cast<unsigned int>(bytePtr[i + j]));
        hex_data->append(tmp);
      } else {
        hex_data->append("   ");
      }
    }

    hex_data->append(" | ");

    // ASCII 출력
    for (size_t j = 0; j < 16 && i + j < size; ++j) {
      char ch = static_cast<char>(bytePtr[i + j]);
      sprintf(tmp, "%c", (std::isprint(ch) ? ch : '.'));
      hex_data->append(tmp);
    }

    hex_data->append("\n");
  }
  return *hex_data;
}

void printHexWithAscii(const void* data, size_t size) {
  std::string hex_data;
  makeHexWithAscii(data, size, &hex_data);
  std::cout << hex_data;
}

// Implementation of the Message struct
const char* MsgTypeStr(int type) { return MsgType_Name(type).c_str(); }
int GetMsgType(const Message* msg) { return static_cast<int>(msg->msg_type()); }
void SetMsgType(Message* msg, int val) {
  msg->set_msg_type(static_cast<MsgType>(val));
}

const char* GetSrcId(const Message* msg) { return msg->src_id().c_str(); }
void SetSrcId(Message* msg, const char* val) { msg->set_src_id(val); }

const char* GetDstId(const Message* msg) { return msg->dst_id().c_str(); }
void SetDstId(Message* msg, const char* val) { msg->set_dst_id(val); }

const char* GetRouteKey(const Message* msg) { return msg->route_key().c_str(); }
void SetRouteKey(Message* msg, const char* val) { msg->set_route_key(val); }

const char* GetSessionKey(const Message* msg) {
  return msg->session_key().c_str();
}
void SetSessionKey(Message* msg, const char* val) { msg->set_session_key(val); }

const char* GetData(const Message* msg, size_t* size) {
  *size = msg->data().size();
  return msg->data().data();
}
void SetData(Message* msg, const char* val, size_t size) {
  msg->set_data(val, size);
}

std::string SprintMessage(const Message* msg, bool is_send) {
  size_t data_size;
  const char* data = GetData(msg, &data_size);
  std::string hex_data;
  std::stringstream buf;
  buf << (is_send ? "Send " : "Receive ") << "UpaGrpcMessage." << std::endl
      << " - message_type   : " << msg->msg_type() << " ("
      << MsgType_Name(msg->msg_type()) << ")" << std::endl
      << " - source_id      : '" << msg->src_id() << "'" << std::endl
      << " - destination_id : '" << msg->dst_id() << "'" << std::endl
      << " - route_key      : '" << msg->route_key() << "'" << std::endl
      << " - session_key    : '" << msg->session_key() << "'" << std::endl
      << " - data           : (" << data_size << ")[" << std::endl
      << makeHexWithAscii(data, data_size, &hex_data) << "]" << std::endl;
  return buf.str();
}

void PrintMessage(const Message* msg, bool is_send) {
  std::cout << SprintMessage(msg, is_send);
}

// Implementation of the UpaGrpcClient class.
class UpaGrpcClientReactor : public UpaGrpcClientReactorClass {
 public:
  explicit UpaGrpcClientReactor(UpaGrpcService::Stub* stub,
                                UpaGrpcClient* owner)
      : stub_(stub), owner_(owner) {
    // Start the RPC; the library will call OnDone/OnReadDone/OnWriteDone
    stub_->async()->SendMessage(&ctx_, this);

    StartRead(&read_msg_);  // Start reading
    StartCall();

    // 연결 상태 변경 시 on connect/close callback 호출된다.
    if(owner_) owner_->GetState(); 
  }

  ~UpaGrpcClientReactor() {
    if (owner_) owner_->StopReactor(false);
  }

  void OnReadDone(bool ok) override {
    if (!ok) {  // Server closed its write side
      return;
    }

    owner_->GetOnRead()(&read_msg_, owner_, nullptr);

    StartRead(&read_msg_);  // Continue reading
  }

  void OnWriteDone(bool /*ok*/) override {
    // write failed or stream closed by server

    // 연결 상태 변경 시 on connect/close callback 호출된다.
    if(owner_) owner_->GetState(); 
  }

  void OnDone(const grpc::Status& status) override {
    // RPC failed or finished cleanly

    // 연결 상태 변경 시 on connect/close callback 호출된다.
    if(owner_) owner_->GetState(); 

    delete this;
  }

 private:
  UpaGrpcService::Stub* stub_;
  UpaGrpcClient* owner_;
  grpc::ClientContext ctx_;
  Message read_msg_;
};

int UpaGrpcClient::Start() {
  if (channel_ != nullptr) {
    std::cout << "Already created channel." << std::endl;
    return 1;
  }

  if (min_backoff_ > 0 || max_backoff_ > 0) {
    grpc::ChannelArguments args;
    if (min_backoff_ > 0) {
      args.SetInt(GRPC_ARG_INITIAL_RECONNECT_BACKOFF_MS, min_backoff_);
      args.SetInt(GRPC_ARG_MIN_RECONNECT_BACKOFF_MS, min_backoff_);
    }
    if (max_backoff_ > 0) {
      args.SetInt(GRPC_ARG_MAX_RECONNECT_BACKOFF_MS, max_backoff_);
    }
    channel_ = grpc::CreateCustomChannel(
        target_, grpc::InsecureChannelCredentials(), args);
  } else {
    channel_ = grpc::CreateChannel(target_, grpc::InsecureChannelCredentials());
  }
  if (channel_ == nullptr) {
    std::cout << "Create channel failed. target(" << target_ << ")"
              << std::endl;
    return -1;
  }
  stub_ = UpaGrpcService::NewStub(channel_);

  return StartReactor();
}

void UpaGrpcClient::Stop() {
  GetState();
  StopReactor(true);
  if (channel_ != nullptr) {
    channel_.reset();
    channel_ = nullptr;
  }
  if (stub_ != nullptr) {
    stub_.reset();
    stub_ = nullptr;
  }
}

int UpaGrpcClient::StartReactor() {
  if (reactor_ != nullptr) {
    std::cout << "Already created reactor." << std::endl;
    return 1;
  }
  if (stub_ == nullptr || channel_ == nullptr) {
    std::cout << "Not created channel." << std::endl;
    return -1;
  }

  reactor_ = new UpaGrpcClientReactor(stub_.get(), this);

  return 0;
}

void UpaGrpcClient::StopReactor(bool sendDoneFlag) {
  if(reactor_ != nullptr) {
    if (sendDoneFlag == true) {
      reactor_->StartWritesDone();  // Inform server we are done sending
    }
    reactor_ = nullptr;
  }
}

/// @brief 채널 연결 상태 조회
/// on connect / close callback 등록된 경우 연결 상태 변경 확인 시 호출한다.
/// @return channel state (GRPC_CHANNEL_IDLE/CONNECTING/READY/FAILURE/SHUTWODN)
int UpaGrpcClient::GetState() {
  int state = GRPC_CHANNEL_IDLE;
  if (channel_) state = static_cast<int>(channel_->GetState(false));

  if (last_channel_state_ != GRPC_CHANNEL_READY &&
      state == GRPC_CHANNEL_READY) {
    if (callback_connect_) callback_connect_(this, nullptr);
  } else if (last_channel_state_ == GRPC_CHANNEL_READY &&
             state != GRPC_CHANNEL_READY) {
    if (callback_close_) callback_close_(this, nullptr);
  }
  last_channel_state_ = state;

  return state;
}

/// @brief server에 연결 상태인지 확인하고 연결될 때까지 대기한다.
/// @param wait_sec 대기시간 (단위:초)
/// @return true: ready 상태, false: 그 외 상태
bool UpaGrpcClient::WaitForConnected(int wait_sec) {
  std::chrono::time_point deadline =
      std::chrono::system_clock::now() + std::chrono::seconds(wait_sec);
  return channel_->WaitForConnected(deadline);
}

/// @brief client 객체 생성 시 등록한 server에 접속할 주소 정보 조회
/// @return target address(ip:port)
const char* UpaGrpcClient::GetTarget() { return target_.c_str(); }

/// @brief client 객체 생성 시 등록한 name 조회
/// @return name
const char* UpaGrpcClient::GetName() { return name_.c_str(); }

/// @brief client 객체 생성 시 등록한 message type 조회
/// @return MsgType
MsgType UpaGrpcClient::GetMsgType() { return msg_type_; }

/// @brief client 객체 생성 시 등록한 on read callback function 조회
/// @return UpaGrpcOnReadCallback
UpaGrpcOnReadCallback UpaGrpcClient::GetOnRead() { return callback_read_; }

/// @brief client 객체에 등록한 on connect callback function 조회
/// @return UpaGrpcOnConnectCallback
UpaGrpcOnConnectCallback UpaGrpcClient::GetOnConnect() {
  return callback_connect_;
}
/// @brief client 객체에 on connect callback function 등록한다.
/// @param onConnect on connect callback function
void UpaGrpcClient::SetOnConnect(UpaGrpcOnConnectCallback onConnect) {
  callback_connect_ = onConnect;
}

/// @brief client 객체에 등록한 on close callback function 조회
/// @return UpaGrpcOnConnectCallback
UpaGrpcOnCloseCallback UpaGrpcClient::GetOnClose() { return callback_close_; }
/// @brief client 객체에 on close callback function 등록한다.
/// @param onClose on close callback function
void UpaGrpcClient::SetOnClose(UpaGrpcOnCloseCallback onClose) {
  callback_close_ = onClose;
}

/// @brief 채널 재접속 시간 옵션을 등록한다.
/// @param min 재접속 최소 시간 (단위:1/1000초)
/// @param max 재접속 최장 시간 (단위:1/1000초)
void UpaGrpcClient::SetReconnectBackoff(int min, int max) {
  if (min >= 0) min_backoff_ = min;
  if (max >= 0) max_backoff_ = max;
}

/// @brief client 객체에 등록한 user data 조회
/// @return user data
void* UpaGrpcClient::GetUserData() { return userdata_; }
/// @brief client 객체에 user data를 등록한다.
/// @param userdata user data
void UpaGrpcClient::SetUserData(void* userdata) { userdata_ = userdata; }

/// @brief server에 메시지를 전송한다.
/// @param msg 전송할 메시지
/// @return result code (0:success, -1:접속 실패 등)
int UpaGrpcClient::Send(Message* msg) {
  if (reactor_ == nullptr) StartReactor();
  if (reactor_ == nullptr) return -1;

  msg->set_msg_type(msg_type_);

  reactor_->StartWrite(msg);

  return 0;
}

// Implementation of the UpaGrpcServer class.
class UpaGrpcServiceImpl final : public UpaGrpcService::CallbackService {
 public:
  UpaGrpcServiceImpl(UpaGrpcServer* owner) : owner_(owner) {}

  UpaGrpcServer* GetUpaGrpcServer() { return owner_; }
  MsgType GetMsgType() { return owner_->GetMsgType(); }
  UpaGrpcOnReadCallback GetOnRead() { return owner_->GetOnRead(); }
  UpaGrpcOnAcceptCallback GetOnAccept() { return owner_->GetOnAccept(); }
  UpaGrpcOnCloseCallback GetOnClose() { return owner_->GetOnClose(); }
  const char* GetReactorName(UpaGrpcServerReactorClass* reactor) {
    return owner_->GetReactorName(reactor);
  }
  int DeleteReactor(std::string name) { return owner_->DeleteReactor(name); }

  // This function is called by gRPC when a new RPC arrives
  UpaGrpcServerReactorClass* SendMessage(
      grpc::CallbackServerContext* context) override {

    class Reactor : public UpaGrpcServerReactorClass {
     public:
      Reactor(grpc::CallbackServerContext* context, UpaGrpcServiceImpl *owner)
          : context_(context), owner_(owner) {
        peer_ = context->peer();
        // Start readind the first message
        StartRead(&read_msg_);
      }
      ~Reactor() {
        UpaGrpcOnCloseCallback onClose = owner_->GetOnClose();
        if (owner_->GetReactorName(this) != "") {
          if (onClose) {
            onClose(owner_->GetUpaGrpcServer(), this);
          } else {
            std::cout << "Closed channel. peer[" << peer_ << "]" << std::endl;
          }
          owner_->DeleteReactor(peer_);
        }
      }

      void OnReadDone(bool ok) override {
        if (!ok) {  // Client closed the write side
          Finish(grpc::Status::OK);
          return;
        }

        // Process incoming message
        if (read_msg_.msg_type() != owner_->GetMsgType()) {
          std::cout << "Invalid type(" << read_msg_.msg_type()
                    << ") message received. drop the message." << std::endl;
          Finish(grpc::Status::CANCELLED);
          return;
        }

        int rv = owner_->GetOnRead()(&read_msg_, owner_->GetUpaGrpcServer(), this);
        if (!rv) {  // Continue reading more messages
          StartRead(&read_msg_);
        } else {
          Finish(grpc::Status::OK);  // Internal error
        }
      }

      void OnWriteDone(bool /*ok*/) override {
        // write failed or stream closed by client
      }

      void OnDone() override {
        // RPC finished
        delete this;
      }

     private:
      grpc::CallbackServerContext* context_;
      UpaGrpcServiceImpl *owner_;
      std::string peer_;
      Message read_msg_;
    };
    UpaGrpcServerReactorClass* reactor = new Reactor(context, this);

    int rv = owner_->SetReactor(reactor, context->peer());
    if (rv < 0) {
      std::cout << "Alreay exists reactor or full reactor array." << std::endl;
      delete reactor;
      return nullptr;
    }

    UpaGrpcOnAcceptCallback onAccept = owner_->GetOnAccept();
    if (onAccept) {
      onAccept(owner_, reactor);
    } else {
      std::cout << "Accepted channel. peer[" << context->peer() << "]"
                << std::endl;
    }

    return reactor;
  }

 private:
  UpaGrpcServer *owner_;
};

int UpaGrpcServer::Run() {
  if (server_ != nullptr) {
    std::cout << "Already started grpc server." << std::endl;
    return 1;
  }

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();

  UpaGrpcServiceImpl serviceImpl(this);

  grpc::ServerBuilder builder;
  builder.AddListeningPort(addr_, grpc::InsecureServerCredentials());
  builder.RegisterService(&serviceImpl);

  server_ = builder.BuildAndStart();
  if (server_ == nullptr) {
    std::cout << "Build and Start grpc server failed. addr(" << addr_ << ")"
              << std::endl;
    return -1;
  }
  std::cout << "Server listening on " << addr_ << std::endl;

  server_->Wait();
  std::cout << "Finish server waiting." << std::endl;
  return 0;
}

void UpaGrpcServer::Start() {
  std::thread t(std::bind(&UpaGrpcServer::Run, this));
  t.detach();
}

void UpaGrpcServer::Stop() {
  if (server_ != nullptr) {
    if (callback_close_) {
      for (int i = 0; i < MAX_UPA_GRPC_SERVER_REACTOR; i++) {
        if(reactor_array_[i] == nullptr) continue;
        callback_close_(this, reactor_array_[i]);
        DeleteReactor(i);
      }
    }
    server_->Shutdown();
    server_->Wait();
    server_.reset();
    server_ = nullptr;

    std::cout << "Server stop." << std::endl;
  }
}

/// @brief server 객체 생성 시 등록한 listen 주소 정보 조회
/// @return listen address(ip:port)
const char* UpaGrpcServer::GetAddr() { return addr_.c_str(); }

/// @brief server 객체 생성 시 등록한 name 조회
/// @return name
const char* UpaGrpcServer::GetName() { return name_.c_str(); }

/// @brief server 객체 생성 시 등록한 message type 조회
/// @return MsgType
MsgType UpaGrpcServer::GetMsgType() { return msg_type_; }

/// @brief server 객체 생성 시 등록한 on read callback function 조회
/// @return UpaGrpcOnReadCallback
UpaGrpcOnReadCallback UpaGrpcServer::GetOnRead() { return callback_read_; }

/// @brief server 객체에 등록한 on accept callback function 조회
/// @return UpaGrpcOnAcceptCallback
UpaGrpcOnAcceptCallback UpaGrpcServer::GetOnAccept() {
  return callback_accept_;
}
/// @brief server 객체에 on accept callback function 등록한다.
/// @param onAccept on accept callback function
void UpaGrpcServer::SetOnAccept(UpaGrpcOnAcceptCallback onAccept) {
  callback_accept_ = onAccept;
};

/// @brief server 객체에 등록한 on close callback function 조회
/// @return UpaGrpcOnCloseCallback
UpaGrpcOnCloseCallback UpaGrpcServer::GetOnClose() { return callback_close_; }
/// @brief server 객체에 on close callback function 등록한다.
/// @param onClose on close callback function
void UpaGrpcServer::SetOnClose(UpaGrpcOnCloseCallback onClose) {
  callback_close_ = onClose;
}

/// @brief server 객체에 등록한 user data 조회
/// @return user data
void* UpaGrpcServer::GetUserData() { return userdata_; }
/// @brief server 객체에 user data를 등록한다.
/// @param userdata user data
void UpaGrpcServer::SetUserData(void* userdata) { userdata_ = userdata; }

void UpaGrpcServer::initReactorArray() {
  for (int i = 0; i < MAX_UPA_GRPC_SERVER_REACTOR; i++) {
    reactor_array_[i] = nullptr;
    reactor_name_array_[i] = "";
  }
}
UpaGrpcServerReactorClass* UpaGrpcServer::GetReactor(std::string name) {
  for (int i = 0; i < MAX_UPA_GRPC_SERVER_REACTOR; i++) {
    if (reactor_array_[i] == nullptr) continue;
    if (reactor_name_array_[i] == "") continue;
    if (reactor_name_array_[i] == name) return reactor_array_[i];
  }
  return nullptr;
}
UpaGrpcServerReactorClass* UpaGrpcServer::GetReactor(int idx) {
  if (idx < 0 || idx >= MAX_UPA_GRPC_SERVER_REACTOR) return nullptr;
  return reactor_array_[idx];
}
int UpaGrpcServer::SetReactor(UpaGrpcServerReactorClass* reactor, std::string name) {
  if (GetReactor(name) != nullptr) return -1;  // Already exists
  for (int i = 0; i < MAX_UPA_GRPC_SERVER_REACTOR; i++) {
    if (reactor_array_[i] == nullptr) {
      reactor_array_[i] = reactor;
      reactor_name_array_[i] = name;
      return i;
    }
  }
  return -1;  // Reactor array fulled
}
int UpaGrpcServer::DeleteReactor(std::string name) {
  for (int i = 0; i < MAX_UPA_GRPC_SERVER_REACTOR; i++) {
    if (reactor_array_[i] == nullptr) continue;
    if (reactor_name_array_[i] == "") continue;
    if (reactor_name_array_[i] == name) {
      reactor_array_[i] = nullptr;
      reactor_name_array_[i] = "";
      return 0;
    }
  }
  return -1; // not found
}
int UpaGrpcServer::DeleteReactor(int idx) {
  if (idx < 0 || idx >= MAX_UPA_GRPC_SERVER_REACTOR) return -1;
  reactor_array_[idx] = nullptr;
  reactor_name_array_[idx] = "";
  return 0;
}

const char* UpaGrpcServer::GetReactorName(UpaGrpcServerReactorClass* reactor) {
  for (int i = 0; i < MAX_UPA_GRPC_SERVER_REACTOR; i++) {
    if (reactor_array_[i] == nullptr) continue;
    if (reactor_array_[i] == reactor) return reactor_name_array_[i].c_str();
  }
  return "";  // not found
}

const char* UpaGrpcServer::GetReactorName(int idx) {
  if (idx < 0 || idx >= MAX_UPA_GRPC_SERVER_REACTOR) return "";
  return reactor_name_array_[idx].c_str();
}

/// @brief reactor를 이용하여 client peer에 메시지를 전송한다.
/// @param msg 전송할 메시지
/// @param reactor client peer 연결 시 생성된 reactor 객체
/// @return result code
int UpaGrpcServer::Send(Message* msg, UpaGrpcServerReactorClass* reactor) {
  if (msg == nullptr || reactor == nullptr) return -1;  // invalid parameters

  const char* name = GetReactorName(reactor);
  if (name == "") return -1;  // not found reactor

  reactor->StartWrite(msg);

  return 0;
}

/// @brief 등록된 reactor를 찾고 이를 이용하여 client peer에 메시지를 전송한다.
/// @param msg 전송할 메시지
/// @param reactor_name server 객체에 등록된 reactor name
/// @return result code
int UpaGrpcServer::Send(Message* msg, std::string reactor_name) {
  if (msg == nullptr || reactor_name == "") return -1;  // invalid parameters

  UpaGrpcServerReactorClass* reactor = GetReactor(reactor_name);
  if (reactor == nullptr) return -1;  // not found reactor

  reactor->StartWrite(msg);

  return 0;
}

/// @brief 등록된 reactor를 찾고 이를 이용하여 client peer에 메시지를 전송한다.
/// @param msg 전송할 메시지
/// @param reactor_idx server 객체제 등록된 reactor index
/// @return result code
int UpaGrpcServer::Send(Message* msg, int reactor_idx) {
  if (msg == nullptr || reactor_idx < 0) return -1;  // invalid parameters

  UpaGrpcServerReactorClass* reactor = GetReactor(reactor_idx);
  if (reactor == nullptr) return -1;  // not found reactor

  reactor->StartWrite(msg);

  return 0;
}
