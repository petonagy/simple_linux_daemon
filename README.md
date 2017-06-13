# Simple linux daemon example

This is an example of simple linux daemon implementation. Daemon is listening on TCP port 5001. It behaves as multithread server.

## Available commands:

* cpu - System CPU usage
* mem - System memory usage

## TODO:

* CPU usage measure as separate thread measuring CPU usage in time intervals
* More sophisticated signal handling
* Server running in loop - not just one command