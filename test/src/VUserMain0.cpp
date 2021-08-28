//=============================================================
//
// Copyright (c) 2021 Simon Southwell. All rights reserved.
//
// Date: 17th August 2021
//
// VProc Node 0 test code for tcp_ip_pg
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
#include "tcptest0.h"

// I'm node 0
static int node = 0;

// ---------------------------------------------
// Main entry point for node 0 VProc
// ---------------------------------------------

extern "C" void VUserMain0()
{
    VPrint("\n*****************************\n");
    VPrint(  "*   Wyvern Semiconductors   *\n");
    VPrint(  "* Virtual Processor (VProc) *\n");
    VPrint(  "*    tcp_ip_pg (node %d)     *\n", node);
    VPrint(  "*    Copyright (c) 2021     *\n");
    VPrint(  "*****************************\n\n");

    tcpTest0* pTest = new tcpTest0(0);

    pTest->runTest();

    pTest->haltSim();
    
    pTest->sleepForever();
}

