CXX = g++
FLAGS = -g -Wno-deprecated -fPIC -m64 -fno-inline -Wno-write-strings

# ROOT
DEPS += $(shell root-config --cflags)
LIBS += $(shell root-config --glibs)

# image libs
LIBS += -L/usr/local/lib
LIBS += -lDDCore -lDDRec
LIBS += -lpodio -lpodioRootIO -ledm4hep
LIBS += -lfmt
LIBS += -lG4global -lG4materials -lG4geometry -lG4persistency
DEPS += -I/usr/local/include

# local libs
LIBS += -L${EIC_SHELL_PREFIX}/lib
LIBS += -lIRT
DEPS += -I${EIC_SHELL_PREFIX}/include/IRT
DEPS += -Isrc

#--------------------------------------------

BIN_TARGET = bin
EXECUTABLES := $(addprefix $(BIN_TARGET)/, $(basename $(notdir $(wildcard src/*.cpp))))

EICRECON_DIR = ${DRICH_DEV}/EICrecon/src/services/geometry/rich
DEPS += -I$(EICRECON_DIR)

LIB_TARGET = lib
IRTGEO_LIB_NAME = IrtGeo
IRTGEO_LIB = $(LIB_TARGET)/lib$(IRTGEO_LIB_NAME).so
IRTGEO_ROOT = $(EICRECON_DIR)/richgeo
IRTGEO_SOURCES := $(wildcard $(IRTGEO_ROOT)/IrtGeo*.cc)
IRTGEO_HEADERS := $(wildcard $(IRTGEO_ROOT)/IrtGeo*.h) $(IRTGEO_ROOT)/RichGeo.h

#--------------------------------------------

all: $(IRTGEO_LIB) $(EXECUTABLES)

$(IRTGEO_LIB): $(IRTGEO_HEADERS) $(IRTGEO_SOURCES)
	mkdir -p $(LIB_TARGET)
	@echo "----- build $@ -----"
	$(CXX) $(IRTGEO_SOURCES) -shared -o $@ $(FLAGS) $(DEPS) $(LIBS)

$(BIN_TARGET)/%: src/%.cpp $(IRTGEO_LIB)
	mkdir -p $(BIN_TARGET)
	@echo "----- build $@.o -----"
	$(CXX) -c $< -o $@.o $(FLAGS) $(DEPS)
	@echo "--- make executable $@"
	$(CXX) -o $@ $@.o $(LIBS) -L$(LIB_TARGET) -l$(IRTGEO_LIB_NAME)
	$(RM) $@.o

clean:
	@echo "CLEAN ======================================================"
	$(RM) $(EXECUTABLES) $(IRTGEO_LIB)

