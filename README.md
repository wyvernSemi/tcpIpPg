# tcpIpPg
## 10GbE XGMII TCP/IPv4 packet generator for Verilog

The tcpIpPg project is a set of verification IP for generating and receiving 10GbE TCP/IPv4 Ethernet packets over an XGMII interface in a Verilog test environment. The generation environment is a set of C++ classes, to generate packets in to a buffer and then send that buffer over the Verilog XGMII interface. The connection between the Verilog and the C++ domain is done using the Virtual Processor, VProc (wyvernSemi/vproc)—a piece of VIP that allows C and C++ code, compiled for the local machine, to run and access the Verilog simulation environment, and VProc is freely available on github.

<p align="center">
<img src="https://github.com/wyvernSemi/tcpIpPg/assets/21970031/0fcfe84e-3a32-414e-bbf1-536266245f8d" width=600>
</p>

The intent for this packet generator is to allow ease of test vector generation when verifying 10G Ethernet logic IP, such as a MAC, and/or a server or client for TCP and IPv4 protocols. The bulk of the functionality is defined in the provided C++ classes, making it easily extensible to allow support for other protocols such as UDP and IPv6. It is also meant to allow exploration of how these protocols function, as an educational vehicle.

An example test environment is provided, for ModelSim, with two packet generators instantiated, connected to one another—one acting as a client and one acting as a server. Connection establishment and disconnection software is provided in the test code to illustrate how packet generation is done, and how to easily build up mode complex and useful patterns of packets. Formatted output of received packets can be displayed during the simulation.

## Features

The basic functionality provided is as listed below

* A Verilog module tcp_ip_pg
    *	Clock input, nominally 156.25MHz (10×109 ÷ 64)
    *	XGMII interface, with TX and RX data and control  ports
    *	A halt output for use in test bench control
*	A class to generate a TCP/IPv4 packet into a buffer
*	A class to send a generated packet over the XGMII interface
*	A means to receive TCP/IPv4 packets over the XGMII interface and buffer them
*	A means to display, in a formatted manner, received packets 
*	Connection state machine not part of the packet generation class, but examples provided as part of the test environment (not complete).
*	A means to request a halt of the simulation (when no more test data to send)
*	A means to read a clock tick counter from the software
