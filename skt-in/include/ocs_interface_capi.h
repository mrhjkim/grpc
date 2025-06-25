#ifndef __GRPC_OCS_INTERFACE_CAPI_H__
#define __GRPC_OCS_INTERFACE_CAPI_H__

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Heartbeat Request/Response message
typedef void HeartbeatRequest_t;
typedef void HeartbeatResponse_t;

typedef enum system_status_e {
  SYSTEM_STATUS_UNKNOWN = 0,
  SYSTEM_STATUS_ACTIVE,
  SYSTEM_STATUS_STANDBY,
} system_status_e;
const char* system_status_str(system_status_e e);

HeartbeatRequest_t* HeartbeatRequest_alloc();
void HeartbeatRequest_destroy(HeartbeatRequest_t* msg);
const char* HeartbeatRequest_get_node_name(const HeartbeatRequest_t* msg);
void HeartbeatRequest_set_node_name(HeartbeatRequest_t* msg, const char* val);
void HeartbeatRequest_print(const HeartbeatRequest_t* msg, int is_send);

HeartbeatResponse_t* HeartbeatResponse_alloc();
void HeartbeatResponse_destroy(HeartbeatResponse_t* msg);
const char* HeartbeatResponse_get_node_name(const HeartbeatResponse_t* msg);
void HeartbeatResponse_set_node_name(HeartbeatResponse_t* msg, const char* val);
int HeartbeatResponse_get_status(const HeartbeatResponse_t* msg);
void HeartbeatResponse_set_status(HeartbeatResponse_t* msg, int val);
void HeartbeatResponse_print(const HeartbeatResponse_t* msg, int is_send);

// Service Request/Response message
typedef void ServiceRequest_t;
typedef void ServiceResponse_t;

typedef enum message_type_e {
  MESSAGE_TYPE_SCPAS = 0,
  MESSAGE_TYPE_AAA,
  MESSAGE_TYPE_INQ,
  // TODO ...
} message_type_e;
const char* message_type_str(message_type_e e);

ServiceRequest_t* ServiceRequest_alloc();
void ServiceRequest_destroy(ServiceRequest_t* msg);
int ServiceRequest_get_message_type(const ServiceRequest_t* msg);
void ServiceRequest_set_message_type(ServiceRequest_t* msg, int val);
const char* ServiceRequest_get_source_node_name(const ServiceRequest_t* msg);
void ServiceRequest_set_source_node_name(ServiceRequest_t* msg,
                                         const char* val);
const char* ServiceRequest_get_destination_node_name(
    const ServiceRequest_t* msg);
void ServiceRequest_set_destination_node_name(ServiceRequest_t* msg,
                                              const char* val);
const char* ServiceRequest_get_user_key(const ServiceRequest_t* msg);
void ServiceRequest_set_user_key(ServiceRequest_t* msg, const char* val);
const char* ServiceRequest_get_session_id(const ServiceRequest_t* msg);
void ServiceRequest_set_session_id(ServiceRequest_t* msg, const char* val);
const char* ServiceRequest_get_data(const ServiceRequest_t* msg, size_t* size);
void ServiceRequest_set_data(ServiceRequest_t* msg, const char* val,
                             size_t size);
void ServiceRequest_print(const ServiceRequest_t* msg, int is_send);

ServiceResponse_t* ServiceResponse_alloc();
void ServiceResponse_destroy(ServiceResponse_t* msg);
int ServiceResponse_get_message_type(const ServiceResponse_t* msg);
void ServiceResponse_set_message_type(ServiceResponse_t* msg, int val);
const char* ServiceResponse_get_source_node_name(const ServiceResponse_t* msg);
void ServiceResponse_set_source_node_name(ServiceResponse_t* msg,
                                          const char* val);
const char* ServiceResponse_get_destination_node_name(
    const ServiceResponse_t* msg);
void ServiceResponse_set_destination_node_name(ServiceResponse_t* msg,
                                               const char* val);
const char* ServiceResponse_get_user_key(const ServiceResponse_t* msg);
void ServiceResponse_set_user_key(ServiceResponse_t* msg, const char* val);
const char* ServiceResponse_get_session_id(const ServiceResponse_t* msg);
void ServiceResponse_set_session_id(ServiceResponse_t* msg, const char* val);
const char* ServiceResponse_get_data(const ServiceResponse_t* msg,
                                     size_t* size);
void ServiceResponse_set_data(ServiceResponse_t* msg, const char* val,
                              size_t size);
void ServiceResponse_print(const ServiceResponse_t* msg, int is_send);

// OCS Interface Client Class
typedef void OcsInterfaceClientContext_t;
typedef void* OcsInterfaceClientHandler;
typedef void (*OcsInterfaceClientCallback)(int, void*);

OcsInterfaceClientContext_t* OcsInterfaceClientContext_alloc();
void OcsInterfaceClientContext_destroy(OcsInterfaceClientContext_t* context);

OcsInterfaceClientHandler OcsInterfaceClient_create(const char* target);
int OcsInterfaceClient_get_state(OcsInterfaceClientHandler handler);
int OcsInterfaceClient_wait_for_connected(OcsInterfaceClientHandler handler,
                                          int wait_sec);
int OcsInterfaceClient_heartbeat(OcsInterfaceClientHandler handler,
                                 OcsInterfaceClientContext_t* context,
                                 const char* node_name,
                                 HeartbeatResponse_t* response,
                                 OcsInterfaceClientCallback callback);
int OcsInterfaceClient_inquiry(OcsInterfaceClientHandler handler,
                               OcsInterfaceClientContext_t* context,
                               ServiceRequest_t* request,
                               ServiceResponse_t* response,
                               OcsInterfaceClientCallback callback);
int OcsInterfaceClient_wait(OcsInterfaceClientHandler handler,
                            OcsInterfaceClientContext_t* context);

// OCS Interface Server Class
typedef int (*OcsInterfaceServerCallback)(const void*, void*);

void OcsInterfaceServer_run(const char* ip, unsigned short port,
                            OcsInterfaceServerCallback heartbeat_f,
                            OcsInterfaceServerCallback inquiry_f);

#ifdef __cplusplus
}
#endif

#endif // __GRPC_OCS_INTERFACE_CAPI_H__
