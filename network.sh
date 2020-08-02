set -e
make BETTERCAMERA=1 NODRAWINGDISTANCE=1 DEBUG=1 IMMEDIATELOAD=1
./build/us_pc/sm64.us.f3dex2e.exe --server --configfile sm64config_server.txt  &
./build/us_pc/sm64.us.f3dex2e.exe --client --configfile sm64config_client.txt  &
#winpty cgdb ./build/us_pc/sm64.us.f3dex2e.exe -ex 'b act_jump_kick' -ex 'run --client --configfile sm64config_client.txt' -ex 'quit'
