-- DATA_FRAMER.VHD
------------------------------------------------------------------------------
-- MODULE: Marmote Main Board
-- AUTHORS: Sandor Szilvasi
-- AUTHOR CONTACT INFO.: Sandor Szilvasi <sandor.szilvasi@vanderbilt.edu>
-- TOOL VERSIONS: Libero 10.1
-- TARGET DEVICE: A2F500M3G (256 FBGA)
--   
-- Copyright (c) 2006-2012, Vanderbilt University
-- All rights reserved.
--
-- Permission to use, copy, modify, and distribute this software and its
-- documentation for any purpose, without fee, and without written agreement is
-- hereby granted, provided that the above copyright notice, the following
-- two paragraphs and the author appear in all copies of this software.
--
-- IN NO EVENT SHALL THE VANDERBILT UNIVERSITY BE LIABLE TO ANY PARTY FOR
-- DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
-- OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE VANDERBILT
-- UNIVERSITY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--
-- THE VANDERBILT UNIVERSITY SPECIFICALLY DISCLAIMS ANY WARRANTIES,
-- INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
-- AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
-- ON AN "AS IS" BASIS, AND THE VANDERBILT UNIVERSITY HAS NO OBLIGATION TO
-- PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
-------------------------------------------------------------------------------
-- Revisions     :
-- Date            Version  Author			Description
-- 2012-09-18      1.0      Sandor Szilvasi	Transmits empty frames
------------------------------------------------------------------------------
--
-- Description: Interface module to serialize and frame the parallel data
--              streams to be transmitted through the FT232H USB (FTDI) chip.
--
--              The module receives 2x16-bit data streams and passes them
--              through synchronizer FIFOs along with sequence numbers.
--
-- TODO:        Make FIFO AEMPTY and AFULL parameters generic.
--              Precalculate checksum field for first 4 bytes (class, ID, len)
------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

library smartfusion;
use smartfusion.all;

entity DATA_FRAMER is
    port (
        -- System clock region
        CLK         : in  std_logic;
        RST         : in  std_logic;

        TX_I        : in  std_logic_vector(15 downto 0);
        TX_Q        : in  std_logic_vector(15 downto 0);
        TX_STROBE   : in  std_logic;

        -- USB clock region
        USB_CLK     : in  std_logic;
        
        TXD_REQ     : out std_logic;
        TXD_EN      : in  std_logic;
        TXD_RD      : in  std_logic;
        TXD         : out std_logic_vector(7 downto 0)
    );
end entity;


architecture Behavioral of DATA_FRAMER is

    -- Components


    -- Constants

    constant c_SYNC_CHAR_1 : std_logic_vector(7 downto 0) := x"B5";
    constant c_SYNC_CHAR_2 : std_logic_vector(7 downto 0) := x"63";

    constant c_MSG_CLASS   : std_logic_vector(7 downto 0) := x"0E"; -- FIXME
    constant c_MSG_ID      : std_logic_vector(7 downto 0) := x"0F"; -- FIXME

    constant c_MSG_LEN     : std_logic_vector(15 downto 0) := x"0000"; -- FIXME


    -- Signals

    alias usb_rst is rst;

    type framer_state_t is (
        st_IDLE,
        st_SYNC_1,
        st_SYNC_2,
        st_MSG_CLASS,
        st_MSG_ID,
        st_LEN_1,
        st_LEN_2,
        st_CHK_A,
        st_CHK_B
    );

    signal s_state      : framer_state_t := st_IDLE;
    signal s_state_next : framer_state_t;

    signal s_chk_a      : std_logic_vector(7 downto 0);
    signal s_chk_b      : std_logic_vector(7 downto 0);

    signal s_txd        : std_logic_vector(7 downto 0);
    signal s_txd_next   : std_logic_vector(7 downto 0);
    signal s_txd_req    : std_logic;
    signal s_txd_req_next : std_logic;

    signal s_seq_fifo_aempty : std_logic;


begin

    -- Port maps


    -- Processes

    p_framer_sync : process (usb_rst, usb_clk)
    begin
        if usb_rst = '1' then
            s_state <= st_IDLE;
            s_txd <= (others => '0');
            s_txd_req <= '0';
        elsif rising_edge(usb_clk) then
            if TXD_EN = '1' then
                s_state <= s_state_next;
            end if;
            s_txd <= s_txd_next;
            s_txd_req <= s_txd_req_next;
        end if;
    end process p_framer_sync;

    p_framer_comb : process (
        s_state,
        s_seq_fifo_aempty
    )
    begin
        -- Default assignments
        s_state_next <= s_state;
        s_txd_next <= s_txd;
        s_txd_req_next <= s_txd_req;

        -- Next state and output logic
        case s_state is

            when st_IDLE =>
                s_txd_next <= c_SYNC_CHAR_1;
                if s_seq_fifo_aempty = '0' then
                    s_txd_req_next <= '1';
                    s_state_next <= st_SYNC_1;
                end if;

            when st_SYNC_1 =>
                s_txd_next <= c_SYNC_CHAR_2;
                if TXD_RD = '1' then
                    s_state_next <= st_SYNC_2;
                end if;

            when st_SYNC_2 =>
                s_txd_next <= c_MSG_CLASS;
                if TXD_RD = '1' then
                    s_state_next <= st_MSG_CLASS;
                end if;

            when st_MSG_CLASS =>
                s_txd_next <= c_MSG_ID;
                if TXD_RD = '1' then
                    s_state_next <= st_MSG_ID;
                end if;

            when st_MSG_ID =>
                s_txd_next <= c_MSG_LEN(7 downto 0);
                if TXD_RD = '1' then
                    s_state_next <= st_LEN_1;
                end if;

            when st_LEN_1 =>
                s_txd_next <= c_MSG_LEN(15 downto 8);
                if TXD_RD = '1' then
                    s_state_next <= st_LEN_2;
                end if;

            when st_LEN_2 =>
                s_txd_next <= s_chk_a;
                if TXD_RD = '1' then
                    s_state_next <= st_CHK_A;
                end if;

            when st_CHK_A =>
                s_txd_next <= s_chk_b;
                if TXD_RD = '1' then
                    s_state_next <= st_CHK_B;
                end if;

            when st_CHK_B =>
                if TXD_RD = '1' then
                    s_txd_req_next <= '0';
                    s_state_next <= st_IDLE;
                end if;

            when others =>
                null;

        end case;

    end process p_framer_comb;


    s_seq_fifo_aempty <= '0'; -- FIXME

    -- Output assignments

    TXD_REQ <= s_txd_req;
    TXD <= s_txd;



end Behavioral;

