============================================
CCeph - a pure c version of ceph
============================================

Architecture
------------

1.Pure C, No STL, No Boost. 

2.No complex dependency between components

3.High Performance is the most important issue (for SATA/PCIe SSD)


RADOS
-----------

1.EC

2.NWR Supported (PG Log is best for NWR) -> Degerde WRITE



FS
-----------

1.Ext4 Based FS

2.MultiThread MDS

3.Sample Tree-Partition MDS

4.Native Windows Client


RBD
-----------

1.Common Protocol Supoort: iSCSI, FC, SRP


QA
----------

1.Full UnitedTest

2.Producation Enviroment Test

3.Different TestSuit For Different Proposal and Community Cooperation


Operations
----------

1.Intergreted with IT/Cloud System
