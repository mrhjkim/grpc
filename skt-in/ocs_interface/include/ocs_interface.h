#ifndef __GRPC_OCS_INTERFACE_H__
#define __GRPC_OCS_INTERFACE_H__

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <condition_variable>
#include <mutex>
#include <string>

#ifdef BAZEL_BUILD
#include "protos/ocs_interface.grpc.pb.h"
#else
#include "ocs_interface.grpc.pb.h"
#endif

struct OcsInterfaceClientContext {
  grpc::ClientContext base;
  std::mutex mu;
  std::condition_variable cv;
  bool done;
  bool result;
};

using OcsInterfaceClientCallback = void (*)(int, void*);
using OcsInterfaceServerCallback = int (*)(const void*, void*);

// API of HeartbeatRequest message
const char* GetNodeName(const ocs_interface::HeartbeatRequest* msg);
void SetNodeName(ocs_interface::HeartbeatRequest* msg, const char* val);
void PrintHeartbeatRequest(const ocs_interface::HeartbeatRequest* msg,
                           bool is_send);

// API of HeartbeatResponse message
const char* GetNodeName(const ocs_interface::HeartbeatResponse* msg);
void SetNodeName(ocs_interface::HeartbeatResponse* msg, const char* val);
int GetStatus(const ocs_interface::HeartbeatResponse* msg);
void SetStatus(ocs_interface::HeartbeatResponse* msg, int val);
void PrintHeartbeatResponse(const ocs_interface::HeartbeatResponse* msg,
                            bool is_send);

// API of ServiceRequest message
int GetMessageType(const ocs_interface::ServiceRequest* msg);
void SetMessageType(ocs_interface::ServiceRequest* msg, int val);
const char* GetSourceNodeName(const ocs_interface::ServiceRequest* msg);
void SetSourceNodeName(ocs_interface::ServiceRequest* msg, const char* val);
const char* GetDestinationNodeName(const ocs_interface::ServiceRequest* msg);
void SetDestinationNodeName(ocs_interface::ServiceRequest* msg,
                            const char* val);
const char* GetUserKey(const ocs_interface::ServiceRequest* msg);
void SetUserKey(ocs_interface::ServiceRequest* msg, const char* val);
const char* GetSessionId(const ocs_interface::ServiceRequest* msg);
void SetSessionId(ocs_interface::ServiceRequest* msg, const char* val);
const char* GetData(const ocs_interface::ServiceRequest* msg, size_t* size);
void SetData(ocs_interface::ServiceRequest* msg, const char* val, size_t size);
void PrintServiceRequest(const ocs_interface::ServiceRequest* msg,
                         bool is_send);

// API of ServiceResponse message
int GetMessageType(const ocs_interface::ServiceResponse* msg);
void SetMessageType(ocs_interface::ServiceResponse* msg, int val);
const char* GetSourceNodeName(const ocs_interface::ServiceResponse* msg);
void SetSourceNodeName(ocs_interface::ServiceResponse* msg, const char* val);
const char* GetDestinationNodeName(const ocs_interface::ServiceResponse* msg);
void SetDestinationNodeName(ocs_interface::ServiceResponse* msg,
                            const char* val);
const char* GetUserKey(const ocs_interface::ServiceResponse* msg);
void SetUserKey(ocs_interface::ServiceResponse* msg, const char* val);
const char* GetSessionId(const ocs_interface::ServiceResponse* msg);
void SetSessionId(ocs_interface::ServiceResponse* msg, const char* val);
const char* GetData(const ocs_interface::ServiceResponse* msg, size_t* size);
void SetData(ocs_interface::ServiceResponse* msg, const char* val, size_t size);
void PrintServiceResponse(const ocs_interface::ServiceResponse* msg,
                          bool is_send);

class OcsInterfaceClient {
 public:
  OcsInterfaceClient(std::shared_ptr<grpc::Channel> channel)
      : channel_(channel),
        stub_(ocs_interface::OcsInterface::NewStub(channel)) {}
  OcsInterfaceClient(std::string target)
      : channel_(
            grpc::CreateChannel(target, grpc::InsecureChannelCredentials())),
        stub_(ocs_interface::OcsInterface::NewStub(channel_)) {}

  int GetState();
  bool WaitForConnected(int wait_sec);

  int Heartbeat(OcsInterfaceClientContext* context,
                const std::string& node_name,
                ocs_interface::HeartbeatResponse* response,
                OcsInterfaceClientCallback callback);
  int Inquiry(OcsInterfaceClientContext* context,
              ocs_interface::ServiceRequest* request,
              ocs_interface::ServiceResponse* response,
              OcsInterfaceClientCallback callback);
  int Wait(OcsInterfaceClientContext* context);

 private:
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<ocs_interface::OcsInterface::Stub> stub_;
};

// OcsInterface Server
void OcsInterfaceServerRun(std::string ip, uint16_t port,
                           OcsInterfaceServerCallback callback_h,
                           OcsInterfaceServerCallback callback_i);

#endif  // __GRPC_OCS_INTERFACE_H__