# Abbreviation #

os     = object_store
osd    = object_storage_daemon
pg     = placement_group
oid    = object_id
pid    = pg_id
cid    = collect_id
msg    = messeage
msger  = messenger
smsger = server_messenger
trasn  = transaction


# Log ID #

Each log will have a log id which is stand for the process of this log.

When debuging, grep by log_id will take the whole process and ignore unrelated logs

Log Prefix is stand for the deamon, it follows the rule DEAMON_TYPE + ENTITY_NUMBER, here is some examples:

0101: mon.1
0201: osd.1
0302: client.2

