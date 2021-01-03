# UDPkdb
UDP Protocol for kdb

Only sync calls allowed (To find way to disable sendto from sd1 callback)

A more straightforward way is to impose async-only (which can be left to the reader as a simple activity to delete codes), which means the return call from server-side callback doesn't have to go through the pain of serialising the returning K object, and the client side to deserialise it
```
Note that UDP Protocol is not meant for large messages, hence the choice of string messages from client side. Advantages are:
1) Code density
2) Strings are natural bytes
3) Prevent use of .z.w callbacks not catered for here (and not intended for)
```

Compile with following commands:
```
gcc -shared -fPIC -o udpQ.so udpQ.c
```
## Server Side:
```
Refer to sample script in udpQ.server.q
```
## Client Side:
```
Refer to sample script in udpQ.client.q

As for reasons discussed above, client can only send UDP-IPC calls in string-format only
```
## Performance Comparisons:
```
UDP:
q)h:hopenUDP[`127.0.0.1;3303i]
q)\ts:10000 sendUDP[h;"b:1+1;2"]
599 896
q)\ts:10000 sendUDP[h;"b:1+1;2"]
587 896

Vanilla TCP/IP:
q)h1:hopen `::5050
q)\ts:10000 h1 "b:1+1;2"
790 544
q)\ts:10000 h1 "b:1+1;2"
776 544
```
