onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -expand -group tb /tb/CLK_FREQ_KHZ
add wave -noupdate -expand -group tb /tb/GUI_RUN
add wave -noupdate -expand -group tb /tb/clk
add wave -noupdate -expand -group tb /tb/reset_n
add wave -noupdate -expand -group tb /tb/count
add wave -noupdate -expand -group tb /tb/halt
add wave -noupdate -expand -group tb -radix hexadecimal /tb/txc
add wave -noupdate -expand -group tb -radix hexadecimal /tb/txd
add wave -noupdate -expand -group tb -radix hexadecimal /tb/rxc
add wave -noupdate -expand -group tb -radix hexadecimal /tb/rxd
add wave -noupdate -group node0 /tb/node0/Addr
add wave -noupdate -group node0 /tb/node0/DataIn
add wave -noupdate -group node0 /tb/node0/DataOut
add wave -noupdate -group node0 /tb/node0/NODE
add wave -noupdate -group node0 /tb/node0/RD
add wave -noupdate -group node0 /tb/node0/Update
add wave -noupdate -group node0 /tb/node0/UpdateResponse
add wave -noupdate -group node0 /tb/node0/WE
add wave -noupdate -group node0 /tb/node0/clk
add wave -noupdate -group node0 /tb/node0/count
add wave -noupdate -group node0 /tb/node0/halt
add wave -noupdate -group node0 /tb/node0/nodenum
add wave -noupdate -group node0 /tb/node0/rxc
add wave -noupdate -group node0 /tb/node0/rxc_int
add wave -noupdate -group node0 /tb/node0/rxd
add wave -noupdate -group node0 /tb/node0/rxd_int
add wave -noupdate -group node0 /tb/node0/txc
add wave -noupdate -group node0 /tb/node0/txd
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {377348 ns} 0}
quietly wave cursor active 1
configure wave -namecolwidth 150
configure wave -valuecolwidth 100
configure wave -justifyvalue left
configure wave -signalnamewidth 1
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits ns
update
WaveRestoreZoom {0 ns} {1720 ns}
