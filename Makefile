UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
    CC=clang++
    CC+=-D_XOPEN_SOURCE
    LIBVMAPP=libvm_app_macos.o
    LIBVMPAGER=libvm_pager_macos.o
else
    CC=g++-13
    LIBVMAPP=libvm_app.o
    LIBVMPAGER=libvm_pager.o
endif

CC+=-g -Wall -fno-builtin -std=c++17

# List of source files for your pager
PAGER_SOURCES=vm_pager.cpp

# Generate the names of the pager's object files
PAGER_OBJS=${PAGER_SOURCES:.cpp=.o}

all: pager test1 testcheckdata

# Compile the pager and tag this compilation
pager: ${PAGER_OBJS} ${LIBVMPAGER}
	${CC} -o $@ $^
# Compile an application program
test1: test1.4.cpp ${LIBVMAPP}
	${CC} -o $@ $^ -ldl

# Compile an application program
testcheckdata: testcheckdata.4.cpp ${LIBVMAPP}
	${CC} -o $@ $^ -ldl

# Generic rules for compiling a source file to an object file
%.o: %.cpp
	${CC} -c $<
%.o: %.cc
	${CC} -c $<

clean:
	rm -f ${PAGER_OBJS} pager test1 testcheckdata
