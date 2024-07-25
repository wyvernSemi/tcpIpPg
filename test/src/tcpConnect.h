//=============================================================
//
// Copyright (c) 2021 Simon Southwell. All rights reserved.
//
// Date: 18th August 2021
//
// Class definition of TCP connection and termination drivers
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
#include <cstdint>

#include "tcpCommon.h"
#include "tcpIpPg.h"

class tcpConnect
{
public:

    // Bit masks of TCP flags field
    static const uint32_t ACK = 0x10;
    static const uint32_t RST = 0x04;
    static const uint32_t SYN = 0x02;
    static const uint32_t FIN = 0x01;

    // Constructor
    tcpConnect()
    {
    };

    // Method for initiating connection with a server
    tcpIpPg::rxInfo_t initiateConnect(int                            node,
                                      tcpIpPg*                       &pTcp,
                                      std::vector<tcpIpPg::rxInfo_t> &rxQueue,
                                      uint32_t                       dst_port,
                                      uint32_t                       initial_winsize,
                                      uint32_t                       ip_dst_addr,
                                      uint64_t                       mac_dst_addr,
                                      uint32_t                       init_seq_num = 0);

    // Method for a server to listen for connection request and process conection protocol
    tcpIpPg::rxInfo_t listenConnect  (int                            node,
                                      tcpIpPg*                       &pTcp,
                                      std::vector<tcpIpPg::rxInfo_t> &rxQueue,
                                      uint32_t                       initial_winsize,
                                      uint32_t                       init_seq_num = 0);


    // Method to initiate termination of a connection and follow closure protocol
    int  initiateTermination         (int                            node,
                                      tcpIpPg*                       &pTcp,
                                      std::vector<tcpIpPg::rxInfo_t> &rxQueue,
                                      uint32_t                       dst_port,
                                      uint32_t                       winsize,
                                      uint32_t                       ip_dst_addr,
                                      uint64_t                       mac_dst_addr,
                                      uint32_t                       seq_num,
                                      uint32_t                       ack_num);

    // Method to wait for a termination request and follow closure protocol.
    // Can process packets until termination initiated.
    int  waitForTermination          (int                            node,
                                      tcpIpPg*                       &pTcp,
                                      std::vector<tcpIpPg::rxInfo_t> &rxQueue,
                                      uint32_t                       winsize,
                                      uint32_t                       seq_num,
                                      uint32_t                       openPort,
                                      bool                           finRxAlready = false,
                                      bool                           processPkts  = false);

private:

    // Packet/data buffers
    uint32_t frmBuf  [PKTBUFSIZE];
    uint32_t payload [PKTBUFSIZE];

    // Packet configuration structure, for use with tcpIpPg class methods
    tcpIpPg::tcpConfig_t pktCfg;
};
