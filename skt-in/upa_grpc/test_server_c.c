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

char _g_ip[32] = { "0.0.0.0"};
unsigned short _g_port = 50051;
int _g_wait_time = 0;

int onServiceRequest(const void* req, void* res) {
  if (!req || !res) {
    printf("%s : Invalid parameters. req=%p, res=%p\n", __func__, req, res);
    return -1;
  }

  if(_g_wait_time > 0 ) {
    printf("Sleep %d seconds for test.\n", _g_wait_time);
    sleep(_g_wait_time);
  }

  const upa_grpc_message_t* request = (const upa_grpc_message_t*)req;
  upa_grpc_message_t* response = (upa_grpc_message_t*)res;

  struct TestData data;
  size_t data_size;
  const char* data_ptr = upa_grpc_message_get_data(request, &data_size);
  if (data_size == sizeof(data)) {
    memcpy(&data, data_ptr, data_size);
    printf(" Request TestData = [%d, %lf, %s](%lu)\n", data.id, data.value,
           data.name, sizeof(data));
  }

  upa_grpc_message_set_msg_type(response,
                                upa_grpc_message_get_msg_type(request));
  upa_grpc_message_set_src_id(response, upa_grpc_message_get_src_id(request));
  upa_grpc_message_set_dst_id(response, "CNCF-OCS-PROXY-1");
  upa_grpc_message_set_route_key(response,
                                 upa_grpc_message_get_route_key(request));
  char sess_key[128];
  snprintf(sess_key, sizeof(sess_key), "%s.1234567890",
           upa_grpc_message_get_route_key(request));
  upa_grpc_message_set_session_key(response, sess_key);
  strcpy(data.name, "result");
  printf(" Response TestData = [%d, %lf, %s](%lu)\n", data.id, data.value,
         data.name, sizeof(data));
  upa_grpc_message_set_data(response, (const char*)&data, sizeof(data));

  return 0;
}

int _get_opt(int argc, char** argv) {
  int rv = 0;
  long val_l;
  unsigned char c;

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
        if (val_l < 0 || val_l >= 3600) {
          printf("Invalid wait time. val='%s'\n", optarg);
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
        "\nUsage: %s [-i IP] [-p Port] [-h|-H]\n"
        "    \"-i\"  listen IP address  (Default : 0.0.0.0)\n"
        "    \"-p\"  listen port number (Default : 50051)\n"
        "    \"-t\"  wait time(sec) before send response (Default: 0)\n",
        pname);
  }
  return rv;
}

int main(int argc, char** argv) {
  int rv;
  if ((rv = _get_opt(argc, argv)) < 0) return rv;
  upa_grpc_server_handler handler = upa_grpc_server_create(
      _g_ip, _g_port, UPA_GRPC_MSG_TYPE_DBIF, onServiceRequest);
#if 0
  upa_grpc_server_run(handler);
#else
  while (1) {
    upa_grpc_server_start(handler);
    sleep(10);
    upa_grpc_server_stop(handler);
    sleep(5);
  }
#endif
  return 0;
}
