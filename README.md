Welcome to project Cuckoo.

This is an application that will run on a Gen 2 nest thermostat to re-purpose them.

In order to load this software onto the nest, you will need root access. The following project explains how to do this:
https://github.com/ajb142/cuckoo_loader

There is a public discord server for discussions on ideas and feedback:
https://discord.gg/VpWvwuEUwa

# Build instruction
## Build

The project supports two build configurations using CMake toolchain files:
- **ARM cross-compilation** (for Nest thermostat): Uses `cmake/arm-toolchain.cmake`
- **Host native compilation** (for testing): Uses `cmake/host-toolchain.cmake`

### ARM Build (for Nest Thermostat)
```bash
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-toolchain.cmake ..
cmake --build .
cd ..
```

### Host Build (for local testing)
```bash
mkdir build-host && cd build-host
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/host-toolchain.cmake ..
cmake --build .
cd ..
```

**Note:** The ARM build will automatically download the Linaro toolchain on first run. This may take a few minutes.

## Upload
SSH to the Nest and start a simple server to receive the file:
```
nc -l -p 51234 > ./cuckoo
```

On the build host. send the cuckoo binary via netcat:
```
cat ./build/output/cuckoo | nc -w1 NEST_IP_ADDR 51234
```

Back on the nest, alter permissions:
```
chmod +x ./cuckoo
```
## Run
Stop the nest client so we can control hardware:
```
/etc/init.d/nestlabs stop
```

Now you can run with
```
./cuckoo
```
