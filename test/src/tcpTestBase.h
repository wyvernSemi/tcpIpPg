//=============================================================
//
// Copyright (c) 2021 Simon Southwell. All rights reserved.
//
// Date: 18th August 2021
//
// Class definition of TCP server example test program
//
// This code is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This code is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this code. If not, see <http://www.gnu.org/licenses/>.
//
//=============================================================

#ifndef _TCP_TEST_BASE_H_
#define _TCP_TEST_BASE_H_

#include "tcpPrintPkt.h"
#include "tcpConnect.h"

class tcpTestBase : public tcpPrintPkt
{
public:

    static const uint32_t ACK = 0x10;
    static const uint32_t RST = 0x04;
    static const uint32_t SYN = 0x02;
    static const uint32_t FIN = 0x01;

                     tcpTestBase(int nodeIn) : node(nodeIn), pTcp(NULL)
                     {
                         init_seq = 0;
                         init_ack = 0;
                     };
    
    // Virtual function to be provided by the derived test class
    virtual uint32_t runTest     () = 0;
    
    // Simulation control methods
    void            sleepForever() {if (pTcp != NULL) while(true) pTcp->TcpVpSendIdle(20000000);};
    void            haltSim     () {if (pTcp != NULL) pTcp->TcpVpSetHalt(1);};

    // Callback function needs to be static to allow it to be used as an
    // argument in the callback registration function. It will be passed
    // the 'this' pointer of its class object in hdl, so can access methods
    // via this pointer.
    static void     rxCallback (tcpIpPg::rxInfo_t rx_info, void* hdl)
    {

        // Display the received packet
        ((tcpTestBase*)hdl)->printRxPkt(rx_info, 
                                        ((tcpTestBase*)hdl)->node,
                                        true,
                                        ((tcpTestBase*)hdl)->init_seq,
                                        ((tcpTestBase*)hdl)->init_ack
                                        );
        
        // Append packet to the receive queue
        ((tcpTestBase*)hdl)->rxQueue.push_back(rx_info);
    }

protected:
    // Node number of this object
    int                            node;
    
    // Pointer to the TCP/IP packet generator object
    tcpIpPg*                       pTcp;
    
    // Receiver queue
    std::vector<tcpIpPg::rxInfo_t> rxQueue;
    
    // TCP connection state object.
    tcpConnect                     conn;
    
    uint32_t                       init_seq;
    uint32_t                       init_ack;

};

#endif