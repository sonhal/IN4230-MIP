CFLAGS=-g -O2 -Wall -Wextra -Isrc -DNDEBUG $(OPTFLAGS)

LIBS=-ldl $(OPTLIBS)

PREFIX?=/usr/local

SOURCES=$(wildcard src/**/*.c src/*.c)
OBJECTS=$(patsubst %.c,%.o, $(SOURCES))

TEST_SRC=$(wildcard tests/*_tests.c)
TESTS=$(patsubst %.c,%,$(TEST_SRC))

EXE=build/mipd
#TARGET=build/mip.a
#SO_TARGET=$(patsubst %.a,%.so,$(TARGET))

# The Target Build
all: $(EXE)

dev: CFLAGS=-g -Wall -Isrc -Wall -Wextra $(OPTFLAGS)

dev: all

$(EXE): build $(OBJECTS)
	cc -o ${EXE} ${SOURCES}
	chmod +x {EXE}

$(TARGET): build $(OBJECTS)
	ar rcs $@ $(OBJECTS)
	ranlib $@

$(SO_TARGET): $(TARGET) $(OBJECTS)
	$(cc) -shared -o $@ $(OBJECTS)

build:
	@mkdir -p build
	@mkdir -p bin

# The Unit Tests
.PHONY: tests


tests: $(TESTS)
	sh ./tests/runtests.sh

# The Cleaner 

clean:
	rm -rf build $(OBJECTS) $(TEST_SRC)
	rm -f tests/tests.log
	find . -name "*.gc*" -exec rm {} \;
	rm -rf `find . -name "*.dSYM" -print`