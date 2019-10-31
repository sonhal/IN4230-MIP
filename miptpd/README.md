# MIPTPD

## Planning

#### MIPTPD - A transport daemon

Capabilities:
- Receives [data, portnumber, destination mip address] over IPC (UNIX domain socket) from another application (file client/server) on the same host.

- The MIPTPD adds a header(important) to the data and sends the now MIPTPD package to the local MIP daemon which will forward the package to the destination.

- The MIPTPD can receive packages from the MIP daemon and forward the data to a listening (UNIX domain socket) application (file client/server). Packages are dropped if there are no listening applications.


**[IMPORTANT] port numbers separate different applications that connect to the same MIPTPD**
*For simplicity it is assumed the source and destination applications use the same port number*

