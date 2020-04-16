# CS336-P1

Group: Erik Parawell, Salima Mukhit

In this file there will be a description on how to execute Server-Client Application and Standalone Application, as well as a short guide on our implementation.

-------------------- Client-Server Application --------------------

Go to configuration file config.ini. It is located in client-src folder. If you open it, you can see all the configurations needed for this part of the project. Both client and server run on our machine, the IP address used to bind server is 127.0.0.1

1) In your terminal you should open a server-src folder. Then you run a make command in Terminal. This will compile a program. Then you have to run it with sudo ./server

2) In your other terminal window you should go to client-src folder. Then you run a make command in Terminal. This will compile a client program. Then you have to run it with sudo ./client

3) In server-src you can see that the file received-config.ini appeared. This is the result of client sending the configuration file to the server.

4) The Part_One.pcap includes the packets captured.

-------------------- Standalone Application --------------------

Go to configuration file solo-config.ini. It is located in soloclient-src. If you open it, you can see all the configurations needed for this part of the project. In a line which starts with ClientIP you will have to provide your IP address. To get it you can use a ip a command in Terminal. The Destination IP for the standalone is 107.180.95.33 which is our VMS. 

1) In your terminal you should open a soloclient-src folder. Run make command which will compile a standalone application. Then run the program with sudo ./standalone. 

2) The Part_Two.pcap includes the packets that were sent through this application. Apply a filter ip.addr == 107.180.95.33 in order to see only those packets that were sent by this app. 

-------------------- Additional Information --------------------

The shared-src folder contains files with helper methods and parser of a configuration file. The purpose of each program is described.

--------------------     INI Information    ---------------------
Notes before use:
1) Comments are not supported in this format.
2) There can be no whitespace before or after the equals sign.
3) There must be a value after the equals sign, it cannot be left blank.
If you want to leave a variable empty then delete the line, although this can only be done with parameters that have a default values.
4) There must be a newline character that terminates each line. Including the last line.

A full INI consists of these parameters:
```
ServerIP=107.180.95.33
SourceUDP=55001
DestinationUDP=55000
HeadDestinationTCP=56000
TailDestinationTCP=62000
PortNumberTCP=50000
PayloadSizeUDP=1000
InterMeasurementTime=5
NumberPackets=6000
TimeToLiveUDP=255
ClientIP=192.168.1.37
Interface=enp4s0
StandAloneDst=107.180.95.33
```
Commented Version ( DO NOT USE )
```
ServerIP= The Server IP used for the Client/Server application
SourceUDP= The UDP port for a client.
DestinationUDP= The UDP port for a server.
HeadDestinationTCP= The TCP port used for the Head SYN packet in the standalone application.
TailDestinationTCP= Same as above, but the tail.
PortNumberTCP= The port that the server is listening on.
PayloadSizeUDP= The size of a UDP payload in bytes.
InterMeasurementTime= The break between measurements of UDP high and low entropy trains.
NumberPackets= The Number of UDP packets to be sent.
TimeToLiveUDP= The time to live for UDP packets.
ClientIP= The IP of the client (Used for the standalone application).
Interface= The networking interface name of the client (Used for the standalone application).
StandAloneDst= The server IP used for the standalone application.
```
