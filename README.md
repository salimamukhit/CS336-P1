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

We used libpcap library to capture RST packets. If you do not have it installed on your machine, in your Terminal run sudo apt install libpcap-dev

1) In your terminal you should open a soloclient-src folder. Run make command which will compile a standalone application. Then run the program with sudo ./standalone. 

2) The Part_Two.pcap includes the packets that were sent through this application. Apply a filter ip.addr == 107.180.95.33 in order to see only those packets that were sent by this app. 

-------------------- Additional Information --------------------

The shared-src folder contains files with helper methods and parser of a configuration file. The purpose of each program is described.
