default:
	pio run -t upload

monitor:
	pio device monitor -b 115200 -p /dev/cu.SLAB_USBtoUART
