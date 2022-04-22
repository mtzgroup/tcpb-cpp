# C++ TeraChem Protocol Buffer (TCPB) Client #

This repository is designed to facilitate the communication between TeraChem and third party software.

The client and server (set by the TeraChem executable) use C-style sockets for communication, and Protocol Buffers for a clean, well-defined way to serialize TeraChem input & output data.

## Requirements

* Protocol Buffers >= 3.2.0 (`protoc` and `libprotobuf.so` for C++)

## Installation (with configure script)

* Ensure `protoc` and `libprotobuf.so` are in your `PATH` and `LD_LIBRARY_PATH`, respectively

* Run `./configure gnu`. Other compiler options are intel and clang (not tested). To pick another install location, like /usr/local, run `./configure --prefix=/usr/local gnu`

* Run `make install`

* To install the Python interface `pytcpb`, run `make pytcpb`. After installation, the API functions can be called from your custom Python script. Refer to `api_examples/python` for usage example.

* Add the absolute path to `lib` into `LD_LIBRARY_PATH`

## Installation (with CMake)

* Ensure you have CMake version 3.8.0 or higher

* Run `mkdir build && cd build`

* To install, for example, with GNU compilers at /usr/local, run `cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DCOMPILER=GNU`

* Run `make install`

* The Python interface `pytcpb` is installed by default. To deactivate its installation, add `-DBUILD_PYTHON=FALSE` into your cmake command. After installation, the API functions can be called from your custom Python script. Refer to `api_examples/python` for usage example.

* Add the absolute path to `../lib` into `LD_LIBRARY_PATH`

## Tests

* After installation with configure script, run `make test`. With CMake, the tests are automatically compiled and placed at the folder `tests`

## Examples

* Native examples: after installation with configure script, run `make example`. With CMake, the examples are automatically compiled and placed at the corresponding folder inside the folder `examples`

* Examples using the API from different programming languages are available at the folder `examples/api`.

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
