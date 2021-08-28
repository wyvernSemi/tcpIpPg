//=============================================================
//
// Copyright (c) 2021 Simon Southwell. All rights reserved.
//
// Date: 18th August 2021
//
// Class method definitions TCP client example test program
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
#include "tcpTest0.h"
#include "tcpCommon.h"

// --------------------------------------------
// --------------------------------------------

uint32_t tcpTest0::runTest()
{
    uint32_t payloadLen;
    uint32_t payload [PKTBUFSIZE];
    uint32_t frmBuf  [PKTBUFSIZE];
    tcpIpPg::tcpConfig_t pktCfg;

    pTcp = new tcpIpPg(node, CLIENT_IPV4_ADDR, CLIENT_MAC_ADDR, TCP_PORT_NUM);

    pTcp->TcpVpSendIdle(SMALL_PAUSE);

    // Register RX call back function
    pTcp->registerUsrRxCbFunc(rxCallback, (void*)this);
    
    init_seq = SERVER_TCP_INIT_SEQ;
    init_ack = CLIENT_TCP_INIT_SEQ;

    tcpIpPg::rxInfo_t connLastPkt = conn.initiateConnect(
                                            node,
                                            pTcp,
                                            rxQueue,
                                            TCP_PORT_NUM,
                                            DEFAULTWINSIZE,
                                            SERVER_IPV4_ADDR,
                                            SERVER_MAC_ADDR,
                                            CLIENT_TCP_INIT_SEQ);

    pTcp->TcpVpSendIdle(SMALL_PAUSE);

    // Send another packet with data...
    
    // Generate a payload as a zero termentated string message.
    char sbuf[STRBUFSIZE];
    payloadLen = sprintf(sbuf, "*** Data Packet from node %d ***\n\n", node);
    
    // Copy string bytes to payload buffer (not bytes)
    for (int idx = 0; idx < payloadLen; idx++)
    {
        payload[idx]    = sbuf[idx];
    }

    // Configure a transmission
    pktCfg.dst_port     = TCP_PORT_NUM;
    pktCfg.seq_num      = connLastPkt.tcp_ack_num;
    pktCfg.ack_num      = connLastPkt.tcp_seq_num +1;
    pktCfg.ack          = true;
    pktCfg.rst_conn     = false;
    pktCfg.sync_seq     = false;
    pktCfg.finish       = false;
    pktCfg.win_size     = DEFAULTWINSIZE;
    pktCfg.ip_dst_addr  = SERVER_IPV4_ADDR;
    pktCfg.mac_dst_addr = SERVER_MAC_ADDR;

    // Generate a frame of data using configuration and payload
    uint32_t len = pTcp->genTcpIpPkt (pktCfg, frmBuf, payload, payloadLen);

    // Transmit packet over node's bus
    pTcp->TcpVpSendRawEthFrame (frmBuf, len);
    
    // Increment the sequence number
    pktCfg.seq_num += payloadLen;

    // Wait for an ACK
    while(rxQueue.empty())
    {
        pTcp->TcpVpSendIdle(SMALL_PAUSE);
    }

    // Copy the received packet, and delete from the queue
    connLastPkt = rxQueue.front();
    rxQueue.erase(rxQueue.begin());
    
    // Check that an ACK received, and all packets acknowledged, then initiate termination
    // of connection.
    if ((connLastPkt.tcp_flags & ACK) && (connLastPkt.tcp_ack_num == pktCfg.seq_num))
    {

        pTcp->TcpVpSendIdle(SMALL_PAUSE);

        int error = conn.initiateTermination(
                               node, pTcp,
                               rxQueue,
                               TCP_PORT_NUM,
                               DEFAULTWINSIZE,
                               SERVER_IPV4_ADDR,
                               SERVER_MAC_ADDR,
                               pktCfg.seq_num,
                               connLastPkt.tcp_seq_num);
                              
        
    }
    
    pTcp->TcpVpSendIdle(END_PAUSE);
    

    return 0;
}