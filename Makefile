CXX = g++
FLAGS = -g -Wno-deprecated -fPIC -m64 -fno-inline -Wno-write-strings

# ROOT
DEPS += $(shell root-config --cflags)
LIBS += $(shell root-config --glibs)

# image libs
LIBS += -L/usr/local/lib
LIBS += -lDDCore
LIBS += -lDD4pod -lpodio -lpodioRootIO -ledm4hep
LIBS += -lfmt
LIBS += -lG4global -lG4materials -lG4geometry -lG4persistency
DEPS += -I/usr/local/include

# local libs
LIBS += -L${EIC_SHELL_PREFIX}/lib
LIBS += -lIRT
DEPS += -I${EIC_SHELL_PREFIX}/include/IRT

#--------------------------------------------

INSTALL_PREFIX = bin
EXECUTABLES := $(addprefix $(INSTALL_PREFIX)/, $(basename $(notdir $(wildcard src/*.cpp))))

#--------------------------------------------

all: $(EXECUTABLES)

clean:
	@echo "CLEAN ======================================================"
	$(RM) $(EXECUTABLES)

#--------------------------------------------

$(INSTALL_PREFIX)/%: src/%.cpp
	mkdir -p $(INSTALL_PREFIX)
	@echo "----- build $@.o -----"
	$(CXX) -c $^ -o $@.o $(FLAGS) $(DEPS)
	@echo "--- make executable $@"
	$(CXX) -o $@ $@.o $(LIBS)
	$(RM) $@.o
