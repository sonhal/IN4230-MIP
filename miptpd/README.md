# MIPTPD

## Planning

#### MIPTPD - A transport daemon

Capabilities:
- Receives [data, portnumber, destination mip address] over IPC (UNIX domain socket) from another application (file client/server) on the same host.

- The MIPTPD adds a header(important) to the data and sends the now MIPTPD package to the local MIP daemon which will forward the package to the destination.

- The MIPTPD can receive packages from the MIP daemon and forward the data to a listening (UNIX domain socket) application (file client/server). Packages are dropped if there are no listening applications.


**[IMPORTANT] port numbers separate different applications that connect to the same MIPTPD**
*For simplicity it is assumed the source and destination applications use the same port number*

MIPTPD receives a timeout value as a CLI argument. If MIPTPD is ran in debug mode (also a CLI argument) every communication event is logged to the console with relevant metadata:
 - packages sent, received
 - window size
 - etc

### MIPTP Header
*The header is added to the data before it is sent over the network*

**MIPTP Header Contents**
*Big endian format*

| data | type |
| ---- | ---- |
| Padding Length bits | 2 bits |
| Port number | 14 bits |
| Packet Sequence number | 16 bits (uint_16) |

If a MIPTP-package does not contain any data, then it is a ACK package. A MIPTP ACK package confirms that all the packages up to the included sequence number. Payload length in a ACK package is set to 1.

- Max frame size of Ethernet is 1500
- Max MIPTP payload size is 1492 (1500 - MIP and MIPTP headers).

Length on MIPTP payload can be calculated with help form the **payload length** value in the MIP header.


*MIPTP payload = (MIP payload length - 1) * 4*




### Communication

##### Interface with tp-client

**Request from tp-client**
| data | type |
| ---- | ---- |
| size of file in bytes | uint_16 |
| filename | c string |
| destination mip address | uint_8 |
| port number | 14 bits |



##### Interface with tp-server

**Request from tp-server**
| data | type |
| ---- | ---- |


#### Notes

use getopt lib for CLI parsing