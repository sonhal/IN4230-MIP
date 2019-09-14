CFLAGS=-g -O2 -Wall -Wextra -Isrc -DNDEBUG $(OPTFLAGS)

LIBS=-ldl $(OPTLIBS)

PREFIX?=/usr/local

SOURCES=$(wildcard src/**/*.c src/*.c)
OBJECTS=$(patsubst %.c,%.o, $(SOURCES))

TEST_SRC=$(wildcard tests/*_tests.c)
TESTS=$(patsubst %.c,%,$(TEST_SRC))

TARGET=bin/mipd
#TARGET=build/mip.a
#SO_TARGET=$(patsubst %.a,%.so,$(TARGET))

# The Target Build
all: $(TARGET)

dev: CFLAGS=-g -Wall -Isrc -Wall -Wextra $(OPTFLAGS)

dev: all

$(TARGET): build $(OBJECTS)
	cc -o $(TARGET) $(SOURCES)
	chmod +x $(TARGET)

build:
	@mkdir -p build
	@mkdir -p bin

# The Unit Tests
.PHONY: tests


tests: $(TESTS)
	sh ./tests/runtests.sh

# The Cleaner 

clean:
	rm -rf bin build $(OBJECTS) $(TEST)
	rm -f tests/tests.log
	find . -name "*.gc*" -exec rm {} \;
	rm -rf `find . -name "*.dSYM" -print`