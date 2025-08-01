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
int UpaGrpcClientContext::Wait() {
  std::unique_lock<std::mutex> lock(mu_);
  cv_.wait(lock, [this] { return this->done_; });
  return result_;
}

void UpaGrpcClientContext::Done(grpc::StatusCode result) {
  std::unique_lock<std::mutex> lock(mu_);
  done_ = true;
  result_ = result;
  cv_.notify_all();
}

int UpaGrpcClient::Start() {
  if (channel_ != nullptr) {
    std::cout << "Already created channel." << std::endl;
    return 1;
  }

  channel_ = grpc::CreateChannel(target_, grpc::InsecureChannelCredentials());
  if (channel_ == nullptr) {
    std::cout << "Create channel failed. target(" << target_ << ")"
              << std::endl;
    return -1;
  }
  stub_ = UpaGrpcService::NewStub(channel_);

  return 0;
}

void UpaGrpcClient::Stop() {
  if (channel_ != nullptr) {
    channel_.reset();
    channel_ = nullptr;
  }
  if (stub_ != nullptr) {
    stub_.reset();
    stub_ = nullptr;
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

int UpaGrpcClient::Send(UpaGrpcClientContext* context, Message* request,
                        Message* response, UpaGrpcClientCallback callback) {
  request->set_msg_type(msg_type_);
  PrintMessage(request, true);

  stub_->async()->SendMessage(
      &context->base, request, response,
      [context, request, response, callback](grpc::Status status) {
        if (status.ok()) {
          PrintMessage(response, false);
        } else {
          std::cout << status.error_code() << ": " << status.error_message()
                    << std::endl;
        }

        callback(status.error_code(), response);

        context->Done(status.error_code());
      });

  return 0;
}


// Implementation of the UpaGrpcServer class.
class UpaGrpcServiceImpl final : public UpaGrpcService::CallbackService {
 public:
  UpaGrpcServiceImpl(MsgType msg_type, UpaGrpcServerCallback callback)
      : msg_type_(msg_type), callback_(callback) {}

  grpc::ServerUnaryReactor* SendMessage(grpc::CallbackServerContext* context,
                                        const Message* request,
                                        Message* response) override {
    std::cout << "Receive message from " << context->peer() << std::endl;

    if (request->msg_type() != msg_type_) {
      std::cout << "Invalid type(" << request->msg_type()
                << ") message received. drop the message." << std::endl;
      grpc::ServerUnaryReactor* reactor = context->DefaultReactor();
      reactor->Finish(grpc::Status::CANCELLED);
      return reactor;
    }

    PrintMessage(request, false);
    int rv = callback_(request, response);
    if (!rv) {
      PrintMessage(response, true);
    }
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();
    reactor->Finish(grpc::Status::OK);
    return reactor;
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


