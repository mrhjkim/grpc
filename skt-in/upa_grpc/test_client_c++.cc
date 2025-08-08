#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include "include/upa_grpc.h"

ABSL_FLAG(std::string, target, "localhost:50051", "Server address");
ABSL_FLAG(int, wait_time, 0, "Wait response time (sec)");

using namespace upa_grpc;

#pragma pack(push, 1)  // 패딩 없이 정렬
struct TestData {
  int id;
  float value;
  char name[16];
};
#pragma pack(pop)

void onServiceResponse(int rv, void* res) {
  std::cout << "onServiceResponse ... " << "rv=" << rv << "\n";

  if (!rv && res) {
    Message* response = static_cast<Message*>(res);
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

void sendTestMessage(UpaGrpcClient* client, int wait_time) {
  while (!client->WaitForConnected(5)) {
    std::cout << "Wating for channel to be ready..." << std::endl;
  }
  std::cout << "Current channel status is " << client->GetState() << "."
            << std::endl;

  UpaGrpcClientContext context(wait_time);
  Message request;
  SetSrcId(&request, "SCPAS2P");
  SetRouteKey(&request, "01067003951");
  TestData data = {1, 100, "test-request"};
  std::cout << " Request TestData = [" << data.id << ", " << data.value << ", "
            << data.name << "]" << std::endl;
  SetData(&request, reinterpret_cast<const char*>(&data), sizeof(TestData));
  Message response;
  int rv = client->Send(&context, &request, &response, &onServiceResponse);

  std::thread t([&context]() {
    int rv = context.Wait();
    std::cout << "Wait ServiceResponse message. rv=" << rv << std::endl;
  });

  t.join();
}

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  std::string target_str = absl::GetFlag(FLAGS_target);
  int wait_time = absl::GetFlag(FLAGS_wait_time);

  UpaGrpcClient client(target_str, MSG_TYPE_DBIF);
  client.SetReconnectBackoff(5000, 5000);

  int rv = client.Start();
  if (rv < 0) return rv;
  sendTestMessage(&client, wait_time);
  client.Stop();

  sleep(3);

  rv = client.Start();
  if (rv < 0) return rv;
  sendTestMessage(&client, wait_time);
  client.Stop();

  return 0;
}
