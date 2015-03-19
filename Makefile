# This is a GNU makefile. It requires Arduino.mk from the arduino-mk
# package.

# You need to create this, as documented in Arduino.mk. It should
# contain something like this. These values are correct for the files
# installed by FreeBSD ports. Under Linux you will probably need
# something different.
#ARDMK_DIR=	/usr/local/arduino-mk
#ARDUINO_DIR=	/usr/local/arduino
#AVR_TOOLS_DIR=	/usr/local

include config.mk

ARDUINO_LIBS=	Wire
BOARD_TAG=	uno

include ./Arduino.mk

%.ii: %.cpp
	$(CXX) -E $(CPPFLAGS) $(CXXFLAGS) $< > $@

%.s: %.cpp
	$(CXX) -S $(CPPFLAGS) $(CXXFLAGS) -o $@ $<

%.s: build-uno/%.cpp
	$(CXX) -S $(CPPFLAGS) $(CXXFLAGS) -o $@ $<

%.i: %.c
	$(CC) -E $(CPPFLAGS) $(CFLAGS) $< > $@

%.s: %.c
	$(CC) -S $(CPPFLAGS) $(CXXFLAGS) -o $@ $<

