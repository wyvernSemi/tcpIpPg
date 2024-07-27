###################################################################
# Makefile for TCP/IPv4 packet generator (tcp_ip_pg) test bench
# for ModelSim and Questa
#
# Copyright (c) 2021-2024 Simon Southwell.
#
# This file is part of tcpIpPg.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The file is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this file. If not, see <http://www.gnu.org/licenses/>.
#
###################################################################

#------------------------------------------------------
# User overridable definitions
#------------------------------------------------------

# Target HDL: VERILOG | VHDL (default VERILOG)
HDL                = VERILOG
ARCHFLAG           = -m64

#------------------------------------------------------
# Internal variables
#------------------------------------------------------

# User files to build, passed into vproc makefile build
USERCODE           = VUserMain0.cpp \
                     VUserMain1.cpp \
                     tcpTest0.cpp   \
                     tcpTest1.cpp   \
                     tcpConnect.cpp

TCPCODE            = tcpIpPg.cpp

# Set up Variables for tools
MAKE_EXE           = make
VSIMEXE            = $(MODEL_TECH)/vsim
VSIMARGS           =

# VProc location, relative to this directory
VPROC_TOP          = ../../vproc

# Define the github repository URL for the VProc virtual processor
VPROC_REPO         = https://github.com/wyvernSemi/vproc.git

USRCFLAGS          = "-I$(CURDIR)/../src"

# Get OS type
OSTYPE             = $(shell uname)

# If run from a place where MODEL_TECH is not defined, construct from path to PLI library
ifeq ("$(MODEL_TECH)", "")
  ifeq ($(OSTYPE), Linux)
    PLILIB         = libmtipli.so
  else
    PLILIB         = mtipli.dll
  endif

  VSIMPATH         = $(shell which vsim)
  SIMROOT          = $(shell dirname $(VSIMPATH))/..
  PLILIBPATH       = $(shell find $(SIMROOT) -name "$(PLILIB)")
  MODEL_TECH       = $(shell dirname $(PLILIBPATH))
endif

# Specify the VProc makefile and .do files to use with simulator
# based on whether VHDL has been specified (or the default VERILOG)
ifeq ("$(HDL)", "VHDL")
  MAKEFILEARG      = HDL=VHDL
  SIMDO            = sim_vhdl.do
  SIMGDO           = simg_vhdl.do
  SIMLOGDO         = simlog_vhdl.do
  COMPDO           = compile_vhdl.do
else
  MAKEFILEARG      = HDL=VERILOG
  SIMDO            = sim.do
  SIMGDO           = simg.do
  SIMLOGDO         = simlog.do
  COMPDO           = compile.do
endif

#------------------------------------------------------
# BUILD RULES
#------------------------------------------------------

# Build is dependant on processing makefile in vproc and riscV/iss
all: vproc

# Call the vproc make file for it to determine if anything
# needs building.
.PHONY : vproc
vproc: $(VPROC_TOP)
	@$(MAKE_EXE) --no-print-directory           \
                 $(MAKEFILEARG)                 \
                 $(CURDIR)/VProc.so             \
                 -C $(VPROC_TOP)/test           \
                 USRFLAGS=$(USRCFLAGS)          \
                 ARCHFLAG=$(ARCHFLAG)           \
                 USRCDIR=$(CURDIR)/src          \
                 USER_C="$(USERCODE)"           \
                 MEM_C="$(TCPCODE)"             \
                 MEMMODELDIR="$(CURDIR)/../src" \
                 TESTDIR=$(CURDIR)              \
                 MODELSIMDIR="$(MODEL_TECH)\.."

#
# Checkout VProc from github if it doesn't exist at the specified location
#
${VPROC_TOP}:
	git clone $(VPROC_REPO) $(VPROC_TOP) --recursive

#------------------------------------------------------
# EXECUTION RULES
#------------------------------------------------------

run: all
	@$(VSIMEXE) -c -do $(SIMDO) $(VSIMARGS)

rungui: all
	@$(VSIMEXE) -gui -do wave.do -do $(SIMGDO) $(VSIMARGS)

runlog: all
	@awk -F" " '/add wave/{print "log " $$NF}' < wave.do > batch.do
	@$(VSIMEXE) -c -do $(SIMLOGDO) $(VSIMARGS)


compile:
	@$(VSIMEXE) -c -do $(COMPDO) -do quit

waves:
	@$(VSIMEXE) -view vsim.wlf -do wave.do

gui: rungui

log: runlog

help:
	@echo "make help                     Display this message"
	@echo "make [HDL=VHDL]               Build C/C++ code without running simulation"
	@echo "make [HDL=VHDL] compile       Build HDL code without running simulation"
	@echo "make [HDL=VHDL] run           Build and run batch simulation"
	@echo "make [HDL=VHDL] rungui|gui    Build and run GUI simulation"
	@echo "make [HDL=VHDL] runlog|log    Build and run batch simulation with signal logging"
	@echo "make waves                    Run wave view (to view runlog signals)"
	@echo "make clean                    clean previous build artefacts"

#------------------------------------------------------
# CLEANING RULES
#------------------------------------------------------

clean: $(VPROC_TOP)
	@$(MAKE_EXE) --no-print-directory -C $(VPROC_TOP)/test USER_C="$(USERCODE)" TESTDIR="$(CURDIR)" clean
	@vsim -c -do cleanvlib.do -do quit
	@rm -rf transcript *wlf*
