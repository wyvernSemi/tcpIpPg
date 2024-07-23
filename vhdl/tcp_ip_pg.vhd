-- =============================================================
--
--  Copyright (c) 2024 Simon Southwell. All rights reserved.
--
--  Date: 22nd Jul 2024
--
--  This file is part of the tcp_ip_pg package.
--
--  tcp_ip_pg is free software: you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation, either version 3 of the License, or
--  (at your option) any later version.
--
--  tcp_ip_pg is distributed in the hope that it will be useful,
--  but WITHOUT ANY WARRANTY; without even the implied warranty of
--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--  GNU General Public License for more details.
--
--  You should have received a copy of the GNU General Public License
--  along with tcp_ip_pg. If not, see <http://www.gnu.org/licenses/>.
--
-- =============================================================

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity tcp_ip_pg is
  generic (
    NODE_NUM                           : integer := 0
  );
port (

  -- Clock and reset

  clk                                  : in  std_logic;

  txd                                  : out std_logic_vector(63 downto 0) := 64x"0707070707070707";
  txc                                  : out std_logic_vector( 7 downto 0) :=  8x"FF";

  rxd                                  : in  std_logic_vector(63 downto 0) := 64x"0707070707070707";
  rxc                                  : in  std_logic_vector( 7 downto 0) :=  8x"FF";

  halt                                 : out std_logic := '0'
);
end entity;

architecture behavioural of tcp_ip_pg is

  constant TXD_LO_ADDR                 : std_logic_vector(31 downto 0) := 32x"0";
  constant TXD_HI_ADDR                 : std_logic_vector(31 downto 0) := 32x"1";
  constant TXC_ADDR                    : std_logic_vector(31 downto 0) := 32x"2";
  constant TICKS_ADDR                  : std_logic_vector(31 downto 0) := 32x"3";
  constant HLT_ADDR                    : std_logic_vector(31 downto 0) := 32x"4";

  -- Signals for VProc
  signal update                        : std_logic;
  signal updateResponse                : std_logic := '1';
  signal RD                            : std_logic;
  signal Addr                          : std_logic_vector(31 downto 0);
  signal WE                            : std_logic;
  signal DataOut                       : std_logic_vector(31 downto 0);
  signal DataIn                        : std_logic_vector(31 downto 0) := (others => '0');

  signal rxd_int                       : std_logic_vector(63 downto 0);
  signal rxc_int                       : std_logic_vector( 7 downto 0);

  signal ClkCount                      : integer := 0;

begin

  -----------------------------------------
  -- Combinatorial logic
  -----------------------------------------

  -- Ensure there is no race on the update ordering on the rising edge of the clock
  -- between updating the inputs and the synchronous process below being called.
  rxd_int                              <=  rxd after 1 ns;
  rxc_int                              <=  rxc after 1 ns;

  -----------------------------------------
  -- Synchronous process
  -----------------------------------------

  process(clk)
  begin
    if clk'event and clk = '1' then
      ClkCount                         <= ClkCount + 1;
    end if;
  end process;

  -----------------------------------------
  -- Memory map I/O to VProc address space
  -----------------------------------------

  process(update)
  begin

    if update'event then
      DataIn <= 32x"0";

      if WE ='1' or RD = '1' then

        case Addr is
        when TXD_LO_ADDR =>
          DataIn                       <= rxd_int(31 downto 0);
          if WE = '1' then
            txd(31 downto 0)           <= DataOut;
          end if;

        when TXD_HI_ADDR =>
          DataIn                       <= rxd_int(63 downto 32);
          if WE = '1' then
            txd(63 downto 32)          <= DataOut;
          end if;

        when TXC_ADDR =>
          DataIn                       <= 24x"0" & rxc_int;
          if WE = '1' then
            txc                        <= DataOut(7 downto 0);
          end if;

        when TICKS_ADDR =>
          DataIn                       <= std_logic_vector(to_unsigned(ClkCount, 32));

        when HLT_ADDR =>
          if WE = '1' then
            halt                       <= DataOut(0);
          end if;

        when others =>
            report "***Error. tcp_ip_pg---access to invalid address from VProc" severity error;

        end case;
      end if;

      -- Finished processing, so flag to VProc
      updateResponse                   <= not updateResponse;

    end if;
  end process;

  -----------------------------------------
  -- VProc instantiation
  -----------------------------------------

  vproc_inst : entity work.VProc
  port map (
    Clk                                => clk,
    Addr                               => Addr,
    WE                                 => WE,
    RD                                 => RD,
    DataOut                            => DataOut,
    DataIn                             => DataIn,
    WRAck                              => WE,
    RDAck                              => RD,
    Interrupt                          => 3x"000",
    Update                             => update,
    UpdateResponse                     => updateResponse,
    Node                               => std_logic_vector(to_unsigned(NODE_NUM, 4))
  );

end behavioural;