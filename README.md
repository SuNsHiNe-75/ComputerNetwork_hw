#### Introduction
This is the project of the "Computer Network" course at NSYSU.  

It mainly covers the following tasks:
- Set the parameters including RTT (20 ms), MSS (1 Kbytes), threshold (64 Kbytes), and the receiver’s buffer size (512 Kbytes), etc.
- Transmit the video files and perform mathematical calculations including power and square root in this step. A client could request a single job or multiple jobs in one command. The server should send the video file, the result of mathematical equations to multiple clients at the same time.
- The mathematical calculations include add, subtract, multiply, divide, power and square root.
- Implement the data transmission (Need to ensure that the data are transmitted from the server to clients, and ACK packets are transmitted from clients to the server).
- Print out the status of the server and clients. For example, for the server, which clients the server is sending to and which files the server receives in this step.

#### Run
For the server: 
```shell
./server 10250
```

For the client: 
```shell
./client 127.0.0.1 10250 x.mp4
```

#### TODO
- In fact, this is an unfinished product, with some technologies such as congestion control or fast retransmit not yet implemented.
