#include "../include/ocs_interface_capi.h"

#include <iostream>
#include <memory>
#include <string>

#include "../include/ocs_interface.h"

using namespace ocs_interface;

/**
 * C API wrapper
 */
extern "C" {

// C API of HeartbeatRequest message
HeartbeatRequest_t* HeartbeatRequest_alloc() { return new HeartbeatRequest(); }
void HeartbeatRequest_destroy(HeartbeatRequest_t* msg) {
  if (msg) delete static_cast<HeartbeatRequest*>(msg);
}

const char* HeartbeatRequest_get_node_name(const HeartbeatRequest_t* msg) {
  if (!msg) return "";
  return GetNodeName(static_cast<const HeartbeatRequest*>(msg));
}
void HeartbeatRequest_set_node_name(HeartbeatRequest_t* msg, const char* val) {
  if (!msg || !val) return;
  SetNodeName(static_cast<HeartbeatRequest*>(msg), val);
}

void HeartbeatRequest_print(const HeartbeatRequest_t* msg, int is_send) {
  if (!msg) return;
  PrintHeartbeatRequest(static_cast<const HeartbeatRequest*>(msg),
                        static_cast<bool>(is_send));
}

// C API of HeartbeatResponse message
HeartbeatResponse_t* HeartbeatResponse_alloc() {
  return new HeartbeatResponse();
}
void HeartbeatResponse_destroy(HeartbeatResponse_t* msg) {
  if (msg) delete static_cast<HeartbeatResponse*>(msg);
}

const char* HeartbeatResponse_get_node_name(const HeartbeatResponse_t* msg) {
  if (!msg) return "";
  return GetNodeName(static_cast<const HeartbeatResponse*>(msg));
}
void HeartbeatResponse_set_node_name(HeartbeatResponse_t* msg,
                                     const char* val) {
  if (!msg || !val) return;
  SetNodeName(static_cast<HeartbeatResponse*>(msg), val);
}

int HeartbeatResponse_get_status(const HeartbeatResponse_t* msg) {
  if (!msg) return static_cast<int>(SystemStatus::SYSTEM_STATUS_UNKNOWN);
  return GetStatus(static_cast<const HeartbeatResponse*>(msg));
}

void HeartbeatResponse_set_status(HeartbeatResponse_t* msg, int val) {
  if (!msg) return;
  SetStatus(static_cast<HeartbeatResponse*>(msg), val);
}

void HeartbeatResponse_print(const HeartbeatResponse_t* msg, int is_send) {
  if (!msg) return;
  PrintHeartbeatRequest(static_cast<const HeartbeatRequest*>(msg),
                        static_cast<bool>(is_send));
}

// C API of ServiceRequest message
ServiceRequest_t* ServiceRequest_alloc() { return new ServiceRequest(); }
void ServiceRequest_destroy(ServiceRequest_t* msg) {
  if (msg) delete static_cast<ServiceRequest*>(msg);
}

int ServiceRequest_get_message_type(const ServiceRequest_t* msg) {
  if (!msg) return -1;
  return GetMessageType(static_cast<const ServiceRequest*>(msg));
}
void ServiceRequest_set_message_type(ServiceRequest_t* msg, int val) {
  if (!msg) return;
  SetMessageType(static_cast<ServiceRequest*>(msg), val);
}

const char* ServiceRequest_get_source_node_name(const ServiceRequest_t* msg) {
  if (!msg) return "";
  return GetSourceNodeName(static_cast<const ServiceRequest*>(msg));
}
void ServiceRequest_set_source_node_name(ServiceRequest_t* msg,
                                         const char* val) {
  if (!msg || !val) return;
  SetSourceNodeName(static_cast<ServiceRequest*>(msg), val);
}

const char* ServiceRequest_get_destination_node_name(
    const ServiceRequest_t* msg) {
  if (!msg) return "";
  return GetDestinationNodeName(static_cast<const ServiceRequest*>(msg));
}
void ServiceRequest_set_destination_node_name(ServiceRequest_t* msg,
                                              const char* val) {
  if (!msg || !val) return;
  SetDestinationNodeName(static_cast<ServiceRequest*>(msg), val);
}

const char* ServiceRequest_get_user_key(const ServiceRequest_t* msg) {
  if (!msg) return "";
  return GetUserKey(static_cast<const ServiceRequest*>(msg));
}
void ServiceRequest_set_user_key(ServiceRequest_t* msg, const char* val) {
  if (!msg || !val) return;
  SetUserKey(static_cast<ServiceRequest*>(msg), val);
}

const char* ServiceRequest_get_session_id(const ServiceRequest_t* msg) {
  if (!msg) return "";
  return GetSessionId(static_cast<const ServiceRequest*>(msg));
}
void ServiceRequest_set_session_id(ServiceRequest_t* msg, const char* val) {
  if (!msg || !val) return;
  SetSessionId(static_cast<ServiceRequest*>(msg), val);
}

const char* ServiceRequest_get_data(const ServiceRequest_t* msg, size_t* size) {
  if (!msg || !size) {
    return NULL;
    if (size) *size = 0;
  }
  return GetData(static_cast<const ServiceRequest*>(msg), size);
}
void ServiceRequest_set_data(ServiceRequest_t* msg, const char* val,
                             size_t size) {
  if (!msg || !val || !size) return;
  SetData(static_cast<ServiceRequest*>(msg), val, size);
}

void ServiceRequest_print(const ServiceRequest_t* msg, int is_send) {
  if (!msg) return;
  PrintServiceRequest(static_cast<const ServiceRequest*>(msg),
                      static_cast<bool>(is_send));
}

// C API of ServiceResponse message
ServiceResponse_t* ServiceResponse_alloc() { return new ServiceResponse(); }
void ServiceResponse_destroy(ServiceResponse_t* msg) {
  if (msg) delete static_cast<ServiceResponse*>(msg);
}

int ServiceResponse_get_message_type(const ServiceResponse_t* msg) {
  if (!msg) return -1;
  return GetMessageType(static_cast<const ServiceResponse*>(msg));
}
void ServiceResponse_set_message_type(ServiceResponse_t* msg, int val) {
  if (!msg) return;
  SetMessageType(static_cast<ServiceResponse*>(msg), val);
}

const char* ServiceResponse_get_source_node_name(const ServiceResponse_t* msg) {
  if (!msg) return "";
  return GetSourceNodeName(static_cast<const ServiceResponse*>(msg));
}
void ServiceResponse_set_source_node_name(ServiceResponse_t* msg,
                                          const char* val) {
  if (!msg || !val) return;
  SetSourceNodeName(static_cast<ServiceResponse*>(msg), val);
}

const char* ServiceResponse_get_destination_node_name(
    const ServiceResponse_t* msg) {
  if (!msg) return "";
  return GetDestinationNodeName(static_cast<const ServiceResponse*>(msg));
}
void ServiceResponse_set_destination_node_name(ServiceResponse_t* msg,
                                               const char* val) {
  if (!msg || !val) return;
  SetDestinationNodeName(static_cast<ServiceResponse*>(msg), val);
}

const char* ServiceResponse_get_user_key(const ServiceResponse_t* msg) {
  if (!msg) return "";
  return GetUserKey(static_cast<const ServiceResponse*>(msg));
}
void ServiceResponse_set_user_key(ServiceResponse_t* msg, const char* val) {
  if (!msg || !val) return;
  SetUserKey(static_cast<ServiceResponse*>(msg), val);
}

const char* ServiceResponse_get_session_id(const ServiceResponse_t* msg) {
  if (!msg) return "";
  return GetSessionId(static_cast<const ServiceResponse*>(msg));
}
void ServiceResponse_set_session_id(ServiceResponse_t* msg, const char* val) {
  if (!msg || !val) return;
  SetSessionId(static_cast<ServiceResponse*>(msg), val);
}

const char* ServiceResponse_get_data(const ServiceResponse_t* msg,
                                     size_t* size) {
  if (!msg || !size) {
    return NULL;
    if (size) *size = 0;
  }
  return GetData(static_cast<const ServiceResponse*>(msg), size);
}
void ServiceResponse_set_data(ServiceResponse_t* msg, const char* val,
                              size_t size) {
  if (!msg || !val || !size) return;
  SetData(static_cast<ServiceResponse*>(msg), val, size);
}

void ServiceResponse_print(const ServiceResponse_t* msg, int is_send) {
  if (!msg) return;
  PrintServiceResponse(static_cast<const ServiceResponse*>(msg),
                       static_cast<bool>(is_send));
}

// C API of the OcsInterfaceClient class.
OcsInterfaceClientContext_t* OcsInterfaceClientContext_alloc() {
  return new OcsInterfaceClientContext();
}

void OcsInterfaceClientContext_destroy(OcsInterfaceClientContext_t* context) {
  if (context) delete static_cast<OcsInterfaceClientContext*>(context);
}

OcsInterfaceClientHandler OcsInterfaceClient_create(const char* target) {
  if (!target) return NULL;
  return new OcsInterfaceClient(target);
}

int OcsInterfaceClient_get_state(OcsInterfaceClientHandler handler) {
  if (!handler) return -1;
  return static_cast<OcsInterfaceClient*>(handler)->GetState();
}

int OcsInterfaceClient_wait_for_connected(OcsInterfaceClientHandler handler,
                                          int wait_sec) {
  if (!handler) return false;
  return static_cast<OcsInterfaceClient*>(handler)->WaitForConnected(wait_sec);
}

int OcsInterfaceClient_heartbeat(OcsInterfaceClientHandler handler,
                                 OcsInterfaceClientContext_t* context,
                                 const char* node_name,
                                 HeartbeatResponse_t* response,
                                 OcsInterfaceClientCallback callback) {
  if (!handler || !context || !node_name || !response || !callback) return -1;
  return static_cast<OcsInterfaceClient*>(handler)->Heartbeat(
      static_cast<OcsInterfaceClientContext*>(context), node_name,
      static_cast<HeartbeatResponse*>(response), callback);
}

int OcsInterfaceClient_inquiry(OcsInterfaceClientHandler handler,
                               OcsInterfaceClientContext_t* context,
                               ServiceRequest_t* request,
                               ServiceResponse_t* response,
                               OcsInterfaceClientCallback callback) {
  if (!handler || !context || !request || !response || !callback) return -1;
  return static_cast<OcsInterfaceClient*>(handler)->Inquiry(
      static_cast<OcsInterfaceClientContext*>(context),
      static_cast<ServiceRequest*>(request),
      static_cast<ServiceResponse*>(response), callback);
}

int OcsInterfaceClient_wait(OcsInterfaceClientHandler handler,
                            OcsInterfaceClientContext_t* context) {
  if (!handler || !context) return -1;
  return static_cast<OcsInterfaceClient*>(handler)->Wait(
      static_cast<OcsInterfaceClientContext*>(context));
}

// C API of OcsInterfaceServer
void OcsInterfaceServer_run(const char* ip, unsigned short port,
                            OcsInterfaceServerCallback heartbeat_f,
                            OcsInterfaceServerCallback inquiry_f) {
  if (!ip || !heartbeat_f || !inquiry_f) return;
  OcsInterfaceServerRun(ip, port, heartbeat_f, inquiry_f);
}
}