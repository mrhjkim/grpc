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

using namespace upa_grpc;

#pragma pack(push, 1)  // 패딩 없이 정렬
struct TestData {
  int id;
  float value;
  char name[16];
};
#pragma pack(pop)

void onServiceResponse(void* res) {
  std::cout << "onServiceResponse ...\n";

  if (!res) return;

  Message* response = static_cast<Message*>(res);
  size_t data_size;

  PrintMessage(response, false);

  const char* data = GetData(response, &data_size);
  if (data_size == sizeof(TestData)) {
    TestData resData;
    memcpy(&resData, data, data_size);
    std::cout << " Response TestData = [" << resData.id << ", " << resData.value
              << ", " << resData.name << "]" << std::endl;
  }
}

void sendTestMessage(UpaGrpcClient* client) {
  while (!client->WaitForConnected(5)) {
    std::cout << "Wating for channel to be ready..." << std::endl;
    client->StopReactor(true);
  }
  std::cout << "Current channel status is " << client->GetState() << "."
            << std::endl;

  Message request;
  SetSrcId(&request, "SCPAS2P");
  SetRouteKey(&request, "01067003951");
  TestData data = {1, 100, "test-request"};
  std::cout << " Request TestData = [" << data.id << ", " << data.value << ", "
            << data.name << "]" << std::endl;
  SetData(&request, reinterpret_cast<const char*>(&data), sizeof(TestData));

  PrintMessage(&request, true);

  int rv = client->Send(&request);
}

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  std::string target_str = absl::GetFlag(FLAGS_target);

  UpaGrpcClient client(target_str, MSG_TYPE_DBIF, onServiceResponse);
  client.SetReconnectBackoff(5000, 5000);

  int rv = client.Start();
  if (rv < 0) return rv;
  sleep(5);
  sendTestMessage(&client);
  sleep(5);
  sendTestMessage(&client);
  sleep(1);
  client.Stop();

  sleep(5);

  rv = client.Start();
  if (rv < 0) return rv;
  sleep(5);
  sendTestMessage(&client);
  sleep(1);
  client.Stop();

  return 0;
}
