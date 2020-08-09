set -e
make BETTERCAMERA=1 NODRAWINGDISTANCE=1 DEBUG=1 IMMEDIATELOAD=1

# find file
FILE=./build/us_pc/sm64.us.f3dex2e.exe
if [ ! -f "$FILE" ]; then
    FILE=./build/us_pc/sm64.us.f3dex2e
fi

$FILE --server --configfile sm64config_server.txt  &

# debug if cgdb exists
if ! [ -x "$(command -v cgdb)" ]; then
    $FILE --client --configfile sm64config_client.txt  &
else
    winpty cgdb $FILE -ex 'break debug_breakpoint_here' -ex 'run --client --configfile sm64config_client.txt' -ex 'quit'
fi
