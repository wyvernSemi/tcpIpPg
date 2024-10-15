# Create clean libraries
do cleanvlib.do

# Compile the code into the appropriate libraries
do compile_vhdl.do

# Run the tests
vsim -gGUI_RUN=1 -t 100ps -gui tb
set StdArithNoWarnings   1
set NumericStdNoWarnings 1
do wave.do
run -all
