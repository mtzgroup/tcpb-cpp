# TeraChem Protocol Buffer (TCPB) Client #

This repository is designed to facilitate the development a C++ client for communicating with TeraChem.

This client uses C-style sockets for communication, and Protocol Buffers for a clean, well-defined way to serialize TeraChem input & output.

## Installation

* Ensure `protoc` and `libprotobuf.so` are in your `PATH` and `LD_LIBRARY_PATH`, respectively

* Add or remove the `-DSOCKETLOGS` flag based on whether you want verbose output (off by default)

* Change `PREFIX` in the Makefile to point to your install location

* Run `make` and `make install`

* Add `$PREFIX/include` to `CPLUS_INCLUDE_PATH` (for C++)

* Add `$PREFIX/lib` to `LD_LIBRARY_PATH` (for linking) and `LIBRARY_PATH` (for runtime)

## Contact

* Stefan Seritan <sseritan@stanford.edu>
