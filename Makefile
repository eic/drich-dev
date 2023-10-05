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
RICHGEO_LIB_NAME = RichGeo
RICHGEO_LIB = $(LIB_TARGET)/lib$(RICHGEO_LIB_NAME).so
RICHGEO_ROOT = $(EICRECON_DIR)
RICHGEO_SOURCES := $(wildcard $(RICHGEO_ROOT)/IrtGeo*.cc) $(RICHGEO_ROOT)/ReadoutGeo.cc
RICHGEO_HEADERS := $(wildcard $(RICHGEO_ROOT)/IrtGeo*.h)  $(RICHGEO_ROOT)/ReadoutGeo.h $(RICHGEO_ROOT)/RichGeo.h

PIXEL_GAP_SOURCE = src/test_pixel_gap_cuts.cpp
PIXEL_GAP_EXECUTABLE = $(BIN_TARGET)/test_pixel_gap_cuts
SOURCES := $(filter-out $(PIXEL_GAP_SOURCE), $(SOURCES))
EXECUTABLES := $(filter-out $(PIXEL_GAP_EXECUTABLE), $(EXECUTABLES))

#--------------------------------------------

all: $(EXECUTABLES) $(RICHGEO_LIB) $(PIXEL_GAP_EXECUTABLE)

$(EXECUTABLES): $(BIN_TARGET)/%: src/%.cpp
	mkdir -p $(BIN_TARGET)
	@echo "----- build $@.o -----"
	$(CXX) -c $< -o $@.o $(FLAGS) $(DEPS)
	@echo "--- make executable $@"
	$(CXX) -o $@ $@.o $(LIBS)
	$(RM) $@.o

$(RICHGEO_LIB): $(RICHGEO_HEADERS) $(RICHGEO_SOURCES)
ifeq ($(IRT_ROOT_DICT_FOUND),1)
	mkdir -p $(LIB_TARGET)
	@echo "----- build $@ -----"
	$(CXX) $(RICHGEO_SOURCES) -shared -o $@ $(FLAGS) $(DEPS) $(LIBS)
else
	@echo "WARNING: skip building $@ since IRT ROOT dict not found"
endif

$(PIXEL_GAP_EXECUTABLE): $(PIXEL_GAP_SOURCE) $(RICHGEO_LIB)
	@echo "WARNING: skip building $@ since broken"
# ifeq ($(IRT_ROOT_DICT_FOUND),1)
# 	mkdir -p $(BIN_TARGET)
# 	@echo "----- build $@.o -----"
# 	$(CXX) -c $< -o $@.o $(FLAGS) $(DEPS)
# 	@echo "--- make executable $@"
# 	$(CXX) -o $@ $@.o $(LIBS) -L$(LIB_TARGET) -l$(RICHGEO_LIB_NAME)
# 	$(RM) $@.o
# else
# 	@echo "WARNING: skip building $@ since IRT ROOT dict not found"
# endif

clean:
	@echo "CLEAN ======================================================"
	$(RM) $(EXECUTABLES) $(PIXEL_GAP_EXECUTABLE) $(RICHGEO_LIB)
