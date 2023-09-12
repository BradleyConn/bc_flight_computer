Add this to .bashrc

export PICO_SDK_PATH={YOUR_DIR_HERE}/bc_flight_computer/Software/pico-sdk/

And maybe this

export PICO_BOARD_HEADER_DIRS={YOUR_DIR_HERE}/bc_flight_computer/Software/flight_software/bsp


`cmake -B./build -S./.` for the cmake command
then `cd build`
then `make`
