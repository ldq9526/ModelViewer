if [ ! -d "build" ]; then
	mkdir build
else
	echo "build/ exists"
fi
cd build
cmake ..
make
