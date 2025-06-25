#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "include/ocs_interface_capi.h"

struct TestData {
  int id;
  float value;
  char name[16];
};

char _g_ip[32] = {"127.0.0.1"};
unsigned short _g_port = 50051;

void OnHeartbeatResponse(int rv, void* res) {
  printf("OnHeartbeatResponse ... rv=%d\n", rv);

  if (res) {
    HeartbeatResponse_t* response = (HeartbeatResponse_t*)res;
    // ...
  }
}

void OnServiceResponse(int rv, void* res) {
  printf("OnServiceResponse ... rv=%d\n", rv );

  if (!rv && res) {
    ServiceResponse_t* response = (ServiceResponse_t*)res;
    size_t data_size;
    const char* data = ServiceResponse_get_data(response, &data_size);
    if (data_size == sizeof(struct TestData)) {
      struct TestData resData;
      memcpy(&resData, data, data_size);
      printf(" Response TestData = [%d, %lf, %s]\n", resData.id, resData.value,
             resData.name);
    }
  }
}

int _get_opt(int argc, char** argv) {
  int rv = 0;
  long val_l;
  char c;

  while ((c = getopt(argc, argv, "i:p:hH")) != 0xFF) {
    switch (c) {
      case 'i':
        if (strlen(optarg) >= sizeof(_g_ip)) {
          printf("Invalid ip address. val='%s'\n", optarg);
          rv = -1;
          goto final;
        }
        strcpy(_g_ip, optarg);
        break;
      case 'p':
        val_l = strtol(optarg, NULL, 0);
        if (val_l <= 0 || val_l > 65535) {
          printf("Invalid port number. val='%s'\n", optarg);
          rv = -1;
          goto final;
        }
        _g_port = (unsigned short)val_l;
        break;
      case 'h':
      case 'H':
        rv = -2;
        goto final;
    }
  }

final:
  if (rv < 0) {
    char* pname = strrchr(argv[0], '/');
    if (pname) {
      pname = pname + 1;
    } else {
      pname = argv[0];
    }
    printf(
        "\nUsage: %s [-i IP] [-p Port] [-h|-H]\n"
        "    \"-i\"  listen IP address  (Default : 0.0.0.0)\n"
        "    \"-p\"  listen port number (Default : 50051)\n",
        pname);
  }
  return rv;
}

int main(int argc, char** argv) {
  int rv;
  char target_str[128];

  if ((rv = _get_opt(argc, argv)) < 0) return rv;
  sprintf(target_str, "%s:%d", _g_ip, _g_port);
  OcsInterfaceClientHandler handler = OcsInterfaceClient_create(target_str);

  while (!OcsInterfaceClient_wait_for_connected(handler, 5)) {
    printf("Wating for channel to be ready...\n");
  }
  printf("Current channel status is %d.\n",
         OcsInterfaceClient_get_state(handler));

  OcsInterfaceClientContext_t* context_h = OcsInterfaceClientContext_alloc();
  HeartbeatResponse_t* response_h = HeartbeatResponse_alloc();
  rv = OcsInterfaceClient_heartbeat(handler, context_h, "SCPAS2P",
                                        response_h, OnHeartbeatResponse);

  usleep(100000);

  OcsInterfaceClientContext_t* context_s = OcsInterfaceClientContext_alloc();
  ServiceRequest_t* request_s = ServiceRequest_alloc();
  ServiceRequest_set_message_type(request_s, MESSAGE_TYPE_SCPAS);
  ServiceRequest_set_source_node_name(request_s, "SCPAS2P");
  ServiceRequest_set_user_key(request_s, "01067003951");
  struct TestData data = {1, 100, "test-request"};
  printf(" Request TestData = [%d, %lf, %s]\n", data.id, data.value, data.name);
  ServiceRequest_set_data(request_s, (void*)&data, sizeof(data));
  ServiceResponse_t* response_s = ServiceResponse_alloc();
  rv = OcsInterfaceClient_inquiry(handler, context_s, request_s, response_s,
                                  OnServiceResponse);

  usleep(100000);

  rv = OcsInterfaceClient_wait(handler, context_h);
  printf("Wait HeartbeatResponse message. rv=%d\n", rv);
  OcsInterfaceClientContext_destroy(context_h);
  HeartbeatResponse_destroy(response_h);

  rv = OcsInterfaceClient_wait(handler, context_s);
  printf("Wait ServiceResponse message. rv=%d\n", rv);
  OcsInterfaceClientContext_destroy(context_s);
  ServiceRequest_destroy(request_s);
  ServiceResponse_destroy(response_s);

  return 0;
}
