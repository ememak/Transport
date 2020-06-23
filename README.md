# Simple udp client downloading data from server.

## Building the project
To build the executable just type

    make
in console.

To clean repository you have to type

    make distclean

One can also clean only temporary files by command

    make clean

## Usage
./transport IPv4-ADDR PORT FILE SIZE

Transport binary is downloading SIZE bytes from IPv4-ADDR port PORT to file named FILE.

In repository there is also an example server binary named transport-server. 
It's serving on specified port and sending data according to specification.
