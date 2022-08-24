CXX = g++
FLAGS = -g -Wno-deprecated -fPIC -m64 -fno-inline -Wno-write-strings

# ROOT
DEPS += $(shell root-config --cflags)
LIBS += $(shell root-config --glibs)

# image libs
LIBS += -L/usr/local/lib
LIBS += -lDDCore
LIBS += -lDD4pod -lpodio -lpodioRootIO -ledm4hep

# local libs
LIBS += -L${EIC_SHELL_PREFIX}/lib
LIBS += -lIRT
DEPS += -I${EIC_SHELL_PREFIX}/include/IRT

#--------------------------------------------

INSTALL_PREFIX = bin
SRC_MAIN := $(basename $(notdir $(wildcard src/*.cpp)))
SRC_EXAMPLES := $(basename $(notdir $(wildcard src/examples/*.cpp)))

#--------------------------------------------

all: 
	@echo ""
	@echo "BUILDING SOURCES ==========================================="
	@echo "$(SRC_MAIN)"
	@echo "============================================================"
	make main
	@echo ""
	@echo "BUILDING EXAMPLES =========================================="
	@echo "$(SRC_EXAMPLES)"
	@echo "============================================================"
	make examples

main: $(SRC_MAIN)
examples: $(SRC_EXAMPLES)

clean:
	@echo "CLEAN ======================================================"
	$(RM) $(addprefix $(INSTALL_PREFIX)/, $(SRC_MAIN))
	$(RM) $(addprefix $(INSTALL_PREFIX)/, $(SRC_EXAMPLES))

#--------------------------------------------

%: %.o
	@echo "--- make executable $(INSTALL_PREFIX)/$@"
	$(CXX) -o $(INSTALL_PREFIX)/$@ $< $(LIBS)

%.o: src/%.cpp
	mkdir -p $(INSTALL_PREFIX)
	@echo "----- build $@ -----"
	$(CXX) -c $^ -o $@ $(FLAGS) $(DEPS)

%.o: src/examples/%.cpp
	mkdir -p $(INSTALL_PREFIX)
	@echo "----- build $@ -----"
	$(CXX) -c $^ -o $@ $(FLAGS) $(DEPS)
