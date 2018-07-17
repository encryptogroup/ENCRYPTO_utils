# ENCRYPTO_utils  [![Build Status](https://travis-ci.org/encryptogroup/ENCRYPTO_utils.svg?branch=master)](https://travis-ci.org/encryptogroup/ENCRYPTO_utils)
Crypto and networking utils used for ABY and OTExtension

## Build

This library can be built in the following way. If
[MIRACL](https://github.com/miracl/MIRACL) cannot be found on the system, it is
used via the git submodule `extern/MIRACL` and automatically compiled.

    $ mkdir build && cd build
    $ cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/installation
    $ make -j $(nproc)

If desired, the build can be installed to the chosen `DCMAKE_INSTALL_PREFIX`.
In case MIRACL was built in the previous step, it will also be installed.

    $ make install  # if desired
