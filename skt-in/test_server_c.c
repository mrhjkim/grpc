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

char _g_ip[32] = { "0.0.0.0"};
unsigned short _g_port = 50051;

int OnHeartbeatRequest(const void* req, void* res) {
  if (!req || !res) {
    printf("%s : Invalid parameters. req=%p, res=%p\n", __func__, req, res);
    return -1;
  }

  const HeartbeatRequest_t* request = (const HeartbeatRequest_t*)req;
  HeartbeatResponse_t* response = (HeartbeatResponse_t*)res;

  const char* node_name = HeartbeatRequest_get_node_name(request);
  HeartbeatResponse_set_node_name(response, "CNCF-OCS-PROXY-1");
  HeartbeatResponse_set_status(response, SYSTEM_STATUS_ACTIVE);
  return 0;
}

int OnServiceRequest(const void* req, void* res) {
  if (!req || !res) {
    printf("%s : Invalid parameters. req=%p, res=%p\n", __func__, req, res);
    return -1;
  }

  const ServiceRequest_t* request = (const ServiceRequest_t*)req;
  ServiceResponse_t* response = (ServiceResponse_t*)res;

  struct TestData data;
  size_t data_size;
  const char* data_ptr = ServiceRequest_get_data(request, &data_size);
  if (data_size == sizeof(data)) {
    memcpy(&data, data_ptr, data_size);
    printf(" Request TestData = [%d, %lf, %s](%lu)\n", data.id, data.value,
           data.name, sizeof(data));
  }

  ServiceResponse_set_message_type(response,
                                   ServiceRequest_get_message_type(request));
  ServiceResponse_set_source_node_name(
      response, ServiceRequest_get_source_node_name(request));
  ServiceResponse_set_destination_node_name(response, "CNCF-OCS-PROXY-1");
  ServiceResponse_set_user_key(response, ServiceRequest_get_user_key(request));
  char sess_id[128];
  snprintf(sess_id, sizeof(sess_id), "%s.1234567890",
           ServiceRequest_get_user_key(request));
  ServiceResponse_set_session_id(response, sess_id);
  strcpy(data.name, "result");
  printf(" Response TestData = [%d, %lf, %s](%lu)\n", data.id, data.value,
         data.name, sizeof(data));
  ServiceResponse_set_data(response, (const char*)&data, sizeof(data));

  return 0;
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
  if ((rv = _get_opt(argc, argv)) < 0) return rv;
  OcsInterfaceServer_run(_g_ip, _g_port, OnHeartbeatRequest, OnServiceRequest);
  return 0;
}
