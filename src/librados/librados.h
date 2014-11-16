#ifndef CCEPH_LIBRADOS_H
#define CCEPH_LIBRADOS_H
/**
 * @typedef rados_t
 *
 * A handle for interacting with a RADOS cluster. It encapsulates all
 * RADOS client configuration, including username, key for
 * authentication, logging, and debugging. Talking different clusters
 * -- or to the same cluster with different users -- requires
 * different cluster handles.
 */
typedef void *rados_t;

/**
 * @struct rados_pool_stat_t
 * Usage information for a pool.
 */
struct rados_pool_stat_t {
  /// space used in bytes
  uint64_t num_bytes;
  /// number of objects in the pool
  uint64_t num_objects;
};

/**
 * Create a handle for communicating with a RADOS cluster.
 *
 * @param cluster where to store the handle
 * @returns 0 on success, negative error code on failure
 */
int rados_create(rados_t *cluster);

/**
 * @typedef rados_ioctx_t
 *
 * An io context encapsulates a few settings for all I/O operations
 * done on it:
 * - pool - set when the io context is created (see rados_ioctx_create())
 * - snapshot context for writes (see
 *   rados_ioctx_selfmanaged_snap_set_write_ctx())
 * - snapshot id to read from (see rados_ioctx_snap_set_read())
 * - object locator for all single-object operations (see
 *   rados_ioctx_locator_set_key())
 *
 * @warning changing any of these settings is not thread-safe -
 * librados users must synchronize any of these changes on their own,
 * or use separate io contexts for each thread
 */
typedef void *rados_ioctx_t;

/**
 * Connect to the cluster.
 *
 * @note BUG: Before calling this, calling a function that communicates with the
 * cluster will crash.
 *
 * @pre The cluster handle is configured with at least a monitor
 * address. If cephx is enabled, a client name and secret must also be
 * set.
 *
 * @post If this succeeds, any function in librados may be used
 *
 * @param cluster The cluster to connect to.
 * @returns 0 on sucess, negative error code on failure
 */
int rados_connect(rados_t cluster);

/**
 * Disconnects from the cluster.
 *
 * For clean up, this is only necessary after rados_connect() has
 * succeeded.
 *
 * @warning This does not guarantee any asynchronous writes have
 * completed. To do that, you must call rados_aio_flush() on all open
 * io contexts.
 *
 * @post the cluster handle cannot be used again
 *
 * @param cluster the cluster to shutdown
 */
void rados_shutdown(rados_t cluster);

/**
 * List pools
 *
 * Gets a list of pool names as NULL-terminated strings.  The pool
 * names will be placed in the supplied buffer one after another.
 * After the last pool name, there will be two 0 bytes in a row.
 *
 * If len is too short to fit all the pool name entries we need, we will fill
 * as much as we can.
 *
 * @param cluster cluster handle
 * @param buf output buffer
 * @param len output buffer length
 * @returns length of the buffer we would need to list all pools
 */
int rados_pool_list(rados_t cluster, char *buf, size_t len);

/**
 * Create an io context
 *
 * The io context allows you to perform operations within a particular
 * pool. For more details see rados_ioctx_t.
 *
 * @param cluster which cluster the pool is in
 * @param pool_name name of the pool
 * @param ioctx where to store the io context
 * @returns 0 on success, negative error code on failure
 */
int rados_ioctx_create(rados_t cluster, const char *pool_name, rados_ioctx_t *ioctx);

/**
 * The opposite of rados_ioctx_create
 *
 * This just tells librados that you no longer need to use the io context.
 *
 * @param io the io context to dispose of
 */
void rados_ioctx_destroy(rados_ioctx_t io);

/**
 * Get pool usage statistics
 *
 * Fills in a rados_pool_stat_t after querying the cluster.
 *
 * @param io determines which pool to query
 * @param stats where to store the results
 * @returns 0 on success, negative error code on failure
 */
int rados_ioctx_pool_stat(rados_ioctx_t io, struct rados_pool_stat_t *stats);

/**
 * Get the id of a pool
 *
 * @param cluster which cluster the pool is in
 * @param pool_name which pool to look up
 * @returns id of the pool
 * @returns -ENOENT if the pool is not found
 */
int64_t rados_pool_lookup(rados_t cluster, const char *pool_name);

/**
 * Create a pool with default settings
 *
 * The default crush rule is rule 0.
 *
 * @param cluster the cluster in which the pool will be created
 * @param pool_name the name of the new pool
 * @returns 0 on success, negative error code on failure
 */
int rados_pool_create(rados_t cluster, const char *pool_name);


/**
 * Remove a pool and all data inside it
 *
 * The pool is removed from the cluster immediately,
 * but the actual data is deleted in the background.
 *
 * @param cluster the cluster the pool is in
 * @param pool_name which pool to delete
 * @returns 0 on success, negative error code on failure
 */
int rados_pool_remove(rados_t cluster, const char *pool_name);


/**
 * Write *len* bytes from *buf* into the *oid* object, starting at
 * offset *off*. The value of *len* must be <= UINT_MAX/2.
 *
 * @note This will never return a positive value not equal to len.
 * @param io the io context in which the write will occur
 * @param oid name of the object
 * @param buf data to write
 * @param len length of the data, in bytes
 * @param off byte offset in the object to begin writing at
 * @returns 0 on success, negative error code on failure
 */
int rados_write(rados_ioctx_t io, const char *oid, const char *buf, size_t len, uint64_t off);

/**
 * Write *len* bytes from *buf* into the *oid* object. The value of
 * *len* must be <= UINT_MAX/2.
 *
 * The object is filled with the provided data. If the object exists,
 * it is atomically truncated and then written.
 *
 * @param io the io context in which the write will occur
 * @param oid name of the object
 * @param buf data to write
 * @param len length of the data, in bytes
 * @returns 0 on success, negative error code on failure
 */
int rados_write_full(rados_ioctx_t io, const char *oid, const char *buf, size_t len);

/**
 * Append *len* bytes from *buf* into the *oid* object. The value of
 * *len* must be <= UINT_MAX/2.
 *
 * @param io the context to operate in
 * @param oid the name of the object
 * @param buf the data to append
 * @param len length of buf (in bytes)
 * @returns 0 on success, negative error code on failure
 */
int rados_append(rados_ioctx_t io, const char *oid, const char *buf, size_t len);

/**
 * Read data from an object
 *
 * The io context determines the snapshot to read from, if any was set
 * by rados_ioctx_snap_set_read().
 *
 * @param io the context in which to perform the read
 * @param oid the name of the object to read from
 * @param buf where to store the results
 * @param len the number of bytes to read
 * @param off the offset to start reading from in the object
 * @returns number of bytes read on success, negative error code on
 * failure
 */
int rados_read(rados_ioctx_t io, const char *oid, char *buf, size_t len, uint64_t off);

/**
 * Delete an object
 *
 * @note This does not delete any snapshots of the object.
 *
 * @param io the pool to delete the object from
 * @param oid the name of the object to delete
 * @returns 0 on success, negative error code on failure
 */
int rados_remove(rados_ioctx_t io, const char *oid);

/**
 * Resize an object
 *
 * If this enlarges the object, the new area is logically filled with
 * zeroes. If this shrinks the object, the excess data is removed.
 *
 * @param io the context in which to truncate
 * @param oid the name of the object
 * @param size the new size of the object in bytes
 * @returns 0 on success, negative error code on failure
 */
int rados_truncate(rados_ioctx_t io, const char *oid, uint64_t size);

#endif
