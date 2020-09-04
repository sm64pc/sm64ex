set -e
if [ $# -eq 0 ]; then
    make BETTERCAMERA=1 NODRAWINGDISTANCE=1 DEBUG=1 IMMEDIATELOAD=1
else
    make BETTERCAMERA=1 NODRAWINGDISTANCE=1 DEBUG=1 IMMEDIATELOAD=1 STRICT=1
fi

# find file
FILE=./build/us_pc/sm64.us.f3dex2e.exe
if [ ! -f "$FILE" ]; then
    FILE=./build/us_pc/sm64.us.f3dex2e
fi

$FILE --server 27015 --configfile sm64config_server.txt  &

# debug if cgdb exists
if ! [ -x "$(command -v cgdb)" ]; then
    $FILE --client 127.0.0.1 27015 --configfile sm64config_client.txt  &
else
    winpty cgdb $FILE -ex 'break debug_breakpoint_here' -ex 'run --client 127.0.0.1 27015 --configfile sm64config_client.txt' -ex 'quit'
fi
