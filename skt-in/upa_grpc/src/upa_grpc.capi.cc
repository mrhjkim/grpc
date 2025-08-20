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

const char* upa_grpc_msg_type_str(upa_grpc_msg_type_e e) {
  return MsgTypeStr(static_cast<int>(e));
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
upa_grpc_client_handler upa_grpc_client_create(
    const char* ip, unsigned short port, const char* name,
    upa_grpc_msg_type_e msg_type, upa_grpc_client_on_read_f callback) {
  if (!ip || !name) return NULL;
  return new UpaGrpcClient(ip, port, name, static_cast<MsgType>(msg_type), callback);
}
/*
upa_grpc_client_handler upa_grpc_client_create(
    const char* target, const char* name, upa_grpc_msg_type_e msg_type,
    upa_grpc_client_on_read_f callback) {
  if (!target || !name) return NULL;
  return new UpaGrpcClient(target, name, static_cast<MsgType>(msg_type),
                           callback);
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

int upa_grpc_client_start_reactor(upa_grpc_client_handler handler) {
  if (!handler) return -1;
  return static_cast<UpaGrpcClient*>(handler)->StartReactor();
}

void upa_grpc_client_stop_reactor(upa_grpc_client_handler handler) {
    if (!handler) return;
  static_cast<UpaGrpcClient*>(handler)->StopReactor();
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

const char* upa_grpc_client_get_target(upa_grpc_client_handler handler) {
  if (!handler) return "";
  return static_cast<UpaGrpcClient*>(handler)->GetTarget();
}

const char* upa_grpc_client_get_name(upa_grpc_client_handler handler) {
  if (!handler) return "";
  return static_cast<UpaGrpcClient*>(handler)->GetName();
}

int upa_grpc_client_get_msg_type(upa_grpc_client_handler handler) {
  if (!handler) return UPA_GRPC_MSG_TYPE_MIN-1;
  return static_cast<UpaGrpcClient*>(handler)->GetMsgType();
}

upa_grpc_client_on_read_f upa_grpc_client_get_on_read(
    upa_grpc_client_handler handler) {
  if (!handler) return NULL;
  return static_cast<UpaGrpcClient*>(handler)->GetOnRead();
}

upa_grpc_client_on_connect_f upa_grpc_client_get_on_connect(
    upa_grpc_client_handler handler) {
  if (!handler) return NULL;
  return static_cast<UpaGrpcClient*>(handler)->GetOnConnect();
}
void upa_grpc_client_set_on_connect(upa_grpc_client_handler handler,
                                    upa_grpc_client_on_connect_f callback) {
  if (!handler) return;
  static_cast<UpaGrpcClient*>(handler)->SetOnConnect(callback);
}

upa_grpc_client_on_close_f upa_grpc_client_get_on_close(
    upa_grpc_client_handler handler) {
  if (!handler) return NULL;
  return static_cast<UpaGrpcClient*>(handler)->GetOnClose();
}
void upa_grpc_client_set_on_close(upa_grpc_client_handler handler,
                                  upa_grpc_client_on_close_f callback) {
  if (!handler) return;
  static_cast<UpaGrpcClient*>(handler)->SetOnClose(callback);
}

void upa_grpc_client_set_reconnect_backoff(upa_grpc_client_handler handler,
                                           int min, int max);

void* upa_grpc_client_get_user_data(upa_grpc_client_handler handler) {
  if(!handler) return NULL;
  return static_cast<UpaGrpcClient*>(handler)->GetUserData();
}

void upa_grpc_client_set_user_data(upa_grpc_client_handler handler,
                                   void* user_data) {
  if (!handler) return;
  static_cast<UpaGrpcClient*>(handler)->SetUserData(user_data);
}

void upa_grpc_client_set_reconnect_backoff(upa_grpc_client_handler handler,
                                           int min, int max) {
  if (!handler) return;
  static_cast<UpaGrpcClient*>(handler)->SetReconnectBackoff(min, max);
}

int upa_grpc_client_send(upa_grpc_client_handler handler,
                         upa_grpc_message_t* msg) {
  if (!handler || !msg) return -1;
  int rv =
      static_cast<UpaGrpcClient*>(handler)->Send(static_cast<Message*>(msg));
  if (rv == 0) upa_grpc_message_destroy(msg);
  return rv;
}

// C API of OcsInterfaceServer
upa_grpc_server_handler upa_grpc_server_create(
    const char* ip, unsigned short port, const char* name,
    upa_grpc_msg_type_e msg_type, upa_grpc_server_on_read_f callback) {
  if (!ip || !name || !callback) return NULL;
  return new UpaGrpcServer(ip, port, name, static_cast<MsgType>(msg_type),
                           callback);
}
/*
upa_grpc_server_handler upa_grpc_server_create(
    const char* target, const char* name, upa_grpc_msg_type_e msg_type,
    upa_grpc_server_on_read_f callback) {
  if (!target || !name || !callback) return NULL;
  return new UpaGrpcServer(target, name, static_cast<MsgType>(msg_type),
                           callback);
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

void upa_grpc_server_stop_reactor(upa_grpc_server_handler handler,
                                  upa_grpc_server_reactor_t* ract) {
  if (!handler) return;
  static_cast<UpaGrpcServer*>(handler)->StopReactor(
      static_cast<UpaGrpcServerReactorClass*>(ract));
}
void upa_grpc_server_stop_reactor_n(upa_grpc_server_handler handler,
                                    const char* ract_name) {
  if (!handler) return;
  static_cast<UpaGrpcServer*>(handler)->StopReactor(ract_name);
}
void upa_grpc_server_stop_reactor_i(upa_grpc_server_handler handler,
                                    int ract_idx) {
  if (!handler) return;
  static_cast<UpaGrpcServer*>(handler)->StopReactor(ract_idx);
}

const char* upa_grpc_server_get_addr(upa_grpc_server_handler handler) {
  if (!handler) return "";
  return static_cast<UpaGrpcServer*>(handler)->GetAddr();
}

const char* upa_grpc_server_get_name(upa_grpc_server_handler handler) {
  if (!handler) return "";
  return static_cast<UpaGrpcServer*>(handler)->GetName();
}

int upa_grpc_server_get_msg_type(upa_grpc_server_handler handler) {
  if (!handler) return UPA_GRPC_MSG_TYPE_MIN-1;
  return static_cast<UpaGrpcServer*>(handler)->GetMsgType();
}
upa_grpc_server_on_read_f upa_grpc_server_get_on_read(
    upa_grpc_server_handler handler) {
  if (!handler) return NULL;
  return static_cast<UpaGrpcServer*>(handler)->GetOnRead();
}

upa_grpc_server_on_accept_f upa_grpc_server_get_on_accept(
    upa_grpc_server_handler handler) {
  if (!handler) return NULL;
  return static_cast<UpaGrpcServer*>(handler)->GetOnAccept();
}
void upa_grpc_server_set_on_accept(upa_grpc_server_handler handler,
                                   upa_grpc_server_on_accept_f callback) {
  if (!handler) return;
  static_cast<UpaGrpcServer*>(handler)->SetOnAccept(callback);
}

upa_grpc_server_on_close_f upa_grpc_server_get_on_close(
    upa_grpc_server_handler handler) {
  if (!handler) return NULL;
  return static_cast<UpaGrpcServer*>(handler)->GetOnClose();
}
void upa_grpc_server_set_on_close(upa_grpc_server_handler handler,
                                  upa_grpc_server_on_close_f callback) {
  if (!handler) return;
  static_cast<UpaGrpcServer*>(handler)->SetOnClose(callback);
}

void* upa_grpc_server_get_user_data(upa_grpc_server_handler handler) {
  if(!handler) return NULL;
  return static_cast<UpaGrpcServer*>(handler)->GetUserData();
}
void upa_grpc_server_set_user_data(upa_grpc_server_handler handler,
                                   void* user_data) {
  if (!handler) return;
  static_cast<UpaGrpcServer*>(handler)->SetUserData(user_data);
}

upa_grpc_server_reactor_t* upa_grpc_server_get_reactor_n(
    upa_grpc_server_handler handler, const char* ract_name) {
  if (!handler || !ract_name) return NULL;
  return static_cast<UpaGrpcServer*>(handler)->GetReactor(ract_name);
}
upa_grpc_server_reactor_t* upa_grpc_server_get_reactor_i(
    upa_grpc_server_handler handler, int ract_idx) {
  if (!handler) return NULL;
  return static_cast<UpaGrpcServer*>(handler)->GetReactor(ract_idx);
}

const char* upa_grpc_server_get_reactor_name(upa_grpc_server_handler handler,
                                             upa_grpc_server_reactor_t* ract) {
  if (!handler || !ract) return "";
  return static_cast<UpaGrpcServer*>(handler)
      ->GetReactorName(static_cast<UpaGrpcServerReactorClass*>(ract));
}

int upa_grpc_server_get_reactor_count(upa_grpc_server_handler handler) {
  if (!handler) return 0;
  return static_cast<UpaGrpcServer*>(handler)->GetReactorCount();
}

int upa_grpc_server_send(upa_grpc_server_handler handler,
                         upa_grpc_message_t* msg,
                         upa_grpc_server_reactor_t* ract) {
  if (!handler) return -1;
  int rv = static_cast<UpaGrpcServer*>(handler)->Send(
      static_cast<Message*>(msg),
      static_cast<UpaGrpcServerReactorClass*>(ract));
  if (rv == 0) upa_grpc_message_destroy(msg);
  return rv;
}
int upa_grpc_server_send_n(upa_grpc_server_handler handler,
                           upa_grpc_message_t* msg, const char* ract_name) {
  if (!handler) return -1;
  int rv = static_cast<UpaGrpcServer*>(handler)->Send(
      static_cast<Message*>(msg), ract_name);
  if (rv == 0) upa_grpc_message_destroy(msg);
  return rv;
}
int upa_grpc_server_send_i(upa_grpc_server_handler handler,
                           upa_grpc_message_t* msg, int ract_idx) {
  if (!handler) return -1;
  int rv = static_cast<UpaGrpcServer*>(handler)->Send(
      static_cast<Message*>(msg), ract_idx);
  if (rv == 0) upa_grpc_message_destroy(msg);
  return rv;
}

}  // extern "C"