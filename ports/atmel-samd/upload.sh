#/bin/bash

# Open debug server
JLinkGDBServer -if SWD -device ATSAMD21G18 &
JLINK_SERVER_PID=$!

sleep 2

./gdbs

kill $JLINK_SERVER_PID
