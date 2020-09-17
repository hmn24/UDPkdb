# UDPkdb
UDP Protocol for kdb 

## Server Side:
```
q)t:`serverUDP 2: (`recvUDP;1)
q)t[3303i]
6i
q)a
'a
  [0]  a
       ^
q)a
2
q)b
4
```

## Client Side:
```
q)t:`clientUDP 2: (`sendUDP;3)
q)t["127.0.0.1";3303i;"a:1+1"]
q)t["127.0.0.1";3303i;"b:1+1"]
q)t["127.0.0.1";3303i;"b:1+3"]
q)t[`127.0.0.1;3303i;"b:1+3"]
```
