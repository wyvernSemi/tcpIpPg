/*
 * Verilog side TCP/IPv4 packet generator, built around VProc
 *
 * Copyright (c) 2021 Simon Southwell.
 *
 * This file is part of tcp_ip_pg.
 *
 * This code is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this code. If not, see <http://www.gnu.org/licenses/>.
 *
 */

// --------------------------------------------
// Timescale
// --------------------------------------------

`timescale 1ps/1ps

// --------------------------------------------
// Definitions
// --------------------------------------------

`define TXD_LO_ADDR                   32'h0
`define TXD_HI_ADDR                   32'h1
`define TXC_ADDR                      32'h2
`define TICKS_ADDR                    32'h3
`define HLT_ADDR                      32'h4

// ============================================
//  MODULE
// ============================================

module tcp_ip_pg
#(parameter                            NODE    = 0)
(
  input                                clk,

  output reg [63:0]                    txd,
  output reg  [7:0]                    txc,

  input      [63:0]                    rxd,
  input       [7:0]                    rxc,

  output reg                           halt
);

// --------------------------------------------
// Signal definitions
// --------------------------------------------

integer     count;

wire [31:0] nodenum = NODE;
wire [63:0] rxd_int;
wire  [7:0] rxc_int;
wire [31:0] Addr;
wire        WE;
wire        RD;
wire [31:0] DataOut;
reg  [31:0] DataIn;
wire        Update;
reg         UpdateResponse;

// --------------------------------------------
// Continuous assignments
// --------------------------------------------

// Ensure there is no race on the update ordering on
// the rising edge of the clock between updating the
// inputs and the synchronous process below being called.
assign #1   rxd_int                    = rxd;
assign #1   rxc_int                    = rxc;

// --------------------------------------------
// Initialisation
// --------------------------------------------

initial
begin
  UpdateResponse                       = 1'b1;
  txd                                  = 64'h0707070707070707;
  txc                                  = 8'hff;

  count                                = 0;
  halt                                 = 1'b0;
end

// --------------------------------------------
// Process to generate a tick count
// --------------------------------------------

always @(posedge clk)
begin
  count                                <= count + 1;
end

// --------------------------------------------
// Asynchronous process to access the ports and
// internal state.
// --------------------------------------------

always @(Update)
begin

  DataIn           = 32'h0;

  if (WE == 1'b1 || RD == 1'b1)
  begin
    case (Addr)

    // Update the TXD low word, if a write, and read the low RXD inputs
    `TXD_LO_ADDR: begin
      DataIn                           = rxd_int[31:0];
      if (WE == 1'b1)
      begin
        txd[31:0]                      = DataOut;
      end
    end

    // Update the TXD high word, if a write, and read the high RXD inputs
    `TXD_HI_ADDR: begin
      DataIn                           = rxd_int[63:32];
      if (WE == 1'b1)
      begin
        txd[63:32]                     = DataOut;
      end
    end

    // Update the TXC bits, if a write, and read the RXC inputs
    `TXC_ADDR: begin
      DataIn                           = {24'h0, rxc_int};
      if (WE == 1'b1)
      begin
        txc                            = DataOut[7:0];
      end
    end

    // This address must be accessed as a delta update since
    // it does not read the RX inputs
    `TICKS_ADDR: begin
      DataIn                           = count;
    end

    // It is recommended that this address is accessed as a delta
    // update in case a packet is being received and the sim does
    // not halt immediately. Assume it is a 'request' to halt to the
    // external logic.
    `HLT_ADDR: begin
      if (WE == 1'b1)
      begin
        halt                           = DataOut[0];
      end
    end

    // Only the above addresses are valid.
    default: begin
       $display("***ERROR: tcp_ip_pg---access to invalid address from VProc");
       $finish;
    end
    endcase
  end

  // Acknowledge the access by inverting the response input to VProc
  UpdateResponse                       = ~UpdateResponse;
end

  // --------------------------------------------
  // Virtual Processor to run packet generation
  // software.
  // --------------------------------------------

  VProc vp (
   .Clk                                (clk),
   .Addr                               (Addr),
   .WE                                 (WE),
   .RD                                 (RD),
   .DataOut                            (DataOut),
   .DataIn                             (DataIn),
   .WRAck                              (WE),
   .RDAck                              (RD),
   .Interrupt                          (3'b000),
   .Update                             (Update),
   .UpdateResponse                     (UpdateResponse),
   .Node                               (nodenum[3:0])
  );

endmodule

