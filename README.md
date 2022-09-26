# C++ TeraChem Protocol Buffer (TCPB) Client #

This repository is designed to facilitate the communication between TeraChem and third party software.

The client and the server (set by the TeraChem executable) use C-style sockets for communication, and Protocol Buffers for a clean, well-defined way to serialize TeraChem input & output data.

## Requirements

* Protocol Buffers >= 3.2.0 (`protoc` and `libprotobuf.so` for C++)

## Cloning this repo and setting up its submodules

```
git clone git@github.com:mtzgroup/tcpb-cpp.git tcpb-cpp
cd tcpb-cpp
git submodule init
git submodule update
```

## Obtaining and installing Protocol Buffers

* Please refer to [this page](https://developers.google.com/protocol-buffers) for instructions on how to download and install Google’s Protocol Buffers libraries and executables.

* At the time of writing, protocol buffers can be downloaded from <https://github.com/protocolbuffers/protobuf/releases> (e.g., `protobuf-cpp-3.20.1.zip`) and installation instructions for C++ are provided at <https://github.com/protocolbuffers/protobuf/blob/main/src/README.md>.

* You might also be able to install Protocol Buffers using your system package manager. For example, on Ubuntu 20.04 you can run

```console
$ sudo apt install protobuf-compiler
```

## Installation (with configure script)

* Ensure `protoc` and `libprotobuf.so` (from Protocol Buffers) are in your `PATH` and `LD_LIBRARY_PATH`, respectively

* Run `./configure gnu` if using GNU compilers or `./configure intel` if using Intel compilers. Other compiler options are intel and clang (not tested). To pick another install location, like /usr/local, run `./configure --prefix=/usr/local gnu`

* Run `make install`

* To compile the C++ and Fortran examples, run `make example`

* To install the Python interface *PyTCPB*, run `make pytcpb`. After installation, the API functions can be called from your custom Python script. Refer to `examples/api/python` for usage example.

* Add the absolute path to `lib` into `LD_LIBRARY_PATH`

## Installation (with CMake)

* Ensure you have CMake version 3.8.0 or higher.

* Run `mkdir build && cd build`

* To install, for example, with GNU compilers at /usr/local, run `cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DCOMPILER=GNU`. To use Intel compilers, the option `-DCOMPILER=INTEL` must be specified instead.

* Run `make install`

* The command above also compiles the C++ and Fortran examples.

* The Python interface *PyTCPB* is installed by default. To deactivate its installation, add `-DBUILD_PYTHON=FALSE` into your CMake command. After installation, the API functions can be called from your custom Python script. Refer to `examples/api/python` for usage example.

* Add the absolute path to `../lib` into `LD_LIBRARY_PATH`

## Running TeraChem in server mode

TeraChem needs to be active in server mode so that TCPB-cpp can connect to it.

TeraChem can be executed in server mode with the following command:
```
terachem -s 12345
```
where `-s` specifies the port number to be used and 12345 is a value picked for illustration purposes only. By default, TeraChem will use all GPUs in the machine, but users can control which GPUs are accessible by using the `-g` in the TeraChem command above or by setting the `CUDA_VISIBLE_DEVICES` environment variable before running the command above.

## Examples

**Compiling C++ and Fortran examples:** as mentioned above, after installation with configure script, run `make example`. With CMake, the examples are automatically compiled and placed at the corresponding folder inside `examples`.

* **Using TCPB-cpp from different programming languages:** examples available at the folder `examples/api`.

* **Native C++ examples:** available at the folders `examples/qm` and `examples/qmmm`.

* **Running example binaries:** By default, all example binaries expect a TeraChem server running on port 12345.

## Notes for TeraChem Developers

### Do not break backwards compatibility of the protocol

This means do not reuse tags in the `proto/terachem_server.proto` file,
as this will cause old server/client versions to misinterpret the tags.

Also, it is critically important that the current values of MessageType enum
are not changed since this is a key part of the protocol's header,
determining how the server/client deserialize the incoming message.

## Contact

* Vinicius Wilian D. Cruzeiro: <vwcruz@stanford.edu>
* Henry Wang: <henryw7@stanford.edu>
