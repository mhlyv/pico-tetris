all:
	mkdir -p build
	PICO_SDK_PATH=~/src/pico-sdk cmake -B build -G Ninja
	ninja -C build

clean:
	rm -rf build
