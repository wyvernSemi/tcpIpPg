//=============================================================
//
// Copyright (c) 2021 Simon Southwell. All rights reserved.
//
// Date: 18th August 2021
//
// Class method definitions for TCP connection and termination
// driver
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

#include "tcpConnect.h"

// --------------------------------------------
// Method to initiate a TCP connection and
// follow protocol to connection establishment
// --------------------------------------------

tcpIpPg::rxInfo_t tcpConnect::initiateConnect(
    int                            node,
    tcpIpPg*                       &pTcp,
    std::vector<tcpIpPg::rxInfo_t> &rxQueue,
    uint32_t                       dst_port,
    uint32_t                       initial_winsize,
    uint32_t                       ip_dst_addr,
    uint64_t                       mac_dst_addr,
    uint32_t                       init_seq_num
)
{
    tcpIpPg::tcpConfig_t pktCfg;

    // Generate a packet to open a connection
    pktCfg.dst_port     = dst_port;
    pktCfg.seq_num      = init_seq_num;
    pktCfg.ack_num      = SERVER_TCP_INIT_SEQ; // Don't care, but set to make initial relative display correct
    pktCfg.ack          = false;
    pktCfg.rst_conn     = false;
    pktCfg.sync_seq     = true;
    pktCfg.finish       = false;
    pktCfg.win_size     = initial_winsize;
    pktCfg.ip_dst_addr  = ip_dst_addr;
    pktCfg.mac_dst_addr = mac_dst_addr;

    uint32_t len = pTcp->genTcpIpPkt (pktCfg, frmBuf, payload, payloadLen);

    pTcp->TcpVpSendRawEthFrame(frmBuf, len);
    
    // Increment sequence number for sending of SYN
    pktCfg.seq_num++;

    // state = SYN_SENT;

    pTcp->TcpVpSendIdle(20);

    // Wait for SYN-ACK
    while(rxQueue.empty())
    {
        pTcp->TcpVpSendIdle(20);
    }

    // Check Flags SYN ACK and payload 0
    tcpIpPg::rxInfo_t pkt = rxQueue.front();
    if (pkt.tcp_flags == (SYN | ACK) && pkt.rx_len == 0)
    {
        // state = ESTABLISHED;

        uint32_t last_dst_seq = pkt.tcp_seq_num;

        // Send ack
        pktCfg.sync_seq = false;
        pktCfg.ack      = true;
        pktCfg.ack_num  = last_dst_seq + 1;

        // Send message
        char sbuf[200];
        payloadLen = sprintf(sbuf, "*** Hello from node %d ***\n\n", node);
        for (int idx = 0; idx < payloadLen; idx++)
        {
            payload[idx] = sbuf[idx];
        }

        len = pTcp->genTcpIpPkt (pktCfg, frmBuf, payload, payloadLen);

        pTcp->TcpVpSendRawEthFrame(frmBuf, len);
        
        pktCfg.seq_num += payloadLen;
    }

    // Delete the packet
    rxQueue.erase(rxQueue.begin());
    
    pkt.tcp_ack_num += payloadLen;

    return pkt;

}


// --------------------------------------------
// Method to listen for a TCP connection SYN
// packet and follow connection protocol to
// connection establishment.
// --------------------------------------------

tcpIpPg::rxInfo_t tcpConnect::listenConnect(
    int                            node,
    tcpIpPg*                       &pTcp,
    std::vector<tcpIpPg::rxInfo_t> &rxQueue,
    uint32_t                       initial_winsize,
    uint32_t                       init_seq_num
)
{
    // state = LISTEN

    uint32_t openportnum;

    // Wait for wakeup
    while(rxQueue.empty())
    {
        pTcp->TcpVpSendIdle(20);
    }

    // Check Flags SYN and payload 0
    tcpIpPg::rxInfo_t pkt = rxQueue.front();
    if (pkt.tcp_flags == SYN && pkt.rx_len == 0)
    {
        // state = SYN_RECEIVED;

        last_dst_seq        = pkt.tcp_seq_num;

        // Reply
        pktCfg.dst_port     = pkt.tcp_src_port;
        pktCfg.seq_num      = init_seq_num;
        pktCfg.ack_num      = last_dst_seq + 1;
        pktCfg.ack          = true;
        pktCfg.rst_conn     = false;
        pktCfg.sync_seq     = true;
        pktCfg.finish       = false;
        pktCfg.win_size     = initial_winsize;
        pktCfg.ip_dst_addr  = pkt.ipv4_src_addr;
        pktCfg.mac_dst_addr = pkt.mac_src_addr;

        uint32_t len = pTcp->genTcpIpPkt (pktCfg, frmBuf, payload, payloadLen);

        pTcp->TcpVpSendRawEthFrame(frmBuf, len);

        // Delete the processed RX packet
        rxQueue.erase(rxQueue.begin());

        // Wait for ACK
        while(rxQueue.empty())
        {
            pTcp->TcpVpSendIdle(20);
        }

        pkt = rxQueue.front();

        // Check we got an ack
        if (pkt.tcp_flags == ACK)
        {
            // state = ESTABLISHED;
            last_dst_seq = pkt.tcp_seq_num;

            // Process any packet data
            if (pkt.rx_len)
            {
                char sbuf[200];
                for(int idx = 0; idx < pkt.rx_len; idx++)
                {
                    sbuf[idx] = pkt.rx_payload[idx];
                }
                sbuf[pkt.rx_len] = 0;
                VPrint("Node%d: %s", node, sbuf);
            }

            // Delete the processed RX packet
            rxQueue.erase(rxQueue.begin());
        }
        else
        {
            // Delete the unprocessed RX packet
            rxQueue.erase(rxQueue.begin());
        }
    }
    else
    {
        // Delete the unprocessed RX packet
        rxQueue.erase(rxQueue.begin());
    }

    return pkt;
}

// --------------------------------------------
// Method to initiate closing of a TCP link,
// following protocol until closure reached.
// --------------------------------------------

int tcpConnect::initiateTermination(
    int                            node,
    tcpIpPg*                       &pTcp,
    std::vector<tcpIpPg::rxInfo_t> &rxQueue,
    uint32_t                       dst_port,
    uint32_t                       winsize,
    uint32_t                       ip_dst_addr,
    uint64_t                       mac_dst_addr,
    uint32_t                       seq_num,
    uint32_t                       ack_num
)
{
    int error = 0;

    tcpIpPg::tcpConfig_t pktCfg;

    // Send FIN packet

    pktCfg.dst_port     = dst_port;
    pktCfg.seq_num      = seq_num;
    pktCfg.ack_num      = ack_num;
    pktCfg.ack          = false;
    pktCfg.rst_conn     = false;
    pktCfg.sync_seq     = false;
    pktCfg.finish       = true;
    pktCfg.win_size     = winsize;
    pktCfg.ip_dst_addr  = ip_dst_addr;
    pktCfg.mac_dst_addr = mac_dst_addr;

    uint32_t len = pTcp->genTcpIpPkt (pktCfg, frmBuf, NULL, 0);

    pTcp->TcpVpSendRawEthFrame(frmBuf, len);
    
    // FIN increments sequence number
    pktCfg.seq_num++;

    // Wait for ACK or ACK+FIN
    while(rxQueue.empty())
    {
        pTcp->TcpVpSendIdle(20);
    }

    tcpIpPg::rxInfo_t pkt = rxQueue.front();
    if ((pkt.tcp_flags & ACK) && pkt.rx_len == 0)
    {
        // if no FIN Wait for FIN
        if(!(pkt.tcp_flags & FIN))
        {
            // Delete ACK packet
            rxQueue.erase(rxQueue.begin());
            
            while(rxQueue.empty())
            {
                pTcp->TcpVpSendIdle(20);
            }
        }

        pkt = rxQueue.front();

        if ((pkt.tcp_flags & FIN) && pkt.rx_len == 0)
        {
            // Send ACK
            pktCfg.finish   = false;
            pktCfg.ack      = true;
            pktCfg.ack_num  = pkt.tcp_seq_num;

            len = pTcp->genTcpIpPkt (pktCfg, frmBuf, NULL, 0);
            pTcp->TcpVpSendRawEthFrame(frmBuf, len);

        }
        else
        {
            // Expected a FIN packet
            error = 1;
        }
        rxQueue.erase(rxQueue.begin());

    }
    else
    {
        // Expected an ACK (or ACK+FIN) packet
        rxQueue.erase(rxQueue.begin());
        error = 2;
    }

    return error;
}

// --------------------------------------------
// Method to wait for initiation of TCP link 
// closure with a FIN packet and follow protcol
// until link closed. Will process non-closure
// packets until FIN packet seen if processPkts
// is set to true
// --------------------------------------------

 int tcpConnect::waitForTermination(
    int                            node,
    tcpIpPg*                       &pTcp,
    std::vector<tcpIpPg::rxInfo_t> &rxQueue,
    uint32_t                       winsize,
    uint32_t                       seq_num,
    uint32_t                       openPort,
    bool                           finRxAlready,
    bool                           processPkts
 )
{
    tcpIpPg::rxInfo_t pkt;
    
    int  error  = 0;
    bool closed = false;
    
    pktCfg.seq_num  = seq_num;
    pktCfg.win_size = winsize;
    
    do {
        // Wait for a FIN packet only if one not seen already externally.
        if (!finRxAlready)
        {
            // Wait for Packet
            while(rxQueue.empty())
            {
                pTcp->TcpVpSendIdle(20);
            }
            
            pkt = rxQueue.front();
            rxQueue.erase(rxQueue.begin());
        }
        
        // Only process packets routed to the open port connections
        if (finRxAlready || pkt.tcp_src_port == openPort)
        {
        
            // If received a FIN packet, start closure procedure
            if (finRxAlready || ((pkt.tcp_flags & FIN) && pkt.rx_len == 0))
            {
                // Send ACK+FIN
                pktCfg.ack_num      = pkt.tcp_seq_num + 1;
                pktCfg.ack          = true;
                pktCfg.rst_conn     = false;
                pktCfg.sync_seq     = false;
                pktCfg.finish       = true;
                pktCfg.ip_dst_addr  = pkt.ipv4_src_addr;
                pktCfg.mac_dst_addr = pkt.mac_src_addr;
                pktCfg.dst_port     = pkt.tcp_src_port;
            
                uint32_t len = pTcp->genTcpIpPkt (pktCfg, frmBuf, NULL, 0);
            
                pTcp->TcpVpSendRawEthFrame(frmBuf, len);
                
                // FIN  increments sequence number
                pktCfg.seq_num++;
            
                // Wait for ACK
                while(rxQueue.empty())
                {
                    pTcp->TcpVpSendIdle(20);
                }
            
                tcpIpPg::rxInfo_t pkt = rxQueue.front();
                rxQueue.erase(rxQueue.begin());
            
                if (!((pkt.tcp_flags & ACK) && pkt.rx_len == 0))
                {
                    // Expected an ACK packet
                    error = 1;
                }
                else
                {
                    closed = true;
                }
            }
            // If a normal packet and processing packets enabled, process them here 
            else
            {
                if (!processPkts)
                {
                    // Expected a FIN packet
                    error = 2;
                }
                else
                {
                    // Process any packet data
                    if (pkt.rx_len)
                    {
                        char sbuf[200];
                        for(int idx = 0; idx < pkt.rx_len; idx++)
                        {
                            sbuf[idx] = pkt.rx_payload[idx];
                        }
                        sbuf[pkt.rx_len] = 0;
                        VPrint("Node%d: %s", node, sbuf);
                    }
                    
                    // Send ACK
                    pktCfg.ack_num      = pkt.tcp_seq_num + 1;
                    pktCfg.ack          = true;
                    pktCfg.rst_conn     = false;
                    pktCfg.sync_seq     = false;
                    pktCfg.finish       = false;
                    pktCfg.ip_dst_addr  = pkt.ipv4_src_addr;
                    pktCfg.mac_dst_addr = pkt.mac_src_addr;
                    pktCfg.dst_port     = pkt.tcp_src_port;
                    
                    int32_t len = pTcp->genTcpIpPkt (pktCfg, frmBuf, NULL, 0);
            
                    pTcp->TcpVpSendRawEthFrame(frmBuf, len);
                    
                }
            }
        }
    } while (processPkts && !error && !closed);

    return error;
}