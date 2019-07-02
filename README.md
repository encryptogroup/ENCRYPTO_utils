# ENCRYPTO_utils  [![Build Status](https://travis-ci.org/encryptogroup/ENCRYPTO_utils.svg?branch=master)](https://travis-ci.org/encryptogroup/ENCRYPTO_utils)
Crypto and networking utils used for ABY and OTExtension

## Build

This library can be built in the following way. If the
[Relic](https://github.com/relic-toolkit/relic) sources cannot be found on the system, it is
used via the git submodule `extern/relic` and automatically compiled statically.

    $ mkdir build && cd build
    $ cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/installation
    $ make -j $(nproc)

If desired, the build can be installed to the chosen `DCMAKE_INSTALL_PREFIX`.
In case Relic was built in the previous step, it will also be installed.

    $ make install  # if desired

## Tests

Optional tests can be built by setting `-DENCRYPTO_UTILS_BUILD_TESTS=On` when running `cmake` (see above). The test binary will be located in `test/` inside the build directory.


