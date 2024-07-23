//=============================================================
//
// Copyright (c) 2021 Simon Southwell. All rights reserved.
//
// Date: 18th August 2021
//
// Class to format and print out a TCP packet from a received
// rxInfo_t packet. Meant to be inherited by the TCP test
// classes.
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

#ifndef _TCP_PRINT_PKT_
#define _TCP_PRINT_PKT_

#include "tcpIpPg.h"

class tcpPrintPkt
{
public:
    // Constructor
    tcpPrintPkt() {};

    // Method to print out formatted receive data
    static void printRxPkt(tcpIpPg::rxInfo_t &rx_info, int nodenum, bool relative = false, uint32_t init_seq=0, uint32_t init_ack=0)
    {
        const char* flags[9] = {" FIN", " SYN", " RST", " PSH", " ACK", "URG", " ECE", " CWR", " NS"};

        VPrint("Node%d: Source MAC Addr...........: ", nodenum);
        for (int idx = 0; idx < 6; idx++)
        {
            VPrint("%02lX", (unsigned long)((rx_info.mac_src_addr >> 8*(5-idx)) & 0xff));
            if (idx != 5)
                VPrint("-");
            else
                VPrint("\n");
        }

        VPrint("Node%d: Source IPv4 Addr..........: ", nodenum);
        for (int idx = 0; idx < 4; idx++)
        {
            VPrint("%02d", (rx_info.ipv4_src_addr >> 8*(3-idx)) & 0xff);
            if (idx != 3)
                VPrint(".");
            else
                VPrint("\n");
        }

        VPrint("Node%d: Source TCP port...........: 0x%04x\n", nodenum, rx_info.tcp_src_port);

        VPrint("Node%d: Source sequence#..........: 0x%08x", nodenum, rx_info.tcp_seq_num);
        if (relative)
        {
            VPrint(" (%4d relative)", rx_info.tcp_seq_num - init_seq);
        }
        VPrint("\n");

        VPrint("Node%d: Source ACK#...............: 0x%08x", nodenum, rx_info.tcp_ack_num);
        if (relative)
        {
            VPrint(" (%4d relative)", rx_info.tcp_ack_num - init_ack);
        }
        VPrint("\n");

        VPrint("Node%d: Source Window Size........: %d\n",     nodenum, rx_info.tcp_win_size);
        VPrint("Node%d: Payload Length............: %d\n",     nodenum, rx_info.rx_len);
        VPrint("Node%d: Flags.....................:", nodenum);

        for (int idx = 0; idx < 9; idx++)
        {
            if (rx_info.tcp_flags & (1 << idx))
            {
                VPrint("%s", flags[idx]);
            }
        }

        VPrint("\n\n");
    }
};

#endif