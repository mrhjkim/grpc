#include <iostream>
#include <memory>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include "include/ocs_interface.h"

ABSL_FLAG(std::string, ip, "0.0.0.0", "Server ip address for the service");
ABSL_FLAG(uint16_t, port, 50051, "Server port for the service");

using namespace ocs_interface;

#pragma pack(push, 1)  // 패딩 없이 정렬
struct TestData {
  int id;
  float value;
  char name[16];
};
#pragma pack(pop)

int OnHeartbeatRequest(const void* req, void* res) {
  if( !req || !res ) return -1;

  const HeartbeatRequest* request = static_cast<const HeartbeatRequest*>(req);
  HeartbeatResponse* response = static_cast<HeartbeatResponse*>(res);

  SetNodeName(response, "CNCF-OCS-PROXY-1");
  SetStatus(response, SYSTEM_STATUS_ACTIVE);
  return 0;
}

int OnServiceRequest(const void* req, void* res) {
  if( !req || !res ) return -1;

  const ServiceRequest* request = static_cast<const ServiceRequest*>(req);
  ServiceResponse* response = static_cast<ServiceResponse*>(res);

  TestData data;
  size_t data_size;
  const char* data_ptr = GetData(request, &data_size);
  if (data_size == sizeof(TestData)) {
    memcpy(&data, data_ptr, data_size);
    std::cout << " Request TestData = [" << data.id << ", " << data.value
              << ", " << data.name << "](" << sizeof(TestData) << ")"
              << std::endl;
  }

  SetMessageType(response, GetMessageType(request));
  SetSourceNodeName(response, GetSourceNodeName(request));
  SetDestinationNodeName(response, "CNCF-OCS-PROXY-1");
  SetUserKey(response, GetUserKey(request));
  std::string sess_id = GetUserKey(response);
  sess_id.append(".1234567890");
  SetSessionId(response, sess_id.c_str());
  strcpy(data.name, "result");
  std::cout << " Response TestData = [ " << data.id << ", " << data.value
            << ", " << data.name << "](" << sizeof(TestData) << ")"
            << std::endl;
  SetData(response, reinterpret_cast<const char*>(&data), sizeof(TestData));

  return 0;
}

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  OcsInterfaceServerRun(absl::GetFlag(FLAGS_ip), absl::GetFlag(FLAGS_port),
                        OnHeartbeatRequest, OnServiceRequest);
  return 0;
}
