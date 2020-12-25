// Sample Client Codes
hopenUDP:`udpQ 2: (`connectUDP;2);
h:hopenUDP[`127.0.0.1;3303i];   // Test symbol
h1:hopenUDP["127.0.0.1";3303i]; // Test string 
hclose h1;

// Results of UDP IPC Calls stored in r
sendUDP:`udpQ 2: (`sendUDP;2);
r:sendUDP[h;"b:1+1;2"];

// Async UDP Calls Not Allowed For Now
// sendUDP[neg h;"b:1+1;2"];
// sendUDP[neg h;"b:1+1;2"];

// Test sendOneShotUDP codes
sendOneShotUDP:`udpQ 2: (`sendOneShotUDP;4);
r2:sendOneShotUDP[`127.0.0.1;3303i;"b:1+1;2";0N];

// Test setSocketTimeout codes
setSocketTimeout:`udpQ 2: (`setSocketTimeout;2);

// Simple check for memory leaks
checkMemLeak:{[fn;args]
  -1 "------------------------------------------------------------------------";
  -1 "Memory stats BEFORE running ", .Q.s1[fn];
  -1 .Q.s[.Q.w[]];
  fn . args;
  -1 "Memory stats AFTER running ", .Q.s1[fn];
  -1 .Q.s[.Q.w[]];
  -1 "------------------------------------------------------------------------";
 };

checkMemLeak[`sendUDP;(h;"b:1+1;2")];
checkMemLeak[`sendOneShotUDP;(`127.0.0.1;3303i;"b:1+1;2";0N)];

// Set Socket Timeout to 2s for both recv/send
checkMemLeak[`setSocketTimeout;(h;30)];

// One can run hclose against hopenUDP sockets like normal hopen TCP sockets in kdb
checkMemLeak[hclose `hopenUDP ::;(`127.0.0.1;3303i)];
