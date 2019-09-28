CFLAGS=-g -O0 -Wall -Wextra -Isrc -DNDEBUG $(OPTFLAGS)

LIBS=-ldl $(OPTLIBS)

PREFIX?=/usr/local


COMMONS_LIB_DIR=../commons
COMMONS_LIB = $(COMMONS_LIB_DIR)/build/libnet_commons.a

MAIN_SOURCES=$(wildcard src/main/**/*.c src/main/*.c )
MAIN_OBJECTS=$(patsubst %.c,%.o, $(MAIN_SOURCES))

LIB_SOURCES=$(wildcard src/lib/**/*.c src/lib/*.c )
LIB_OBJECTS=$(patsubst %.c,%.o, $(LIB_SOURCES))

TEST_SRC=$(wildcard tests/*_tests.c)
TESTS=$(patsubst %.c,%,$(TEST_SRC))

MAIN_TARGET=../bin/mipd
LIB_TARGET=build/libmipd.a


# The Target Build
all: commons $(LIB_TARGET) $(MAIN_TARGET) tests

dev: CFLAGS=-g -Wall -Isrc -Wall -Wextra $(OPTFLAGS)

dev: all

# Exectable
$(MAIN_TARGET): build $(MAIN_OBJECTS)
	cc $(CFLAGS) $(MAIN_SOURCES) $(LIB_TARGET) $(COMMONS_LIB) -o $(MAIN_TARGET)
	chmod +x $(MAIN_TARGET)

# Libary used by executable
$(LIB_TARGET): CFLAGS += $(COMMONS_LIB)
$(LIB_TARGET): CFLAGS += -fPIC
$(LIB_TARGET): build $(LIB_OBJECTS)
	ar rcs $@ $(LIB_OBJECTS)
	ranlib $@

build:
	@mkdir -p build
	@mkdir -p ../../bin

# The Unit Tests
.PHONY: tests


tests: $(TESTS)
	sh ./tests/runtests.sh

%_tests:
	cc $(CFLAGS) -o $@ $@.c $(LIB_TARGET) $(COMMONS_LIB) 

# The Cleaner 

clean:
	rm -rf bin build $(MAIN_OBJECTS) $(LIB_OBJECTS) $(TESTS)
	rm -f tests/tests.log
	find . -name "*.gc*" -exec rm {} \;
	rm -rf `find . -name "*.dSYM" -print`

commons:
	$(MAKE) -C $(COMMONS_LIB_DIR)