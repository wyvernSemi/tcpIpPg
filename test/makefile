###################################################################
# Makefile for TCP/IPv4 packet generator (tcp_ip_pg) test bench
#
# Copyright (c) 2021 Simon Southwell
#
###################################################################

#
# Ensure that the MODEL_TECH environment variable has been setup and
# points to the correct ModelSim location
#

# User files to build, passed in to vproc makefile build
USERCODE           = VUserMain0.cpp VUserMain1.cpp tcpTest0.cpp tcpTest1.cpp tcpConnect.cpp
TCPCODE            = tcpIpPg.cpp

# Set up Variables for tools
MAKE_EXE           = make
VSIMEXE            = ${MODEL_TECH}\\vsim
VSIMARGS           =

# VProc location, relative to this directory
VPROCDIR           = ../../vproc

USRCFLAGS          = "-I${CURDIR}/../src"

#------------------------------------------------------
# BUILD RULES
#------------------------------------------------------

# Build is dependant on processing makefile in vproc and riscV/iss
all: vproc

# Call the vproc make file for it to determine if anything
# needs building.
.PHONY : vproc
vproc:
	@${MAKE_EXE} ${CURDIR}/VProc.so             \
                 -C ${VPROCDIR}                 \
                 USRFLAGS=${USRCFLAGS}          \
                 USRCDIR=${CURDIR}/src          \
                 USER_C="${USERCODE}"           \
                 MEM_C="${TCPCODE}"             \
                 MEMMODELDIR="${CURDIR}/../src" \
                 TESTDIR=${CURDIR}              \
                 MODELSIMDIR="${MODEL_TECH}\.."

#------------------------------------------------------
# EXECUTION RULES
#------------------------------------------------------

run: all
	@${VSIMEXE} -c -do sim.do ${VSIMARGS}

rungui: all
	@${VSIMEXE} -gui -do wave.do -do simg.do ${VSIMARGS}
    
runlog: all
	@awk -F" " '/add wave/{print "log " $$NF}' < wave.do > batch.do
	@${VSIMEXE} -c -do simlog.do ${VSIMARGS}
    

compile:
	@${VSIMEXE} -c -do compile.do -do quit

waves:
	@${VSIMEXE} -view vsim.wlf -do wave.do

gui: rungui

sim: run

log: runlog

help:
	@echo "make help          Display this message"
	@echo "make               Build C/C++ code without running simulation"
	@echo "make compile       Build HDL code without running simulation"
	@echo "make run/sim       Build and run batch simulation"
	@echo "make rungui/gui    Build and run GUI simulation"
	@echo "make runlog/log    Build and run batch simulation with signal logging"
	@echo "make waves         Run wave view in free starter ModelSim (to view runlog signals)"
	@echo "make clean         clean previous build artefacts"

#------------------------------------------------------
# CLEANING RULES
#------------------------------------------------------

clean:
	@${MAKE_EXE} -C ${VPROCDIR} USER_C="${USERCODE}" TESTDIR="${CURDIR}" clean
	@rm -rf "wlf*"
	@rm -rf "work"
	@vsim -c -do cleanvlib.do -do quit
	@rm -rf transcript
