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
                                UpaGrpcClientCallback callback, UpaGrpcClient* owner)
      : stub_(stub), callback_(callback), owner_(owner) {
    // Start the RPC; the library will call OnDone/OnReadDone/OnWriteDone
    stub_->async()->SendMessage(&ctx_, this);

    StartRead(&read_msg_);  // Start reading
    StartCall();
  }
  ~UpaGrpcClientReactor() {
    if( owner_) owner_->StopReactor(false);
  }

  void OnReadDone(bool ok) override {
    if (!ok) {  // Server closed its write side
      return;
    }

    callback_(&read_msg_);

    StartRead(&read_msg_);  // Continue reading
  }

  void OnWriteDone(bool /*ok*/) override {
    // write failed or stream closed by server
  }

  void OnDone(const grpc::Status& status) override {
    std::cout << "Closed channel. peer[" << ctx_.peer() << "]"
              << std::endl;
    // RPC failed or finished cleanly
    delete this;
  }

 private:
  UpaGrpcService::Stub* stub_;
  UpaGrpcClientCallback callback_;
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

  reactor_ = new UpaGrpcClientReactor(stub_.get(), callback_, this);

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

int UpaGrpcClient::GetState() {
  return static_cast<int>(channel_->GetState(false));
}

bool UpaGrpcClient::WaitForConnected(int wait_sec) {
  std::chrono::time_point deadline =
      std::chrono::system_clock::now() + std::chrono::seconds(wait_sec);
  return channel_->WaitForConnected(deadline);
}

void UpaGrpcClient::SetReconnectBackoff(int min, int max) {
  if (min >= 0) min_backoff_ = min;
  if (max >= 0) max_backoff_ = max;
}

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
  UpaGrpcServiceImpl(MsgType msg_type, UpaGrpcServerCallback callback)
      : msg_type_(msg_type), callback_(callback) {}

  // TODO : accept / close callback을 적절히 잘 넣으면 될 듯
  // -- SetAcctptCallback, SetCloseCallback

  // This function is called by gRPC when a new RPC arrives
  UpaGrpcServerReactorClass* SendMessage(
      grpc::CallbackServerContext* context) override {

    std::cout << "Accepted channel. peer[" << context->peer() << "]"
              << std::endl;

    class Reactor : public UpaGrpcServerReactorClass {
     public:
      Reactor(grpc::CallbackServerContext* context, MsgType msg_type,
              UpaGrpcServerCallback callback)
          : context_(context), msg_type_(msg_type), callback_(callback) {
        peer_ = context->peer();
        // Start readind the first message
        StartRead(&read_msg_);
      }
      ~Reactor() {}

      void OnReadDone(bool ok) override {
        if (!ok) {  // Client closed the write side
          Finish(grpc::Status::OK);
          return;
        }

        // Process incoming message
        if (read_msg_.msg_type() != msg_type_) {
          std::cout << "Invalid type(" << read_msg_.msg_type()
                    << ") message received. drop the message." << std::endl;
          Finish(grpc::Status::CANCELLED);
          return;
        }

        int rv = callback_(context_, &read_msg_, this);
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
        std::cout << "Closed channel. peer[" << peer_ << "]" << std::endl;
        // RPC finished
        delete this;
      }

     private:
      grpc::CallbackServerContext* context_;
      MsgType msg_type_;
      UpaGrpcServerCallback callback_;
      std::string peer_;
      Message read_msg_;
    };

    return new Reactor(context, msg_type_, callback_);
  }

 private:
  MsgType msg_type_;
  UpaGrpcServerCallback callback_;
};

int UpaGrpcServer::Run() {
  if (server_ != nullptr) {
    std::cout << "Already started grpc server." << std::endl;
    return 1;
  }

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();

  UpaGrpcServiceImpl serviceImpl(msg_type_, callback_);

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
    server_->Shutdown();
    server_->Wait();
    server_.reset();
    server_ = nullptr;

    std::cout << "Server stop." << std::endl;
  }
}

int UpaGrpcServerSend(UpaGrpcServerReactorClass* reactor, Message* msg) {
  if (reactor == nullptr || msg == nullptr) return -1;
  reactor->StartWrite(msg);
  return 0;
}