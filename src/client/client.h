#ifndef CCEPH_CLIENT_H
#define CCEPH_CLIENT_H

/**
 * Object Write Function Pointer
 * write len bytes from buf into the oid object, starting at
 * offset off. The value of len must be <= UINT_MAX/2.
 *
 * @note This will never return a positive value not equal to len.
 * @param 1: oid, name of the object
 * @param 2: buf, data to write
 * @param 3: length of the data, in bytes
 * @param 4: byte offset in the object to begin writing at
 * @returns 0 on success, negative error code on failure
 */
typedef int (*object_write_function_ptr)(const char*, const char*, size_t, uint_64);

struct client {
  object_write_function_ptr write_function;
  int (*object_write_function_ptr)(const char*, const char*, size_t, uint_64) write_function;
}

#endif
