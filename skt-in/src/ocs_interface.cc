#include "../include/ocs_interface.h"

#include <iostream>
#include <memory>
#include <string>

using namespace ocs_interface;

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
        //(std::setw(2) << static_cast<int>(bytePtr[i + j]) << " "));
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

// API of HeartbeatRequest message
const char* system_status_str(int status) {
  return SystemStatus_Name(status).c_str();
}

const char* GetNodeName(const HeartbeatRequest* msg) {
  return msg->node_name().c_str();
}
void SetNodeName(HeartbeatRequest* msg, const char* val) {
  msg->set_node_name(val);
}

void PrintHeartbeatRequest(const HeartbeatRequest* msg, bool is_send) {
  std::cout << (is_send ? "Send " : "Receive ") << "HeartbeatRequest message."
            << std::endl
            << " - node_name : '" << msg->node_name() << "'" << std::endl;
}

// API of HeartbeatResponse message
const char* GetNodeName(const HeartbeatResponse* msg) {
  return msg->node_name().c_str();
}
void SetNodeName(HeartbeatResponse* msg, const char* val) {
  msg->set_node_name(val);
}

int GetStatus(const HeartbeatResponse* msg) {
  return static_cast<int>(msg->status());
}
void SetStatus(HeartbeatResponse* msg, int val) {
  msg->set_status(static_cast<SystemStatus>(val));
}

void PrintHeartbeatResponse(const HeartbeatResponse* msg, bool is_send) {
  std::cout << (is_send ? "Send " : "Receive ") << "HeartbeatResponse message."
            << std::endl
            << " - node_name : '" << msg->node_name() << "'" << std::endl
            << " - status    : " << msg->status() << " ("
            << SystemStatus_Name(msg->status()) << ")" << std::endl;
}

// API of ServiceRequest message
const char* message_type_str(int type) {
  return MessageType_Name(type).c_str();
}

int GetMessageType(const ServiceRequest* msg) {
  return static_cast<int>(msg->message_type());
}
void SetMessageType(ServiceRequest* msg, int val) {
  msg->set_message_type(static_cast<MessageType>(val));
}

const char* GetSourceNodeName(const ServiceRequest* msg) {
  return msg->source_node_name().c_str();
}
void SetSourceNodeName(ServiceRequest* msg, const char* val) {
  msg->set_source_node_name(val);
}

const char* GetDestinationNodeName(const ServiceRequest* msg) {
  return msg->destination_node_name().c_str();
}
void SetDestinationNodeName(ServiceRequest* msg, const char* val) {
  msg->set_destination_node_name(val);
}

const char* GetUserKey(const ServiceRequest* msg) {
  return msg->user_key().c_str();
}
void SetUserKey(ServiceRequest* msg, const char* val) {
  msg->set_user_key(val);
}

const char* GetSessionId(const ServiceRequest* msg) {
  return msg->session_id().c_str();
}
void SetSessionId(ServiceRequest* msg, const char* val) {
  msg->set_session_id(val);
}

const char* GetData(const ServiceRequest* msg, size_t* size) {
  *size = msg->data().size();
  return msg->data().data();
}
void SetData(ServiceRequest* msg, const char* val, size_t size) {
  msg->set_data(val, size);
}

void PrintServiceRequest(const ServiceRequest* msg, bool is_send) {
  size_t data_size;
  const char* data = GetData(msg, &data_size);
  std::string hex_data;
  std::cout << (is_send ? "Send " : "Receive ") << "ServiceRequest message."
            << std::endl
            << " - message_type          : " << msg->message_type() << " ("
            << MessageType_Name(msg->message_type()) << ")" << std::endl
            << " - source_node_name      : '" << msg->source_node_name() << "'"
            << std::endl
            << " - destination_node_name : '" << msg->destination_node_name()
            << "'" << std::endl
            << " - user_key              : '" << msg->user_key() << "'"
            << std::endl
            << " - session_id            : '" << msg->session_id() << "'"
            << std::endl
            << " - data                  : (" << data_size << ")[" << std::endl
            << makeHexWithAscii(data, data_size, &hex_data) << "]" << std::endl;
}

// API of ServiceResponse message
int GetMessageType(const ServiceResponse* msg) {
  return static_cast<int>(msg->message_type());
}
void SetMessageType(ServiceResponse* msg, int val) {
  msg->set_message_type(static_cast<MessageType>(val));
}

const char* GetSourceNodeName(const ServiceResponse* msg) {
  return msg->source_node_name().c_str();
}
void SetSourceNodeName(ServiceResponse* msg, const char* val) {
  msg->set_source_node_name(val);
}

const char* GetDestinationNodeName(const ServiceResponse* msg) {
  return msg->destination_node_name().c_str();
}
void SetDestinationNodeName(ServiceResponse* msg, const char* val) {
  msg->set_destination_node_name(val);
}

const char* GetUserKey(const ServiceResponse* msg) {
  return msg->user_key().c_str();
}
void SetUserKey(ServiceResponse* msg, const char* val) {
  msg->set_user_key(val);
}

const char* GetSessionId(const ServiceResponse* msg) {
  return msg->session_id().c_str();
}
void SetSessionId(ServiceResponse* msg, const char* val) {
  msg->set_session_id(val);
}

const char* GetData(const ServiceResponse* msg, size_t* size) {
  *size = msg->data().size();
  return msg->data().data();
}
void SetData(ServiceResponse* msg, const char* val, size_t size) {
  msg->set_data(val, size);
}

void PrintServiceResponse(const ServiceResponse* msg, bool is_send) {
  size_t data_size;
  const char* data = GetData(msg, &data_size);
  std::string hex_data;
  std::cout << (is_send ? "Send " : "Receive ") << "ServiceResponse message."
            << std::endl
            << " - message_type          : " << msg->message_type() << " ("
            << MessageType_Name(msg->message_type()) << ")" << std::endl
            << " - source_node_name      : '" << msg->source_node_name() << "'"
            << std::endl
            << " - destination_node_name : '" << msg->destination_node_name()
            << "'" << std::endl
            << " - user_key              : '" << msg->user_key() << "'"
            << std::endl
            << " - session_id            : '" << msg->session_id() << "'"
            << std::endl
            << " - data                  : (" << data_size << ")[" << std::endl
            << makeHexWithAscii(data, data_size, &hex_data) << "]" << std::endl;
}

// Implementation of the OcsInterfaceClient class.
int OcsInterfaceClient::GetState() {
  return static_cast<int>(channel_->GetState(false));
}

bool OcsInterfaceClient::WaitForConnected(int wait_sec) {
  std::chrono::time_point deadline =
      std::chrono::system_clock::now() + std::chrono::seconds(wait_sec);
  return channel_->WaitForConnected(deadline);
}

int OcsInterfaceClient::Heartbeat(OcsInterfaceClientContext* context,
                                  const std::string& node_name,
                                  HeartbeatResponse* response,
                                  OcsInterfaceClientCallback callback) {
  HeartbeatRequest request;
  SetNodeName(&request, node_name.c_str());
  PrintHeartbeatRequest(&request, true);

  context->done = false;
  stub_->async()->HeartbeatMessage(
      &context->base, &request, response,
      [context, request, response, callback](grpc::Status status) {
        if (status.ok()) {
          PrintHeartbeatResponse(response, false);
        } else {
          std::cout << status.error_code() << ": " << status.error_message()
                    << std::endl;
        }

        callback(status.error_code(), response);

        std::lock_guard<std::mutex> lock(context->mu);
        context->result = status.error_code();
        context->done = true;
        context->cv.notify_one();
      });

  return 0;
}

int OcsInterfaceClient::Inquiry(OcsInterfaceClientContext* context,
                                ServiceRequest* request,
                                ServiceResponse* response,
                                OcsInterfaceClientCallback callback) {
  PrintServiceRequest(request, true);

  context->done = false;
  stub_->async()->ServiceMessage(
      &context->base, request, response,
      [context, request, response, callback](grpc::Status status) {
        if (status.ok()) {
          PrintServiceResponse(response, false);
        } else {
          std::cout << status.error_code() << ": " << status.error_message()
                    << std::endl;
        }

        callback(status.error_code(), response);

        std::lock_guard<std::mutex> lock(context->mu);
        context->result = status.error_code();
        context->done = true;
        context->cv.notify_one();
      });

  return 0;
}

int OcsInterfaceClient::Wait(OcsInterfaceClientContext* context) {
  std::unique_lock<std::mutex> lock(context->mu);
  while (!context->done) {
    context->cv.wait(lock);
  }
  return context->result;
}

// Implementation of the OcsInterface(Server) class.
class OcsInterfaceImpl final : public OcsInterface::CallbackService {
 public:
  OcsInterfaceImpl(OcsInterfaceServerCallback callback_h,
                   OcsInterfaceServerCallback callback_i)
      : callback_h_(callback_h), callback_i_(callback_i) {}

  grpc::ServerUnaryReactor* HeartbeatMessage(
      grpc::CallbackServerContext* context, const HeartbeatRequest* request,
      HeartbeatResponse* response) override {
    PrintHeartbeatRequest(request, false);
    int rv = callback_h_(request, response);
    if (!rv) {
      PrintHeartbeatResponse(response, true);
    }
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();
    reactor->Finish(grpc::Status::OK);  // TODO : callback 실행 결과 실패인 경우
                                        // 적절한 에러 코드 전달해야지 싶다..
    return reactor;
  }

  grpc::ServerUnaryReactor* ServiceMessage(grpc::CallbackServerContext* context,
                                           const ServiceRequest* request,
                                           ServiceResponse* response) override {
    PrintServiceRequest(request, false);
    int rv = callback_i_(request, response);
    if (!rv) {
      PrintServiceResponse(response, true);
    }
    grpc::ServerUnaryReactor* reactor = context->DefaultReactor();
    reactor->Finish(grpc::Status::OK);
    return reactor;
  }

 private:
  OcsInterfaceServerCallback callback_h_, callback_i_;
};

// Run OcsInterfaceServer function
void OcsInterfaceServerRun(std::string ip, uint16_t port,
                           OcsInterfaceServerCallback callback_h,
                           OcsInterfaceServerCallback callback_i) {
  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();

  std::string server_address = ip + ":" + std::to_string(port);
  OcsInterfaceImpl serviceImpl(callback_h, callback_i);

  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&serviceImpl);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}
