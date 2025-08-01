#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "include/upa_grpc.capi.h"

struct TestData {
  int id;
  float value;
  char name[16];
};

char _g_ip[32] = {"127.0.0.1"};
unsigned short _g_port = 50051;
int _g_wait_time = 0;

void onServiceResponse(int rv, void* res) {
  printf("onServiceResponse ... rv=%d\n", rv );

  if (!rv && res) {
    upa_grpc_message_t* response = (upa_grpc_message_t*)res;
    size_t data_size;
    const char* data = upa_grpc_message_get_data(response, &data_size);
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

  while ((c = getopt(argc, argv, "i:p:t:hH")) != 0xFF) {
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
        case 't':
        val_l = strtol(optarg, NULL, 0);
        if (val_l < 0 || val_l > 3600) {
          printf("Invalid wait timer. val='%s'\n", optarg);
          rv = -1;
          goto final;
        }
        _g_wait_time = (int)val_l;
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
        "\nUsage: %s [-i IP] [-p Port] [-t wait-sec] [-h|-H]\n"
        "    \"-i\"  listen IP address  (Default : 0.0.0.0)\n"
        "    \"-p\"  listen port number (Default : 50051)\n"
        "    \"-t\"  wait response time (Default : 0 sec)\n",
        pname);
  }
  return rv;
}

void sendTestMessage(upa_grpc_client_handler handler, int wait_time) {
  while (!upa_grpc_client_wait_for_connected(handler, 5)) {
    printf("Wating for channel to be ready...\n");
  }
  printf("Current channel status is %d.\n", upa_grpc_client_get_state(handler));

  upa_grpc_client_context_t* context = upa_grpc_client_context_alloc(wait_time);
  upa_grpc_message_t* request = upa_grpc_message_alloc();
  upa_grpc_message_set_src_id(request, "SCPAS2P");
  upa_grpc_message_set_route_key(request, "01067003951");
  struct TestData data = {1, 100, "test-request"};
  printf(" Request TestData = [%d, %lf, %s]\n", data.id, data.value, data.name);
  upa_grpc_message_set_data(request, (void*)&data, sizeof(data));

  upa_grpc_message_t* response = upa_grpc_message_alloc();
  int rv = upa_grpc_client_send(handler, context, request, response,
                                onServiceResponse);

  rv = upa_grpc_client_context_wait(context);
  printf("Wait ServiceResponse message. rv=%d\n", rv);
  upa_grpc_client_context_destroy(context);
  upa_grpc_message_destroy(request);
  upa_grpc_message_destroy(response);
}

int main(int argc, char** argv) {
  int rv;

  if ((rv = _get_opt(argc, argv)) < 0) return rv;
  upa_grpc_client_handler handler =
      upa_grpc_client_create(_g_ip, _g_port, UPA_GRPC_MSG_TYPE_DBIF);

  rv = upa_grpc_client_start(handler);
  if (rv < 0) return rv;
  sendTestMessage(handler, _g_wait_time);
  upa_grpc_client_stop(handler);

  sleep(5);

  rv = upa_grpc_client_start(handler);
  if (rv < 0) return rv;
  sendTestMessage(handler, _g_wait_time);
  upa_grpc_client_stop(handler);

  return 0;
}
