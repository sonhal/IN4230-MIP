CFLAGS=-g -O0 -Wall -Wextra -Isrc -DNDEBUG $(OPTFLAGS)

LIBS=-ldl $(OPTLIBS)

PREFIX?=/usr/local

COMMONS_LIB_DIR=../commons
COMMONS_LIB = $(COMMONS_LIB_DIR)/build/libnet_commons.a

SOURCES=$(wildcard src/**/*.c src/*.c )
OBJECTS=$(patsubst %.c,%.o, $(SOURCES))

TARGET=../bin/ping_client


# The Target Build
all: $(TARGET)

dev: CFLAGS=-g -Wall -Isrc -Wall -Wextra $(OPTFLAGS)

dev: all

$(TARGET): build commons $(OBJECTS)
	cc $(CFLAGS) $(SOURCES) $(COMMONS_LIB) -o $(TARGET)
	chmod +x $(TARGET)

build:
	@mkdir -p build
	@mkdir -p bin

# The Cleaner 

clean:
	rm -rf bin build $(OBJECTS) $(TESTS)
	rm -f tests/tests.log
	find . -name "*.gc*" -exec rm {} \;
	rm -rf `find . -name "*.dSYM" -print`


commons:
	$(MAKE) -C $(COMMONS_LIB_DIR)