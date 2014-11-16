#ifndef CCEPH_CLIENT_H
#define CCEPH_CLIENT_H

struct client {
  int (* write)(const char*, const char*, size_t, uint_64);
  int (* write_full)(const char*, const char*, size_t);
  int (* append)(const char*, const char*, size_t);
};

#endif
