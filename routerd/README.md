# Routing Daemon

Offers routing services to the network layer for end to end transmission

**The core task in this assignment is the implementation of DVR with split horizon. You must implement a routing protocol at the network layer which builds and maintains routing tables as defined in the DVR scheme. Assume that all directly connected neighbors have distance 1.**

```
 Dx(y) = min { C(x,v) + Dv(y)} for each node y ∈ N

```

## Distance Vector Routing

- Least cost between two nodes
- Bellman ford algorithm
  Defines distance at each node

```
x (Source)
y (Destination)
v (intermediate node)
Dv(y) (Cost of intermediate node to y)

Dx(y) = cost of least costpath from x to y

```
Update distances based on neighbors:

```
 Dx(y) = min { C(x,v) + Dv(y)} for each node y ∈ N

```

## Implementation details

## Routerd

- The router will use distance Vector Routing (DVR) with Split Horizon routing mechanism.
- Every host runs such a routing daemon, which offers routing services by communicating with the MIP daemon on the same host.
- The routing daemon must answer routing requests made by the MIP daemon to enable the latter to perform packet forwarding. 
- The routing daemon receives a MIP destination address from the MIP daemon and returns the MIP address of the corresponding neighbor node which is to serve as the next hop.
- The routing daemon must communicate with other routing daemons running on other hosts through the intermediary of the MIP daemon. 
- To differentiate routing requests (between the local MIP daemon and routing daemon) and route information exchange (between the local routing daemon and remote routing daemons), you are to use two separate UNIX domain sockets in the interface between the MIP daemon and the routing daemon.
- The routing daemon must regularly (periodically) update its routing table.
- The routing process begins as soon as both the MIP and routing daemons are started on a host.

## Mipd



- The interface between the MIP daemon and the application must not be changed from how it worked in the mandatory assignment.
- The MIP daemon must differentiate between packets exchanged between routing daemons and regular application traffic.
- The MIP daemon must be able to request the MIP address of the correct next hop neighbour from the routing daemon, enabling communication between hosts which are not directly connected at the link layer.
- The MIP daemon will discriminate routing packets (used as part of the routing protocol) from MIP-ARP and application payloads by setting the TRA-bits in the MIP header to the 010 bit combination.
- The MIP daemon must forward transport packets (TRA combination 100) in accordance with the current routing table state, or pass it up to the application if the destination address is local to the host.
- When the MIP daemon forwards a packet in its router capacity, it must decrement the Time to Live (TTL) field of the MIP header by one.
- When the TTL reaches a value of -1, the MIP daemon must drop the packet rather than forwarding it further. 
- Neither MIP-ARP lookups nor routing lookups should block (delay) the forwarding of packets arriving after the initiation of such lookups. In other words, lookups should happen asynchronously.


Domain sockets for routerd

/run/mipd-routing-remote.sock <- socket for remote communication
/run/mipd-routing-local.sock <- socket for local communication