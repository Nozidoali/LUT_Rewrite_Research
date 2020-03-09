echo "make" > result.out
make
echo "adder_depth_2018\n" >> result.out
./contest.out -i benchmarks/adder_depth_2018.blif -o output.blif -l 6       >>   result.out
echo "arbiter_depth_2018\n" >> result.out
./contest.out -i benchmarks/arbiter_depth_2018.blif -o output.blif -l 6     >>   result.out
echo "bar_depth_2015\n" >> result.out
./contest.out -i benchmarks/bar_depth_2015.blif -o output.blif -l 6         >>   result.out
echo "cavlc_depth_2018\n" >> result.out
./contest.out -i benchmarks/cavlc_depth_2018.blif -o output.blif -l 6       >>   result.out
echo "ctrl_depth_2017\n" >> result.out
./contest.out -i benchmarks/ctrl_depth_2017.blif -o output.blif -l 6        >>   result.out
echo "dec_depth_2018\n" >> result.out
./contest.out -i benchmarks/dec_depth_2018.blif -o output.blif -l 6         >>   result.out
echo "div_depth_2018\n" >> result.out
./contest.out -i benchmarks/div_depth_2018.blif -o output.blif -l 6         >>   result.out
echo "hyp_depth_2018\n" >> result.out
./contest.out -i benchmarks/hyp_depth_2018.blif -o output.blif -l 6         >>   result.out
echo "i2c_depth_2018\n" >> result.out
./contest.out -i benchmarks/i2c_depth_2018.blif -o output.blif -l 6         >>   result.out
echo "int2float_depth_2018\n" >> result.out
./contest.out -i benchmarks/int2float_depth_2018.blif -o output.blif -l 6   >>   result.out
echo "log2_depth_2018\n" >> result.out
./contest.out -i benchmarks/log2_depth_2018.blif -o output.blif -l 6        >>   result.out
echo "max_depth_2018\n" >> result.out
./contest.out -i benchmarks/max_depth_2018.blif -o output.blif -l 6         >>   result.out
echo "mem_ctrl_depth_2018\n" >> result.out
./contest.out -i benchmarks/mem_ctrl_depth_2018.blif -o output.blif -l 6    >>   result.out
echo "multiplier_depth_2018\n" >> result.out
./contest.out -i benchmarks/multiplier_depth_2018.blif -o output.blif -l 6  >>   result.out
echo "priority_depth_2018\n" >> result.out
./contest.out -i benchmarks/priority_depth_2018.blif -o output.blif -l 6    >>   result.out
echo "router_depth_2018\n" >> result.out
./contest.out -i benchmarks/router_depth_2018.blif -o output.blif -l 6      >>   result.out
echo "sin_depth_2018\n" >> result.out
./contest.out -i benchmarks/sin_depth_2018.blif -o output.blif -l 6         >>   result.out
echo "sqrt_depth_2018\n" >> result.out
./contest.out -i benchmarks/sqrt_depth_2018.blif -o output.blif -l 6        >>   result.out
echo "square_depth_2018\n" >> result.out
./contest.out -i benchmarks/square_depth_2018.blif -o output.blif -l 6      >>   result.out
echo "voter_depth_2018\n" >> result.out
./contest.out -i benchmarks/voter_depth_2018.blif -o output.blif -l 6       >>   result.out
