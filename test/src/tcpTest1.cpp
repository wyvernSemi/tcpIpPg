//=============================================================
//
// Copyright (c) 2021 Simon Southwell. All rights reserved.
//
// Date: 18th August 2021
//
// Class method definitions TCP server example test program
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

#include <vector>

#include "tcpIpPg.h"
#include "tcpTest1.h"
#include "tcpCommon.h"

// --------------------------------------------
// --------------------------------------------

uint32_t tcpTest1::runTest()
{

    pTcp = new tcpIpPg(node, SERVER_IPV4_ADDR, SERVER_MAC_ADDR, TCP_PORT_NUM);

    // Register RX call back function
    pTcp->registerUsrRxCbFunc(rxCallback, (void*)this);
    
    init_seq = CLIENT_TCP_INIT_SEQ;
    init_ack = SERVER_TCP_INIT_SEQ;

    // Listen for a connection and go through establishment
    tcpIpPg::rxInfo_t connLastPkt = conn.listenConnect (
                                         node,
                                         pTcp,
                                         rxQueue,
                                         DEFAULTWINSIZE,
                                         SERVER_TCP_INIT_SEQ);


    // Wait for termination, processing normal packets until FIN seen
    int error = conn.waitForTermination (
                         node,
                         pTcp,
                         rxQueue,
                         DEFAULTWINSIZE,
                         connLastPkt.tcp_ack_num,
                         connLastPkt.tcp_src_port,
                         false,
                         true);

    return 0;
}