#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include "include/upa_grpc.h"

ABSL_FLAG(std::string, ip, "0.0.0.0", "Server ip address for the service");
ABSL_FLAG(uint16_t, port, 50051, "Server port for the service");
ABSL_FLAG(int, wait_time, 0, "Wait time(sec) before send response.");

int _g_wait_time = 0;

using namespace upa_grpc;

#pragma pack(push, 1)  // 패딩 없이 정렬
struct TestData {
  int id;
  float value;
  char name[16];
};
#pragma pack(pop)

int onServiceRequest(const void* req, void* owner, void* ract) {
  if (!req || !owner || !ract) return -1;

  if (_g_wait_time > 0 && _g_wait_time <= 3600) {
    std::cout << "Sleep " << _g_wait_time << "seconds for test." << std::endl;
    sleep(_g_wait_time);
  }

  const Message* request = static_cast<const Message*>(req);
  UpaGrpcServer* server = static_cast<UpaGrpcServer*>(owner);
  grpc::ServerBidiReactor<Message, Message>* reactor =
      static_cast<grpc::ServerBidiReactor<Message, Message>*>(ract);

  std::cout << "onServiceRequest... name[" << server->GetName()
            << "], msg_type[" << MsgTypeStr(server->GetMsgType()) << "], peer["
            << server->GetReactorName(reactor) << "]" << std::endl;

  PrintMessage(request, false);

  TestData data;
  size_t data_size;
  const char* data_ptr = GetData(request, &data_size);
  if (data_size == sizeof(TestData)) {
    memcpy(&data, data_ptr, data_size);
    std::cout << " Request TestData = [" << data.id << ", " << data.value
              << ", " << data.name << "](" << sizeof(TestData) << ")"
              << std::endl;
  }

  Message response;
  SetMsgType(&response, GetMsgType(request));
  SetSrcId(&response, GetSrcId(request));
  SetDstId(&response, "CNCF-OCS-PROXY-1");
  SetRouteKey(&response, GetRouteKey(request));
  std::string sess_key = GetRouteKey(&response);
  sess_key.append(".1234567890");
  SetSessionKey(&response, sess_key.c_str());
  strcpy(data.name, "result");
  std::cout << " Response TestData = [ " << data.id << ", " << data.value
            << ", " << data.name << "](" << sizeof(TestData) << ")"
            << std::endl;
  SetData(&response, reinterpret_cast<const char*>(&data), sizeof(TestData));

  PrintMessage(&response, true);

  server->Send(&response, reactor);
  sleep(3);
  server->Send(&response, reactor);

  return 0;
}

int onAccept(void* owner, void* ract) {
  if (!owner || !ract) return -1;

  UpaGrpcServer* server = static_cast<UpaGrpcServer*>(owner);
  grpc::ServerBidiReactor<Message, Message>* reactor =
      static_cast<grpc::ServerBidiReactor<Message, Message>*>(ract);

  std::cout << "onAccept... name[" << server->GetName()
            << "], msg_type[" << MsgTypeStr(server->GetMsgType()) << "], peer["
            << server->GetReactorName(reactor) << "]" << std::endl;

  return 0;
}

int onClose(void* owner, void* ract) {
  if (!owner || !ract) return -1;

  UpaGrpcServer* server = static_cast<UpaGrpcServer*>(owner);
  grpc::ServerBidiReactor<Message, Message>* reactor =
      static_cast<grpc::ServerBidiReactor<Message, Message>*>(ract);

  std::cout << "onClose... name[" << server->GetName()
            << "], msg_type[" << MsgTypeStr(server->GetMsgType()) << "], peer["
            << server->GetReactorName(reactor) << "]" << std::endl;

  return 0;
}

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  _g_wait_time = absl::GetFlag(FLAGS_wait_time);
  UpaGrpcServer server(absl::GetFlag(FLAGS_ip), absl::GetFlag(FLAGS_port),
                       "UPA_GRPC:SERVER", MSG_TYPE_DBIF, onServiceRequest);
  server.SetOnAccept(onAccept);
  server.SetOnClose(onClose);

#if 0
  server.Run();
#else
  while (1) {
    server.Start();
    sleep(60);
    server.Stop();
    sleep(5);
  }
#endif

  return 0;
}
