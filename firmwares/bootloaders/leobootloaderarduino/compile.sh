export PATH=$PATH:./pkg-x86_64-unknown-linux-gnu/bin

echo "Compiling LEO 32u4 bootloader modified by Dreg"
rm leobootloaderarduino.hex
cd Caterina2
rm Caterina2.hex
make clean && ARDUINO_MODEL_PID=0x0036 make #Arduino Leonardo
cp Caterina2.hex ../leobootloaderarduino.hex
cd ..
