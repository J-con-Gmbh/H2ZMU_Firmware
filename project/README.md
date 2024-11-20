# H2ZMU Firmware

[![Build and Test Status](https://github.com/RigaEDV/h2zmu_v1_pfc200/actions/workflows/build_and_test_workflow.yml/badge.svg)](https://github.com/RigaEDV/h2zmu_v1_pfc200/actions) 

## BUILD

### Requirements
#### Install packages

+ cmake
+ make
+ gcc
+ libc6-dev
+ build-essential
+ git
+ git-core
+ libmodbus-dev 

```
  sudo apt-get install cmake make gcc libc6-dev build-essential git git-core libmodbus-dev
```

#### Clone repository

``` 
  git clone https://{secret token}@github.com/RigaEDV/h2zmu_v1_pfc200.git
```

#### Build Project

```
  cd h2zmu_v1_pfc200/
  mkdir build
  cd build/
  cmake .. # modify this line as described in next section
  make
```

### CMake Variables

**PLT** <br>
Set target platform.<br>

* `-DPLT=LINUX_x86_64`: Linux development, default value
* `-DPLT=PFC`: PFC 200 Wago Controller
* `-DPLT=RPI`: RaspberryPi, only for CLion's integrated remote development!<br>

**LIBMODBUS** <br>
Provides the path to libmodbus.so.<br>

* `-DLIBMODBUS=/usr/lib/x86_64-linux-gnu/libmodbus.so`<br>

**LIBMODBUS_INCLUDE_DIR** <br>
Provides the path to libmodbus include directories.<br>

* `-DLIBMODBUS_INCLUDE_DIR=/usr/include/modbus`<br>


***
**NOTE**  
  If compiling for Wago PFC Controller, libmodbus needs to be crosscompiled with an **arm-linux-gnueabihf** toolchain, and **LIBMODBUS** needs to point to that binary. For both **LIBMODBUS** and **LIBMODBUS_INCLUDE_DIR**, cmake will determine default values, which will mostly work for the development machine, but not for crosscompiling for other systems.
***


**LIB_LINK_TYPE** <br>
Lib link type can be **static** or **dynamic**, default ist **static**.<br>

* `-DLIB_LINK_TYPE=dynamic`<br>

**GITHUB_TOKEN** <br>
For cmake to download the necessary dependencies, a GitHub "Personal access token" is needed.<br>
The Token can be provided via an env-variable: **H2ZMU_GITHUB_TOKEN**<br>
or via the cmake variable **GITHUB_TOKEN**: <br>

* `-DGITHUB_TOKEN={secret token}`<br>

**CMAKE_BUILD_TYPE** <br>
Compiles executable in **debug** or **release** mode. Select debug to enable IDE features like breakpoints. Default is **release**.<br>

* `-DCMAKE_BUILD_TYPE=Debug`<br>


A building command could look like this:<br>
```
cmake -DPLT=PFC \
      -DLIBMODBUS=/usr/lib/x86_64-linux-gnu/libmodbus.so \ 
      -DLIBMODBUS_INCLUDE_DIR=/usr/include/modbus \
      -DLIB_LINK_TYPE=dynamic \
      -DGITHUB_TOKEN={secret token} \ 
      -DCMAKE_BUILD_TYPE=Debug \
      /path/to/CMake/dir
```


### CLion IDE Settings
#### Switch: Raspi -> Lokal
* Settings -> CMake -> disable Profile "Debug-H2ZMU"
* Settings -> CMake -> enable Profile "Debug-H2ZMU-local"
* Run -> Run (Alt+Umschalt+F10) -> h2zmu_v1-local


#### Switch: Lokal -> Raspi
* Settings -> CMake -> disable Profile "Debug-H2ZMU-local"
* Settings -> CMake -> enable Profile "Debug-H2ZMU"
* Run -> Run (Alt+Umschalt+F10) -> h2zmu_v1

