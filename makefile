GCC := gcc
LIB_NAME := driver_000relativty000

IMP_LIB := ./libraries/x64/HvrAPI.lib

DIR_SRC := ./source
DIR_INC := ./include
DIR_OBJ := ./object
DIR_DLL := ./build

SHARE_LIB   := $(LIB_NAME).dll
CFLAGS  := -I${DIR_INC}/include -Wl,--add-stdcall-alias -shared


SRC := $(${DIR_SRC}/*.cpp)
OBJ := $(${DIR_OBJ}/%.o, $(notdir ${SRC}))

all:$(OBJ)
    $(GCC) -o $(DIR_LIB)/$(SHARE_LIB) $(SRC) $(CFLAGS) -l$(IMP_LIB)
clean:
    rm -rf $(DIR_OBJ)/*.o $(DIR_LIB)/*