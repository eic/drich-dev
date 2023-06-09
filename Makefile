CXX = g++
FLAGS = -g -Wno-deprecated -fPIC -m64 -fno-inline -Wno-write-strings

# ROOT
DEPS += $(shell root-config --cflags)
LIBS += $(shell root-config --glibs)

# local libs
LIBS += -L${EIC_SHELL_PREFIX}/lib
LIBS += -lIRT -ledm4eic
DEPS += -I${EIC_SHELL_PREFIX}/include
DEPS += -I${EIC_SHELL_PREFIX}/include/IRT
DEPS += -Isrc

# image libs
LIBS += -L/usr/local/lib
LIBS += -lDDCore -lDDRec
LIBS += -lpodio -lpodioRootIO -ledm4hep
LIBS += -lfmt -lspdlog
LIBS += -lG4global -lG4materials -lG4geometry -lG4persistency
DEPS += -I/usr/local/include
FLAGS += -DSPDLOG_FMT_EXTERNAL

#--------------------------------------------

BIN_TARGET = bin
SOURCES := $(wildcard src/*.cpp)
EXECUTABLES := $(addprefix $(BIN_TARGET)/, $(basename $(notdir $(SOURCES))))

EICRECON_DIR = ${DRICH_DEV}/EICrecon/src/services/geometry/richgeo
DEPS += -I$(EICRECON_DIR)

LIB_TARGET = lib
IRTGEO_LIB_NAME = IrtGeo
IRTGEO_LIB = $(LIB_TARGET)/lib$(IRTGEO_LIB_NAME).so
IRTGEO_ROOT = $(EICRECON_DIR)
IRTGEO_SOURCES := $(wildcard $(IRTGEO_ROOT)/IrtGeo*.cc)
IRTGEO_HEADERS := $(wildcard $(IRTGEO_ROOT)/IrtGeo*.h) $(IRTGEO_ROOT)/RichGeo.h

IRT_AUXFILE_SOURCE = src/create_irt_auxfile.cpp
IRT_AUXFILE_EXECUTABLE = $(BIN_TARGET)/create_irt_auxfile
SOURCES := $(filter-out $(IRT_AUXFILE_SOURCE), $(SOURCES))
EXECUTABLES := $(filter-out $(IRT_AUXFILE_EXECUTABLE), $(EXECUTABLES))

#--------------------------------------------

all: $(EXECUTABLES) $(IRTGEO_LIB) $(IRT_AUXFILE_EXECUTABLE)

$(EXECUTABLES): $(BIN_TARGET)/%: src/%.cpp
	mkdir -p $(BIN_TARGET)
	@echo "----- build $@.o -----"
	$(CXX) -c $< -o $@.o $(FLAGS) $(DEPS)
	@echo "--- make executable $@"
	$(CXX) -o $@ $@.o $(LIBS)
	$(RM) $@.o

$(IRTGEO_LIB): $(IRTGEO_HEADERS) $(IRTGEO_SOURCES)
ifeq ($(IRT_ROOT_DICT_FOUND),1)
	mkdir -p $(LIB_TARGET)
	@echo "----- build $@ -----"
	$(CXX) $(IRTGEO_SOURCES) -shared -o $@ $(FLAGS) $(DEPS) $(LIBS)
else
	@echo "WARNING: skip building $@ since IRT ROOT dict not found"
endif

$(IRT_AUXFILE_EXECUTABLE): $(IRT_AUXFILE_SOURCE) $(IRTGEO_LIB)
ifeq ($(IRT_ROOT_DICT_FOUND),1)
	mkdir -p $(BIN_TARGET)
	@echo "----- build $@.o -----"
	$(CXX) -c $< -o $@.o $(FLAGS) $(DEPS)
	@echo "--- make executable $@"
	$(CXX) -o $@ $@.o $(LIBS) -L$(LIB_TARGET) -l$(IRTGEO_LIB_NAME)
	$(RM) $@.o
else
	@echo "WARNING: skip building $@ since IRT ROOT dict not found"
endif

clean:
	@echo "CLEAN ======================================================"
	$(RM) $(EXECUTABLES) $(IRT_AUXFILE_EXECUTABLE) $(IRTGEO_LIB)

