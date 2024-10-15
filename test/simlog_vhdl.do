# Create clean libraries
do cleanvlib.do

# Compile the code into the appropriate libraries
do compile_vhdl.do

# Run the tests. 
vsim -quiet -t 100ps tb
do batch.do
set StdArithNoWarnings   1
set NumericStdNoWarnings 1
run -all

#Exit the simulations
quit
