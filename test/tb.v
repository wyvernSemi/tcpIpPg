// -----------------------------------------------------------------------------
//  Title      : Test bench for TCP/IPv4 packet generator
//  Project    : tcp_ip_pg
// -----------------------------------------------------------------------------
//  File       : tb.v
//  Author     : Simon Southwell
//  Created    : 2021-08-16
//  Standard   : Verilog 2001
// -----------------------------------------------------------------------------
//  Description:
//  This block defines the top level test bench for theTCP/IP packet generator.
//  Checkout repo to same folder as VProc (github.com/wyvernSemi/vproc)
// -----------------------------------------------------------------------------
//  Copyright (c) 2021 Simon Southwell
// -----------------------------------------------------------------------------
//
//  This is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation(), either version 3 of the License(), or
//  (at your option) any later version.
//
//  It is distributed in the hope that it will be useful(),
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this code. If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------------

`timescale 1ps/1ps

`define RESET_PERIOD    10
`define TIMEOUT_COUNT   400000

`define HALT_ADDR       32'hAFFFFFF0

module tb
#(parameter GUI_RUN          = 0,
  parameter CLK_FREQ_KHZ     = 156250,
  parameter VCD_DUMP         = 0,
  parameter DEBUG_STOP       = 0
  )
();


// Clock, reset and simulation control state
reg            clk;
wire           reset_n;
integer        count;

wire [63:0]    txd;
wire  [7:0]    txc;
wire [63:0]    rxd;
wire  [7:0]    rxc;

wire  [1:0]    halt;

// -----------------------------------------------
// Initialisation, clock and reset
// -----------------------------------------------

initial
begin
   if (VCD_DUMP != 0)
   begin
     $dumpfile("waves.vcd");
     $dumpvars(0, tb);
   end
   
   clk                                 = 1'b1;
   count                               = -1;
   
   #0 // Ensure first x->1 clock edge is complete before initialisation
   
   if (DEBUG_STOP != 0)
   begin
     $display("\n***********************************************");
     $display("* Stopping simulation for debugger attachment *");
     $display("***********************************************\n");
     $stop;
   end
   
   // Generate a clock
   forever #(500000000/CLK_FREQ_KHZ) clk = ~clk;
end

// Generate a reset signal using count
assign reset_n                         = (count >= `RESET_PERIOD) ? 1'b1 : 1'b0;

// -----------------------------------------------
// Simulation control process
// -----------------------------------------------
always @(posedge clk)
begin
  count                                <= count + 1;

  // Stop/finish the simulations of timeout or a halt signal
  if (count == `TIMEOUT_COUNT || |halt == 1'b1)
  begin
    if (GUI_RUN == 0)
    begin
      $finish;
    end
    else
    begin
      $stop;
    end
  end
end

// -----------------------------------------------
// TCP/IPv4 node 0
// -----------------------------------------------

  tcp_ip_pg #(.NODE(0)) node0
  (
    .clk                     (clk),

    .txd                     (txd),
    .txc                     (txc),

    .rxd                     (rxd),
    .rxc                     (rxc),

    .halt                    (halt[0])
  );

// -----------------------------------------------
// TCP/IPv4 node 1
// -----------------------------------------------

  tcp_ip_pg #(.NODE(1)) node1
  (
    .clk                     (clk),
    .txd                     (rxd),
    .txc                     (rxc),

    .rxd                     (txd),
    .rxc                     (txc),

    .halt                    (halt[1])
  );

endmodule