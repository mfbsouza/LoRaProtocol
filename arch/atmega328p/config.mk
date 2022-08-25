# config related to AVR ATmega 328p boards

CFLAGS   += -mmcu=atmega328p -DF_CPU=16000000UL
CXXFLAGS += -mmcu=atmega328p -DF_CPU=16000000UL
LDFLAGS  += -mmcu=atmega328p -Wl,-Map=$(BUILDDIR)/memory.map
