# C++ TeraChem Protocol Buffer (TCPB) Client #

This repository is designed to facilitate the communication between TeraChem and third party software.

The client and server (set by the TeraChem executable) use C-style sockets for communication, and Protocol Buffers for a clean, well-defined way to serialize TeraChem input & output.

## Requirements

* C++11 compiler

* Protocol Buffers >= 3.2.0 (`protoc` and `libprotobuf.so` for C++)

## Installation

* Ensure `protoc` and `libprotobuf.so` are in your `PATH` and `LD_LIBRARY_PATH`, respectively

* Run `./configure gnu`. Other compiler options are intel and clang (not tested). To pick another install location, like /usr/local, run `./configure --prefix=/usr/local gnu`

* Run `make install`

* Add `include` to `CPLUS_INCLUDE_PATH` (for C++)

* Add `lib` to `LD_LIBRARY_PATH` (for linking) and `LIBRARY_PATH` (for runtime)

## Tests

* After installation, run `make test`

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
* Vinicius Wilian D. Cruzeiro <vwcruz@stanford.edu>