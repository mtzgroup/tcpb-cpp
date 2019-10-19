# C++ TeraChem Protocol Buffer (TCPB) Client & Server #

This repository is designed to facilitate the development a TCP-based interface for TeraChem.

The client and server use C-style sockets for communication, and Protocol Buffers for a clean, well-defined way to serialize TeraChem input & output.

As of v1.0, the client and server are both included here and share wrappers (i.e. TCPB::Input, TCPB::Output) over the underlying serialization protocols (currently just Protocol Buffers, but may be extended to include MsgPack for larger data).

## Requirements

* C++11 compiler (g++ is fine, icpc not yet tested)

* Protocol Buffers >= 3.2.0 (`protoc` and `libprotobuf.so` for C++)

## Installation

* Ensure `protoc` and `libprotobuf.so` are in your `PATH` and `LD_LIBRARY_PATH`, respectively

* Add or remove the `-DSOCKETLOGS` flag based on whether you want verbose output (off by default)

* Change `PREFIX` in the Makefile to point to your install location

* Run `make` and `make install`

* Add `$PREFIX/include` to `CPLUS_INCLUDE_PATH` (for C++)

* Add `$PREFIX/lib` to `LD_LIBRARY_PATH` (for linking) and `LIBRARY_PATH` (for runtime)

## Notes for TeraChem Developers

### Do not break backwards compatibility of the protocol

This means do not reuse tags in the `proto/terachem_server.proto` file,
as this will cause old server/client versions to misinterpret the tags.

Also, it is critically important that the current values of MessageType enum
are not changed since this is a key part of the protocol's header,
determining how the server/client deserialize the incoming message.

### Branches and tests for any new features

Extensions of the protocol are encouraged, but please follow some basic guidelines:

* Develop any new features in a branch to avoid accidentally pushing broken protocols to `master`

* Add test cases for your protocol/job type

* Use the BitBucket Pipelines Docker container to ensure your tests will pass when pushed

* Submit a pull request for your branch to facilitate code review

The `run_pipelines_tests.sh` script automates build, install, and testing for the repo.
The `run_docker_tests.sh` script wraps the magic Docker lines to do this with the container that Pipelines will use.

## Contact

* Stefan Seritan <sseritan@stanford.edu>
