#include "client/client.h"

static struct cceph_client *clients[MAX_CLIENT_TYPES] = {0};

void cceph_client_register(enum CCEPH_CLIENT_TYPE type, struct cceph_client *client) {
    clients[type] = client;
}

void cceph_client_unregister(enum CCEPH_CLIENT_TYPE type) {
    clients[type] = 0;
}
