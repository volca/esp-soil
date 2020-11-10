default:
	cp -X src/code.py /Volumes/CIRCUITPY/

monitor:
	pio device monitor -b 115200
