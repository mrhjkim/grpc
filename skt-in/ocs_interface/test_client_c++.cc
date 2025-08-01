#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include "include/ocs_interface.h"

ABSL_FLAG(std::string, target, "localhost:50051", "Server address");

using namespace ocs_interface;

#pragma pack(push, 1)  // 패딩 없이 정렬
struct TestData {
  int id;
  float value;
  char name[16];
};
#pragma pack(pop)

void OnHeartbeatResponse(int rv, void* res) {
  std::cout << "OnHeartbeatResponse ... " << "rv=" << rv << "\n";

  if (res) {
    HeartbeatResponse* response = static_cast<HeartbeatResponse*>(res);
    // ...
  }
}

void OnServiceResponse(int rv, void* res) {
  std::cout << "OnServiceResponse ... " << "rv=" << rv << "\n";

  if (!rv && res) {
    ServiceResponse* response = static_cast<ServiceResponse*>(res);
    size_t data_size;
    const char* data = GetData(response, &data_size);
    if (data_size == sizeof(TestData)) {
      TestData resData;
      memcpy(&resData, data, data_size);
      std::cout << " Response TestData = [" << resData.id << ", " << resData.value
                << ", " << resData.name << "]" << std::endl;
    }
  }
}

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  std::string target_str = absl::GetFlag(FLAGS_target);
  OcsInterfaceClient client(target_str);

  while (!client.WaitForConnected(5)) {
    std::cout << "Wating for channel to be ready..." << std::endl;
  }
  std::cout << "Current channel status is " << client.GetState() << "."
            << std::endl;

  OcsInterfaceClientContext context_h;
  HeartbeatResponse heartbeatResponse;
  int rv =
      client.Heartbeat(&context_h, "SCPAS2P", &heartbeatResponse, &OnHeartbeatResponse);

  std::thread t_h([&client, &context_h]() {
    int rv = client.Wait(&context_h);
    std::cout << "Wait HeartbeatResponse message. rv=" << rv << std::endl;
  });

  usleep(100000);

  OcsInterfaceClientContext context_s;
  ServiceRequest serviceRequest;
  SetMessageType(&serviceRequest, MESSAGE_TYPE_SCPAS);
  SetSourceNodeName(&serviceRequest, "SCPAS2P");
  SetUserKey(&serviceRequest, "01067003951");
  TestData data = {1, 100, "test-request"};
  std::cout << " Request TestData = [" << data.id << ", " << data.value << ", "
            << data.name << "]" << std::endl;
  SetData(&serviceRequest, reinterpret_cast<const char*>(&data),
          sizeof(TestData));
  ServiceResponse serviceResponse;
  rv = client.Inquiry(&context_s, &serviceRequest, &serviceResponse,
                      &OnServiceResponse);

  std::thread t_s([&client, &context_s]() {
    int rv = client.Wait(&context_s);
    std::cout << "Wait ServiceResponse message. rv=" << rv << std::endl;
  });

  t_h.join();
  t_s.join();

  return 0;
}
