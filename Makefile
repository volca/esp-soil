default:
	pio run

clean:
	pio run -t clean

flash:
	pio run -t upload

monitor:
	pio device monitor
