default:
	cp -X src/*.py /Volumes/CIRCUITPY/

monitor:
	pio device monitor -b 115200
