./build/src/server > ./pingpong_sleep_trace.txt &
echo server run
sleep 1
echo client run, it\'s very slow
./build/src/client_sleep > ./pingpong_sleep.txt
sleep 2
ps aux | grep "server" | grep "./build/src" | awk '{print $2}' | xargs kill
sleep 2
echo sleep fin


./build/src/server > ./pingpong_nosleep_trace.txt &
echo server run
sleep 2
./build/src/client_nosleep > ./pingpong_nosleep.txt
echo client run
sleep 2
ps aux | grep "server" | grep "./build/src" | awk '{print $2}' | xargs kill
sleep 2
echo nosleep fin

python3.10 profiling.py