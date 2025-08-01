#ifndef __UPA_GRPC_CAPI_H__
#define __UPA_GRPC_CAPI_H__

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// gRPC Protocol Adapter Message
typedef void upa_grpc_message_t;

typedef enum upa_grpc_msg_type_e {
  UPA_GRPC_MSG_TYPE_DBIF = 0,
  UPA_GRPC_MSG_TYPE_MMIF,
  UPA_GRPC_MSG_TYPE_TCP,
  UPA_GRPC_MSG_TYPE_DIAMETER,
  UPA_GRPC_MSG_TYPE_SIP,
} upa_grpc_msg_type_e;
#define UPA_GRPC_MSG_TYPE_MIN UPA_GRPC_MSG_TYPE_DBIF
#define UPA_GRPC_MSG_TYPE_MAX UPA_GRPC_MSG_TYPE_SIP
const char* upa_grpc_msg_type_str(upa_grpc_msg_type_e e);

upa_grpc_message_t* upa_grpc_message_alloc();
void upa_grpc_message_destroy(upa_grpc_message_t* msg);
int upa_grpc_message_get_msg_type(const upa_grpc_message_t* msg);
void upa_grpc_message_set_msg_type(upa_grpc_message_t* msg, int val);
const char* upa_grpc_message_get_src_id(const upa_grpc_message_t* msg);
void upa_grpc_message_set_src_id(upa_grpc_message_t* msg, const char* val);
const char* upa_grpc_message_get_dst_id(const upa_grpc_message_t* msg);
void upa_grpc_message_set_dst_id(upa_grpc_message_t* msg, const char* val);
const char* upa_grpc_message_get_route_key(const upa_grpc_message_t* msg);
void upa_grpc_message_set_route_key(upa_grpc_message_t* msg, const char* val);
const char* upa_grpc_message_get_session_key(const upa_grpc_message_t* msg);
void upa_grpc_message_set_session_key(upa_grpc_message_t* msg, const char* val);
const char* upa_grpc_message_get_data(const upa_grpc_message_t* msg,
                                      size_t* size);
void upa_grpc_message_set_data(upa_grpc_message_t* msg, const char* val,
                               size_t size);
void upa_grpc_message_print(const upa_grpc_message_t* msg, int is_send);

// gRPC Protocol Adapter Client Class
typedef void upa_grpc_client_context_t;
typedef void* upa_grpc_client_handler;
typedef void (*upa_grpc_client_callback_f)(int, void*);

upa_grpc_client_context_t* upa_grpc_client_context_alloc(int wait_sec);
void upa_grpc_client_context_destroy(upa_grpc_client_context_t* context);
int upa_grpc_client_context_wait(upa_grpc_client_context_t* context);

upa_grpc_client_handler upa_grpc_client_create(const char* ip,
                                               unsigned short port,
                                               upa_grpc_msg_type_e msg_type);
// upa_grpc_client_handler upa_grpc_client_create(const char* target,
//                                                upa_grpc_msg_type_e msg_type);
int upa_grpc_client_start(upa_grpc_client_handler handler);
void upa_grpc_client_stop(upa_grpc_client_handler handler);
int upa_grpc_client_get_state(upa_grpc_client_handler handler);
int upa_grpc_client_wait_for_connected(upa_grpc_client_handler handler,
                                       int wait_sec);
int upa_grpc_client_send(upa_grpc_client_handler handler,
                         upa_grpc_client_context_t* context,
                         upa_grpc_message_t* request,
                         upa_grpc_message_t* response,
                         upa_grpc_client_callback_f callback);

// gRPC Protocol Adapter Server Class
typedef int (*upa_grpc_server_callback_f)(const void*, void*);
typedef void* upa_grpc_server_handler;

upa_grpc_server_handler upa_grpc_server_create(
    const char* ip, unsigned short port, upa_grpc_msg_type_e msg_type,
    upa_grpc_server_callback_f callback);
// upa_grpc_server_handler upa_grpc_server_create(
//     const char* target, upa_grpc_msg_type_e msg_type,
//     upa_grpc_server_callback_f callback);
int upa_grpc_server_run(upa_grpc_server_handler handler);
void upa_grpc_server_start(upa_grpc_server_handler handler);
void upa_grpc_server_stop(upa_grpc_server_handler handler);

#ifdef __cplusplus
}
#endif

#endif // __UPA_GRPC_CAPI_H__
