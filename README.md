# IN4230 Mandatory Assignment 1
![Github Actions badge](https://github.com/sonhal/IN4230-M1/workflows/CI/badge.svg)

### Plan
create mipd - a MIP daemon for handling network requests

1. mipd starts up with a UNIX socket address clients can connect to - done
2. MIP-ARP done
3. Update ARP-cache regularly
4. Drop the packet when a packet arrives and no application is connected on domain socket
