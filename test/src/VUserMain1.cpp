//=============================================================
//
// Copyright (c) 2021 Simon Southwell. All rights reserved.
//
// Date: 17th August 2021
//
// VProc Node 1 test code for tcp_ip_pg
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

#include <stdio.h>
#include <stdlib.h>

#include "VUserMain.h"
#include "tcpTest1.h"

// I'm node 1
static int node = 1;

// ---------------------------------------------
// Main entry point for node 1 VProc
// ---------------------------------------------

extern "C" void VUserMain1()
{
    VPrint("\n*****************************\n");
    VPrint(  "*   Wyvern Semiconductors   *\n");
    VPrint(  "* Virtual Processor (VProc) *\n");
    VPrint(  "*    tcp_ip_pg (node %d)     *\n", node);
    VPrint(  "*    Copyright (c) 2021     *\n");
    VPrint(  "*****************************\n\n");

    tcpTest1* pTest = new tcpTest1(node);
    
    pTest->runTest();
    
    pTest->sleepForever();

}

