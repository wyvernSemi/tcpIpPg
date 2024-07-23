//=============================================================
// 
// Copyright (c) 2021 Simon Southwell. All rights reserved.
//
// Date: 16th August 2021
//
// Class header for TCP/IPv4 packet generation
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

#ifndef _TCP_IP_PG_H_
#define _TCP_IP_PG_H_

#include <stdio.h>
#include <stdint.h>

#include "tcpVProc.h"

class tcpIpPg  : public tcpVProc
{
public:

    // --------------------------------------------
    // Static constants
    // --------------------------------------------
    
    // Version
    static const uint32_t major_version        = 1;
    static const uint32_t minor_version        = 0;
    
    // CRC32 parameters
    static const uint32_t POLY                 = 0xEDB88320;  /* 0x04C11DB7 bit reversed */
    static const uint32_t INIT                 = 0xFFFFFFFF;

    // Nominal 10G clock frequency (Hz)
    static const uint32_t CLK10G_FREQ          = 156250000;

    // IPv4 parameters
    static const uint32_t IPV4_MULTICAST_ADDR  = 0x00000000;
    static const uint32_t IPV4_SUBNET_MASK     = 0xffffffff;
    static const uint32_t IPV4_MIN_HDR_LEN     = 5;  // DWORDS
    static const uint32_t IPV4_SRC_ADDR_OFFSET = 3;  // DWORDS
    
    // TCP parameters
    static const uint32_t IPV4_DST_ADDR_OFFSET = 4;  // DWORDS
    static const uint32_t TCP_MIN_HDR_LEN      = 5;  // DWORDS
    static const uint32_t TCP_CHKSUM_OFFSET    = 16; // BYTES
    static const uint32_t TCP_PROTOCOL_NUM     = 6;

    // TCP header flag masks
    static const uint32_t TCP_FLAG_NS          = 0x100;
    static const uint32_t TCP_FLAG_CWR         = 0x080;
    static const uint32_t TCP_FLAG_ECE         = 0x040;
    static const uint32_t TCP_FLAG_URG         = 0x020;
    static const uint32_t TCP_FLAG_ACK         = 0x010;
    static const uint32_t TCP_FLAG_PSH         = 0x008;
    static const uint32_t TCP_FLAG_RST         = 0x004;
    static const uint32_t TCP_FLAG_SYN         = 0x002;
    static const uint32_t TCP_FLAG_FIN         = 0x001;

    // Receiver error masks
    static const uint32_t RX_BAD_CRC           = 0x0001;
    static const uint32_t RX_WRONG_MAC_ADDR    = 0x0002;
    static const uint32_t RX_BAD_IPV4_CHECKSUM = 0x0004;
    static const uint32_t RX_WRONG_IPV4_ADDR   = 0x0008;
    static const uint32_t RX_BAD_TCP_CHECKSUM  = 0x0010;
    static const uint32_t RX_WRONG_TCP_PORT    = 0x0020;

    // --------------------------------------------
    // Type definitions
    // --------------------------------------------
    
    // Structure for received packet information
    typedef struct {
        uint64_t mac_src_addr;
        uint32_t ipv4_src_addr;
        uint32_t tcp_src_port;
        uint32_t tcp_seq_num;
        uint32_t tcp_ack_num;
        uint32_t tcp_flags;
        uint32_t tcp_win_size;
        uint8_t  rx_payload[ETH_MTU];
        uint32_t rx_len;
    } rxInfo_t;

    // Structure definition for transmit parameters
    typedef class {
    public:
        // TCP controls
        uint32_t dst_port;
        uint32_t seq_num;
        uint32_t ack_num;
        bool     ack;
        bool     rst_conn;
        bool     sync_seq;
        bool     finish;
        uint32_t win_size;

        // IPV4 parameters
        uint32_t ip_dst_addr ;

        // MAC parameters
        uint64_t mac_dst_addr;
    } tcpConfig_t;

    // Type definition for user callback function to receive packets
    typedef void (*pUsrRxCbFunc_t) (rxInfo_t rx_info, void* hdl);

    // --------------------------------------------
    // Constructor
    // --------------------------------------------
    
    tcpIpPg  (uint32_t nodeIn, uint32_t ipv4AddrIn, uint64_t macAddrIn, uint32_t tcpPortIn) :
                                        tcpVProc(nodeIn),
                                        ipv4_addr(ipv4AddrIn),
                                        mac_addr(macAddrIn),
                                        tcp_port(tcpPortIn)
    {
        usrRxCbFunc                    = NULL;
    };

    // --------------------------------------------
    // Public methods
    // --------------------------------------------
    
    // Function to register user callback function to receive packets
    void           registerUsrRxCbFunc (pUsrRxCbFunc_t pFunc, void* hdlIn) { usrRxCbFunc = pFunc; hdl = hdlIn;};

    // Method to generate a TCP/IPv4 packet
    uint32_t       genTcpIpPkt         (tcpConfig_t &cfg, uint32_t* frm_buf, uint32_t* payload, uint32_t payload_len);

private:

    // --------------------------------------------
    // Private methods
    // --------------------------------------------
    
    // Method to construct an ethernet frame with (optional) payload
    uint32_t       ethFrame            (uint32_t* eth_frame,  uint32_t* payload, uint32_t payload_len, uint64_t dst_addr);
    
    
    // Method to construct an IPV4 frame with (optional) payload
    uint32_t       ipv4Frame           (uint32_t* ipv4_frame,
                                        uint32_t* payload,
                                        uint32_t  payload_len,
                                        uint32_t  ipv4_dst_addr,
                                        bool      add_tcp_chksum = true);

    // Method to construct a TCP segment with (optional) payload
    uint32_t       tcpSegment          (uint32_t* tcp_seg,
                                        uint32_t* payload,
                                        uint32_t  payload_len,
                                        uint32_t  dst_port,
                                        uint32_t  seq_num,
                                        uint32_t  ack_num,
                                        bool      ack,
                                        bool      reset_connection,
                                        bool      sync_seq,
                                        bool      last_pkt,
                                        uint32_t  window_size = 32768);

    // Method for processing raw receive data
    uint32_t       processFrame        (uint32_t* rx_buff, uint32_t rx_len);

    // Ethernet CR32 calculation method
    uint32_t       crc32               (uint32_t* buf, uint32_t len, uint32_t poly = POLY, uint32_t init = INIT, bool debug = false);
    
    // Method to calculate IP4v checksum. Also used (in ipv4frame) to calculate TCP checksum
    uint32_t       ipv4_chksum         (uint32_t* buf, uint32_t len, bool debug = false);
    
    // Method to extract receive data
    void           extractRx           (void);

    // --------------------------------------------
    // Private member variables
    // --------------------------------------------
    
    // This node's TCP port number
    uint32_t       tcp_port;
    
    // This node's IPV4 address
    uint32_t       ipv4_addr;
    
    // This node's MAC address
    uint64_t       mac_addr;

    // Pointer to the user's receive callback function
    pUsrRxCbFunc_t usrRxCbFunc;
    
    // Handle passed in with callback registration as pointer to calling class instance ('this' pointer).
    // Used to reference specific instances' methods and member variables.
    void*          hdl;
};

#endif