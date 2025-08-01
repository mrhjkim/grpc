#include "../include/upa_grpc.capi.h"

#include <iostream>
#include <memory>
#include <string>

#include "../include/upa_grpc.h"

using namespace upa_grpc;

/**
 * C API wrapper
 */
extern "C" {

// C API of the gRPC Protocol Adapter Message struct.
upa_grpc_message_t* upa_grpc_message_alloc() { return new Message(); }

void upa_grpc_message_destroy(upa_grpc_message_t* msg) {
  if (msg) delete static_cast<Message*>(msg);
}

int upa_grpc_message_get_msg_type(const upa_grpc_message_t* msg) {
  if (!msg) return -1;
  return GetMsgType(static_cast<const Message*>(msg));
}
void upa_grpc_message_set_msg_type(upa_grpc_message_t* msg, int val) {
  if (!msg) return;
  SetMsgType(static_cast<Message*>(msg), val);
}

const char* upa_grpc_message_get_src_id(const upa_grpc_message_t* msg) {
  if (!msg) return "";
  return GetSrcId(static_cast<const Message*>(msg));
}
void upa_grpc_message_set_src_id(upa_grpc_message_t* msg,
                                         const char* val) {
  if (!msg || !val) return;
  SetSrcId(static_cast<Message*>(msg), val);
}

const char* upa_grpc_message_get_dst_id(
    const upa_grpc_message_t* msg) {
  if (!msg) return "";
  return GetDstId(static_cast<const Message*>(msg));
}
void upa_grpc_message_set_dst_id(upa_grpc_message_t* msg,
                                              const char* val) {
  if (!msg || !val) return;
  SetDstId(static_cast<Message*>(msg), val);
}

const char* upa_grpc_message_get_route_key(const upa_grpc_message_t* msg) {
  if (!msg) return "";
  return GetRouteKey(static_cast<const Message*>(msg));
}
void upa_grpc_message_set_route_key(upa_grpc_message_t* msg, const char* val) {
  if (!msg || !val) return;
  SetRouteKey(static_cast<Message*>(msg), val);
}

const char* upa_grpc_message_get_session_key(const upa_grpc_message_t* msg) {
  if (!msg) return "";
  return GetSessionKey(static_cast<const Message*>(msg));
}
void upa_grpc_message_set_session_key(upa_grpc_message_t* msg, const char* val) {
  if (!msg || !val) return;
  SetSessionKey(static_cast<Message*>(msg), val);
}

const char* upa_grpc_message_get_data(const upa_grpc_message_t* msg, size_t* size) {
  if (!msg || !size) {
    return NULL;
    if (size) *size = 0;
  }
  return GetData(static_cast<const Message*>(msg), size);
}
void upa_grpc_message_set_data(upa_grpc_message_t* msg, const char* val,
                             size_t size) {
  if (!msg || !val || !size) return;
  SetData(static_cast<Message*>(msg), val, size);
}

void upa_grpc_message_print(const upa_grpc_message_t* msg, int is_send) {
  if (!msg) return;
  PrintMessage(static_cast<const Message*>(msg),
                      static_cast<bool>(is_send));
}

// C API of the gRPC Protocol Adapter Client class.
upa_grpc_client_context_t* upa_grpc_client_context_alloc(int wait_sec) {
  return new UpaGrpcClientContext(wait_sec);
}

void upa_grpc_client_context_destroy(upa_grpc_client_context_t* context) {
  if (context) delete static_cast<UpaGrpcClientContext*>(context);
}

int upa_grpc_client_context_wait(upa_grpc_client_context_t* context) {
  if (!context) return -1;
  return static_cast<UpaGrpcClientContext*>(context)->Wait();
}

upa_grpc_client_handler upa_grpc_client_create(const char* ip,
                                               unsigned short port,
                                               upa_grpc_msg_type_e msg_type) {
  if (!ip) return NULL;
  return new UpaGrpcClient(ip, port, static_cast<MsgType>(msg_type));
}
/*
upa_grpc_client_handler upa_grpc_client_create(const char* target,
                                               upa_grpc_msg_type_e msg_type) {
  if (!target) return NULL;
  return new UpaGrpcClient(target, static_cast<MsgType>(msg_type));
}
*/

int upa_grpc_client_start(upa_grpc_client_handler handler) {
  if (!handler) return -1;
  return static_cast<UpaGrpcClient*>(handler)->Start();
}

void upa_grpc_client_stop(upa_grpc_client_handler handler) {
  if (!handler) return;
  static_cast<UpaGrpcClient*>(handler)->Stop();
}

int upa_grpc_client_get_state(upa_grpc_client_handler handler) {
  if (!handler) return -1;
  return static_cast<UpaGrpcClient*>(handler)->GetState();
}

int upa_grpc_client_wait_for_connected(upa_grpc_client_handler handler,
                                          int wait_sec) {
  if (!handler) return false;
  return static_cast<UpaGrpcClient*>(handler)->WaitForConnected(wait_sec);
}

int upa_grpc_client_send(upa_grpc_client_handler handler,
                               upa_grpc_client_context_t* context,
                               upa_grpc_message_t* request,
                               upa_grpc_message_t* response,
                               upa_grpc_client_callback_f callback) {
  if (!handler || !context || !request || !response || !callback) return -1;
  return static_cast<UpaGrpcClient*>(handler)->Send(
      static_cast<UpaGrpcClientContext*>(context),
      static_cast<Message*>(request), static_cast<Message*>(response),
      callback);
}

// C API of OcsInterfaceServer
upa_grpc_server_handler upa_grpc_server_create(
    const char* ip, unsigned short port, upa_grpc_msg_type_e msg_type,
    upa_grpc_server_callback_f callback) {
  if (!ip || !callback) return NULL;
  return new UpaGrpcServer(ip, port, static_cast<MsgType>(msg_type), callback);
}
/*
upa_grpc_server_handler upa_grpc_server_create(
    const char* target, upa_grpc_msg_type_e msg_type,
    upa_grpc_server_callback_f callback) {
  if (!target || !callback) return NULL;
  return new UpaGrpcServer(target, static_cast<MsgType>(msg_type), callback);
}
*/

int upa_grpc_server_run(upa_grpc_server_handler handler) {
  if (!handler) return -1;
  return static_cast<UpaGrpcServer*>(handler)->Run();
}

void upa_grpc_server_start(upa_grpc_server_handler handler) {
  if (!handler) return;
  return static_cast<UpaGrpcServer*>(handler)->Start();
}

void upa_grpc_server_stop(upa_grpc_server_handler handler) {
  if (!handler) return;
  static_cast<UpaGrpcServer*>(handler)->Stop();
}

}  // extern "C"