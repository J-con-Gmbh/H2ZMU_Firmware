
# INSTALLATION GUIDE

## Environment

You need 2 scripts that you can find [here](https://www.phoenixcontact.com/fr-fr/produits/commande-axc-f-2152-2404267).
* PLCnext_Toolchain_linux_2022.6.sh
* pxc-glibc-x86_64-axcf2152-image-sdk-cortexa9t2hf-neon-axcf2152-toolchain-2022.6.sh

For PLC_next_Toolchain:
```
chmod +x PLCnext_Toolchain_linux_2022.6.sh
./PLCnext_Toolchain_linux_2022.6.sh (--accept to automatically accept the conditions)
And follow the instructions.
```

For pxc-glibc-x86_64-axcf2152-image-sdk-cortexa9t2hf-neon-axcf2152-toolchain-2022.6.sh:
```
chmod +x pxc-glibc-x86_64-axcf2152-image-sdk-cortexa9t2hf-neon-axcf2152-toolchain-2022.6.sh
./pxc-glibc-x86_64-axcf2152-image-sdk-cortexa9t2hf-neon-axcf2152-toolchain-2022.6.sh
And follow the instructions.
```

## Setup the librairies

After cloning the repo you will find two scripts in the external directory one for grpc one for libmodbus.

* gRPC (current directory: h2zmu-axcf/external)
```
sudo ./build-grpc.sh
```

* libmodbus
(current directory: h2zmu-axcf/external/lib)
```
cd lib
git clone -b "v3.1.10" https://github.com/stephane/libmodbus.git
cd libmodbus
./autogen.sh
./configure --host=arm-linux-gnueabihf --enable-static --prefix=/path/to/h2zmu-axcf/external/build/cmake/external/libmodbus
```

> For the next ones you will need access to the other repos (ask for the access if you don't have them)

* cpp_log (current directory: h2zmu-axcf/project/lib)
```
git clone git@github.com:RigaEDV/cpp_log.git
```

* h2zmu_utils (current directory: h2zmu-axcf/project/lib)
```
git clone git@github.com:RigaEDV/h2zmu_utils.git
```

* uart_comm (current directory: h2zmu-axcf/project/lib)
```
git clone git@github.com:RigaEDV/uart_comm.git
```

* modbus_interface
```
git clone git@github.com:RigaEDV/modbus_interface.git
```

## Build

* Before compiling
```
source /opt/pxc/2022.6/environment-setup-cortexa9t2hf-neon-pxc-linux-gnueabi
```
> This enables the local variables needed by the compiler. (For arm) 
> --> Installed with pxc-glibc-x86_64-axcf2152-image-sdk-cortexa9t2hf-neon-axcf2152-toolchain-2022.6.sh

* Compiling example

```
cmake -DCMAKE_BUILD_TYPE=Debug \ --> Enable debug mode (to use tools like gdb)
      -DCMAKE_TOOLCHAIN_FILE=/opt/pxc/2022.6/toolchain.cmake \ --> Path the toolchain.cmake file installed with PLCnext toolchain 
      -DARP_TOOLCHAIN_ROOT=/opt/pxc/2022.6 \ --> specify the root directory for our environment
      -DCMAKE_STAGING_PREFIX=/path/to/h2zmu-axcf/build/cmake/axcf2152,22.6.0.43/Debug/out \
      -DCMAKE_PREFIX_PATH=/path/to/h2zmu-axcf/external/build/cmake/external/grpc/lib/cmake/grpc \
      -G Ninja \
      -S /path/to/h2zmu-axcf/ \
      -B /path/to/h2zmu-axcf/cmake-build-debug
```

* After compilation

> You should see a new directory called cmake-build-debug
```
cd cmake-build-debug
scp /path/to/h2zmu-axcf/cmake-build-debug/h2zmu-axcf root@ip.to.the.device:/home/root
```

> scp copies the given directory and copy it to te desitnation through ssh (it copies the the project to the device)

### Other info: 

cmake

necessary (cli) cmake options:

 - -DCMAKE_TOOLCHAIN_FILE=**{/path/to/selected/sdk}**/toolchain.cmake 
 - -DARP_TOOLCHAIN_ROOT=**{/path/to/selected/sdk}** 
 - -DCMAKE_PROGRAM_PATH="**{/path/to/selected/sdk}**/sysroots/x86_64-pokysdk-linux/usr/bin;**{/path/to/project/root}**/build/cmake/external/grpc"
 - -DCMAKE_STAGING_PREFIX=**{/path/to/project/root}**/build/cmake/${buildKit}/${buildType}/out/

recomendet cli cmake options:

 - -G Ninja 
 - -S **{/path/to/project/root}**
 - -B **{/path/to/project/root}**/cmake-build-debug

# gRPC project from Phoenix-Contact

https://github.com/PLCnext/plcnext-grpc-example-cpp

Used source-code ist found [here](plcnext-grpc-example-cpp-main.zip).

<details>
  <summary><i>plcnext-grpc-example-cpp</i></summary>
  <p>

# PLCnext gRPC with C++ Example

This project shows how to use the PLCnext gRPC Service in a C++ application.
The PLCnext gRPC service description files are compiled and then used in a standalone C++ example application.
For more information about gRPC in PLCnext Technology see the [Using gRPC communication](https://www.plcnext.help/te/Service_Components/gRPC_Introduction.htm) topic in the [PLCnext Info Center](https://www.plcnext.help).

This example was tested with the following PLCnext controllers:

* AXC F 2152 FW 2022.6
* AXC F 3152 FW 2023.0

An Example for the toolchain configuration is included as Visual Studio Code [cmake-kits.json](.vscode/cmake-kits.json) file.

## Build the example

How to compile the `*.proto` service files is described in the [PLCnext gRPC repository](https://github.com/PLCnext/gRPC). This compile step is done by the CMake module [`FindPlcnextGrpc.cmake`](cmake/FindPlcnextGrpc.cmake) that is included in this example.

The Protobuf compiler `protoc` is included in the PLCnext SDK. Just declare the CMake variable `CMAKE_PROGRAM_PATH` with the location of the `protoc` executable in the native PLCnext SDK. See the [`cmake-kits.json`](.vscode/cmake-kits.json) file for a working example.

However the gRPC C++ Protocol Buffers plugin `grpc_cpp_plugin` is only available in the SDK sysroot of the target controller. So the plugin can not be used on the development host. Therefor the gRPC C++ plugin has to be compiled from source. This example includes tasks for Visual Studio Code to fetch the gRPC sources an compile them on the local machine. The following tasks are used to fetch and compile gRPC from source:

* Fetch external content: gRPC
* gRPC plugins: CMake configure
* gRPC plugins: CMake build

The vscode task `Fetch external content: gRPC` clones the gRPC git repository into the folder `build/external/grpc`. It can then be configured and build with the other two provided vscode tasks that call CMake. The CMake build directory is defined as `build/cmake/external/grpc` in the vscode task `gRPC plugins: CMake configure`.

Pre-requisites to build gRPC:

```bash
apt-get install build-essential autoconf libtool pkg-config cmake
```

Be aware that the version of gRPC has to match the version that is included in the used PLCnext target. The Visual Studio Code task `Fetch external content: gRPC` has to be adjusted to clone the right git tag. You can find the used version information in the file `/usr/lib/cmake/grpc/gRPCConfigVersion.cmake` that is included in the target sysroot of the SDK. For example for the SDK of the controller AXC F 2152 this is the file `sysroots/cortexa9t2hf-neon-pxc-linux-gnueabi/usr/lib/cmake/grpc/gRPCConfigVersion.cmake`.

For CMake to be able to find the compiled gRPC C++ Protobuf plugin `grpc_cpp_plugin` the CMake module [`FindPlcnextGrpc.cmake`](cmake/FindPlcnextGrpc.cmake) has to be given the path as hint where to search for the plugin binaries. This is done by declaring the CMake variable `CMAKE_PROGRAM_PATH` with the location of the CMake build directory for gRPC. See the [`cmake-kits.json`](.vscode/cmake-kits.json) file for a working example of how to configure CMake.

## Run the example

To be able to run the example on the target controller the `libabsl*.so` shared object libraries have to be copied from the SDK to the target. This was tested with the controller AXC F 2152 firmware 2022.6. This step is not necessary with the controller AXC F 3152 firmware 2023.0.

To be able to copy the files over ssh you need root access to the target. Run the copy command from your development machine and copy the library files from your PLCnext SDK target sysroot to the target:

```bash
scp sysroots/cortexa9t2hf-neon-pxc-linux-gnueabi/usr/lib/libabsl*.so root@192.168.1.10:/usr/lib
```

Then login into your target as root an run:

```bash
ldconfig
```

to create the necessary links and update the linker cache.

Copy the example executable to the controller and call it with a valid port name of the type uint16 to read and write to:

```bash
./plcnext-grpc-example Arp.Plc.Eclr/wDO16
```

## Useful hints

A good lookup on how to use the generated C++ protocol buffer classes is the [C++ Generated Code Guide](https://protobuf.dev/reference/cpp/cpp-generated/) from [Protocol Buffers Documentation](https://protobuf.dev/).


</p>
</details>
