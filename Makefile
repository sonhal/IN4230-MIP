CFLAGS=-g -O0 -Wall -Wextra -Isrc -DNDEBUG $(OPTFLAGS)

LIBS=-ldl $(OPTLIBS)

PREFIX?=/usr/local

COMMONS_DIR=commons/
MIPD_DIR=mipd/
PING_CLIENT_DIR=ping-client/
PING_SERVER_DIR=ping-server/

all: build
	$(MAKE) -C $(COMMONS_DIR)
	$(MAKE) -C $(MIPD_DIR)
	$(MAKE) -C $(PING_CLIENT_DIR)
	$(MAKE) -C $(PING_SERVER_DIR)

dev: build
	$(MAKE) dev -C $(COMMONS_DIR)
	$(MAKE) dev -C $(MIPD_DIR)
	$(MAKE) dev -C $(PING_CLIENT_DIR)
	$(MAKE) dev -C $(PING_SERVER_DIR)

build:
	@mkdir -p build
	@mkdir -p bin

# The Unit Tests
.PHONY: tests


tests: $(TESTS)
	sh ./tests/runtests.sh

# The Cleaner 

clean:
	rm -rf bin build
	$(MAKE) clean -C $(COMMONS_DIR)
	$(MAKE) clean -C $(MIPD_DIR)
	$(MAKE) clean -C $(PING_CLIENT_DIR)
	$(MAKE) clean -C $(PING_SERVER_DIR)
	find . -name "*.gc*" -exec rm {} \;
	rm -rf `find . -name "*.dSYM" -print`