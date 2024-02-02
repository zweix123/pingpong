../build/pingpong/server > ../pingpong_trace.txt &
sleep 2
../build/pingpong/client > ../pingpong.txt
sleep 2
ps aux | grep "server" | grep "pingpong" | awk '{print $2}' | xargs kill
sleep 2