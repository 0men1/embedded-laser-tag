FQBN=esp32:esp32:esp32
PORT=/dev/cu.usbserial-0001
BAUD=115200
TARGET?=blaster

.PHONY: all compile upload flash monitor clean

all: flash monitor

compile:
	arudino-cli compile --fqbn $(FQBN) $(TARGET)

upload:
	arduino-cli upload -p $(PORT) --fqbn $(FQBN) $(TARGET)

flash:
	arduino-cli compile --upload -p $(PORT) --fqbn $(FQBN) $(TARGET)

monitor:
	arduino-cli monitor -p $(PORT) --config baudrate=$(BAUD)

clean:
	arduino-cli cache clean
