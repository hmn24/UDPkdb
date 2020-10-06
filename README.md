# UDPkdb
UDP Protocol for kdb

## Server Side:
```
q)t:`udpQ 2: (`recvUDP;1)
q)t[3303i]
6i
q)t[3304i]
7i
q)hclose 7
q)a
2
q)b
2
q)c
4
```

## Client Side (kdb+ one shot IPC Format):
```
q)t:`udpQ 2: (`sendOneShotUDP;3)
q)t["127.0.0.1";3303i;"a:1+1"]
q)t["127.0.0.1";3303i;"b:1+1"]
q)t[`127.0.0.1;3303i;"c:1+3"]
```

## Client Side (vanilla kdb+ IPC Format):
```
q)t:`udpQ 2: (`connectUDP;2)
q)t[`127.0.0.1;3303i]
6i
q)hclose 6
q)t1:`udpQ 2: (`sendUDPMsg;2)
q)t1[6i;"a:1+1"]
'Failed to send UDP Msg
  [0]  t1[6i;"a:1+1"]
       ^
q)t[`127.0.0.1;3303i]
6i
q)t1[6i;"a:1+3"]
```

## Performance Comparisons:

Server side:
```
(base) hming@hming:~/UDPkdb$ q -p 5050
KDB+ 4.0 2020.05.04 Copyright (C) 1993-2020 Kx Systems

q)t:`udpQ 2: (`recvUDP;1)
q)t[3303i]
8i
```

Client side:
```
(base) hming@hming:~/UDPkdb$ q
KDB+ 4.0 2020.05.04 Copyright (C) 1993-2020 Kx Systems

q)t:`udpQ 2: (`connectUDP;2)
q)t[`127.0.0.1;3303i]
6i
q)t1:`udpQ 2: (`sendUDPMsg;2)
q)h:hopen `::5050
q)h
7i


Sync:
q)\ts:1000 t1[6i;"{a::1+1}[]"]
57 896
q)\ts:1000 h "{a::1+1}[]"
107 544
q)\ts:1000 t1[6i;"{a::1+1}[]"]
50 896
q)\ts:1000 h "{a::1+1}[]"
123 544
q)\ts:1000 t1[6i;"{a::1+1}[]"]
62 896
q)\ts:1000 h "{a::1+1}[]"
105 544

Async:
q)\ts:1000 t1[-6i;"{a::1+1}[]"]
13 896
q)\ts:1000 neg[h] "{a::1+1}[]"
1 94944
q)\ts:1000 t1[-6i;"{a::1+1}[]"]
12 896
q)\ts:1000 neg[h] "{a::1+1}[]"
1 96256
q)\ts:1000 t1[-6i;"{a::1+1}[]"]
14 896
q)\ts:1000 neg[h] "{a::1+1}[]"
1 96256
```
