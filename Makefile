CFLAGS=-g -O0 -Wall -Wextra -Isrc -DNDEBUG $(OPTFLAGS)

LIBS=-ldl $(OPTLIBS)

PREFIX?=/usr/local

SOURCES=$(wildcard src/**/*.c src/*.c )
SOURCES := $(filter-out src/client/ping_client.c src/server/ping_server.c , $(SOURCES))
OBJECTS=$(patsubst %.c,%.o, $(SOURCES))

TEST_SRC=$(wildcard tests/*_tests.c)
TESTS=$(patsubst %.c,%,$(TEST_SRC))

TARGET=bin/mipd

CLIENT=bin/ping_client
CLIENT_SRC=$(wildcard src/**/*.c src/*.c )
CLIENT_SRC := $(filter-out src/mipd.c src/server/ping_server.c, $(CLIENT_SRC))

SERVER=bin/ping_server
SERVER_SRC=$(wildcard src/**/*.c src/*.c )
SERVER_SRC := $(filter-out src/mipd.c src/client/ping_client.c, $(SERVER_SRC))
#TARGET=build/mip.a
#SO_TARGET=$(patsubst %.a,%.so,$(TARGET))

# The Target Build
all: $(TARGET) $(CLIENT) $(SERVER)

dev: CFLAGS=-g -Wall -Isrc -Wall -Wextra $(OPTFLAGS)

dev: all

$(TARGET): build $(OBJECTS)
	cc $(CFLAGS) -o $(TARGET) $(SOURCES)
	chmod +x $(TARGET)

$(CLIENT): 
	cc $(CFLAGS) -o $(CLIENT) $(CLIENT_SRC)
	chmod +x $(CLIENT)

$(SERVER): 
	cc $(CFLAGS) -o $(SERVER) $(SERVER_SRC)
	chmod +x $(SERVER)

build:
	@mkdir -p build
	@mkdir -p bin

# The Unit Tests
.PHONY: tests


tests: $(TESTS)
	sh ./tests/runtests.sh

# The Cleaner 

clean:
	rm -rf bin build $(OBJECTS) $(TESTS)
	rm -f tests/tests.log
	find . -name "*.gc*" -exec rm {} \;
	rm -rf `find . -name "*.dSYM" -print`