default:
	pio run

clean:
	pio run -t clean

flash:
	pio run -t upload

monitor:
	pio device monitor

archive:
	mkdir -p build
	git archive --format=zip --output=build/archive.zip HEAD
