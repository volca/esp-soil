default:
	pio run

clean:
	pio run -t clean

flash:
	pio run -t upload

monitor:
	pio device monitor

dfu:
	mkdfu.py write -o .pio/build/dfu.bin --json flasher_args.json

dfu-flash:
	dfu-util -D .pio/build/dfu.bin
