//=============================================================
// 
// Copyright (c) 2021 Simon Southwell. All rights reserved.
//
// Date: 16th August 2021
//
// Class for TCP/IPv4 packet generation access to tcp_ip_pg.v
// VProc based Verilog component.
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

#ifndef _TCP_VPROC_H_
#define _TCP_VPROC_H_

#include <stdio.h>
#include <stdint.h>

extern "C" {
#include "VUser.h"
}

class tcpVProc
{

protected :

    // Virtual method, provided by derived class, where received data is sent
    virtual uint32_t processFrame (uint32_t* rx_buf, uint32_t rx_len) = 0;

    // The VProc node for the tcpClient HDL model
    int              node;

public:

    // --------------------------------------------
    // Static constants
    // --------------------------------------------

    // tcpClient VProc address offsets
    static const uint32_t TXD_LO_ADDR          = 0;
    static const uint32_t TXD_HI_ADDR          = 1;
    static const uint32_t TXC_ADDR             = 2;
    static const uint32_t TICKS_ADDR           = 3;
    static const uint32_t HALT_ADDR            = 4;

    // Ethernet tags and frame delimeters
    static const uint32_t IDLE                 = 0x107;
    static const uint32_t SOF                  = 0x1fb;
    static const uint32_t EoF                  = 0x1fd;
    static const uint32_t PREAMBLE             = 0x055;
    static const uint32_t SFD                  = 0x0d5;

    // Ethernet parameters and header dimensions
    static const uint32_t ETH_MTU              = 1500;
    static const uint32_t ETH_PREAMBLE         = 9;  // BYTES
    static const uint32_t ETH_802_1Q_LEN       = 4;  // BYTES
    static const uint32_t ETH_CRC_LEN          = 4;  // BYTES
    static const uint32_t ETH_HDR_LEN          = 14; // BYTES

    // --------------------------------------------
    // Constructor
    // --------------------------------------------

    tcpVProc(int nodeIn) : node(nodeIn)
    {
        currTickCount                  = 0xffffffff;
        receiving_frame                = false;
        rx_idx                         = 0;
    };


    // --------------------------------------------------
    // Method to idle for specified number of cycles
    // --------------------------------------------------

    uint32_t TcpVpSendIdle(uint32_t ticks)
    {
        uint32_t error = 0;
        uint32_t currTicks;

        VWrite(TXD_LO_ADDR, 0x07070707, true, node);
        VWrite(TXD_HI_ADDR, 0x07070707, true, node);
        VWrite(TXC_ADDR,          0xff, true, node);

        // Get start time
        for (int idx = 0; idx < ticks; idx++)
        {
            VRead(TICKS_ADDR, &currTicks, true, node);

            TcpVpExtractRx();
        }

        return error;
    }

    // --------------------------------------------------
    // Method to send a pre-prepared (raw) ethernet frame
    // --------------------------------------------------
    uint32_t TcpVpSendRawEthFrame(uint32_t* frame, uint32_t len)
    {
        uint32_t error = 0;
        uint32_t fidx  = 0;

        uint32_t buf[3];

        // Construct 64 bit TXD words and associated TXC byte from frame data,
        // flushed to 64 bit boundary
        for (int widx = 0; widx < (len+7)/8; widx++)
        {
            buf[0] = buf[1] = buf[2] = 0;

            // Take 8 TXD bytes and TXC bits and construct the word
            for (int idx = 0; idx < 8; idx++)
            {
                // If output index is less than frame length, construct using the frame data,
                // else pad with idle.
                if (fidx < len)
                {
                    buf[idx/4] |= (frame[fidx] & 0xff) << (8*(idx%4));
                    buf[2]     |= frame[fidx++] & 0x100 ? (1 << idx): 0;
                }
                else
                {
                    buf[idx/4] |= 0x7 << (8*(idx%4));
                    buf[2]     |= 1 << idx;
                }
            }

            // After each TXD/TXC word is constructed, send it out
            VWrite(TXD_LO_ADDR, buf[0], true, node);
            VWrite(TXD_HI_ADDR, buf[1], true, node);
            VWrite(TXC_ADDR,    buf[2], true, node);

            // Extract RX data and advance tick
            TcpVpExtractRx();
        }

        TcpVpSendIdle(1);

        return error;
    }
    
    // --------------------------------------------------
    // Method to set the halt output signal
    // --------------------------------------------------
    void TcpVpSetHalt(uint32_t val) {VWrite(HALT_ADDR, val & 0x1, false, node);}
    
private:

    // --------------------------------------------------
    // Method to extract received data from VProc input
    // interface.
    // --------------------------------------------------
    void TcpVpExtractRx ()
    {
        uint32_t rx[3];
        uint32_t clk_count;

        // If the current tick count is uninitialised, fetch clock tick count from the HDL,
        // else increment for each read cycle.
        if (currTickCount == 0xffffffff)
        {
            VRead(TICKS_ADDR, &currTickCount, true, node);
        }
        else
        {
            currTickCount++;
        }

        // Read the input pins: the 64 bits of data and 8 of control
        VRead(TXD_LO_ADDR, &rx[0],     true, node);
        VRead(TXD_HI_ADDR, &rx[1],     true, node);
        VRead(TXC_ADDR   , &rx[2],     false, node);

        // Amalgamate inputs into single words
        uint64_t rxd = (uint64_t)rx[0] | ((uint64_t)rx[1] << 32);
        uint64_t rxc =  rx[2];

        // Process the input unless completely idle
        if (!(rxd == 0x0707070707070707 && rxc == 0xff))
        {
            // Scan through the input byte at a time
            for (int idx = 0; idx < 8; idx++)
            {
                // Extract the data byte, with bit 8 as the control bit
                uint32_t rxbyte = ((rxd >> (8 * idx)) & 0xff) | ((rxc & (1 << idx)) ? 0x100 : 0);

                // If not receiving a frame already, and a start-of-frame detected,
                // flag receiving and reset the RX buffer index
                if (!receiving_frame && rxbyte == SOF)
                {
                    receiving_frame = true;
                    rx_idx          = 0;
                }

                // If receving a frame...
                if (receiving_frame)
                {
                    // If an end-of-frame detected, clear the receiving frame state, and call the
                    // method to process the data,
                    if (rxbyte == EoF)
                    {
                        receiving_frame = false;

                        // Process input, subtracting the preamble
                        processFrame(&rx_buf[ETH_PREAMBLE], rx_idx-ETH_PREAMBLE);
                    }
                    // Whilst receiving a frame, place it in the receive buffer
                    else
                    {
                        if (rx_idx == (ETH_MTU + ETH_HDR_LEN + ETH_PREAMBLE + ETH_CRC_LEN + ETH_802_1Q_LEN))
                        {
                            printf("WARNING: received packet of maximum size without an end-of-frame delimiter. Terminating packet\n");
                            receiving_frame = false;
                        }
                        else
                        {
                            rx_buf[rx_idx++] = rxbyte;
                        }
                    }
                }
            }
        }
        else
        {
            if (receiving_frame == true)
            {
                printf("WARNING: idle state reached in active packet without end-of-frame delimiter. Terminating packet\n");
                receiving_frame = false;
            }
        }

    }

    // -------------------------------------------------
    // Private member variables
    // -------------------------------------------------

    // Clock tick count (nominally at 6.4ns) to give timing
    uint32_t       currTickCount;

    // State flag to indicate actively receiving data
    bool           receiving_frame;

    // Receive buffer and index. Buffer size is the maximum for largest payload, plus headers
    uint32_t       rx_buf[ETH_MTU + ETH_HDR_LEN + ETH_PREAMBLE + ETH_CRC_LEN + ETH_802_1Q_LEN];
    uint32_t       rx_idx;

};

#endif