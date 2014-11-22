#include "src/client/client.h"

static struct cceph_client* clients = NULL;

int cceph_client_register(CCEPH_CLIENT_TYPE type, struct cceph_client *client) {
    if (clients == NULL) {
        clients = (struct cceph_client *)malloc(sizeof(struct cceph_client *) * CCEPH_CLIENT_TYPE_MAX);
    }

    clients[type] = client;
}
void cceph_client_unregister(CCEPH_CLIENT_TYPE type) {
    clients[type] = NULL;
}
