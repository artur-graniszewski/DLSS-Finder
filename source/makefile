# Makefile

# Directories
SRC_DIR = ..\source
RES_DIR = ..\resources
BUILD_DIR = ..\build

# Compiler
CC = cl
# Compiler flags
CFLAGS = /Ox /EHsc /MD /nologo /std:c++17 /c

# Linker
LD = link
# Linker flags
LDFLAGS = /nologo /LIBPATH:. /DEFAULTLIB:Shell32.lib User32.lib

# Resource compiler
RC = rc
# Resource flags
RCFLAGS = /fo

# Source files
SRC = $(SRC_DIR)\dlss-finder.cpp
# Object files
OBJ_FILE = dlss-finder.obj
OBJ = $(BUILD_DIR)\$(OBJ_FILE)

# Executable name
TARGET = $(BUILD_DIR)\dlss-finder.exe

# Icon file
ICON_FILE = $(RES_DIR)\FSR3.ico
RES_FILE = $(BUILD_DIR)\resources.res
RC_FILE = $(SRC_DIR)\resources.rc

# Build rule
all: $(TARGET)

$(RES_FILE): $(RC_FILE)
    $(RC) $(RCFLAGS)$(RES_FILE) $(RC_FILE)

.cpp.obj:
    $(CC) $(CFLAGS) /Fo$(OBJ) $(SRC)

$(TARGET): dlss-finder.obj $(RES_FILE)
    $(LD) $(LDFLAGS) /out:$(TARGET) /MACHINE:X64 $(OBJ) $(RES_FILE) $(DET_LIB_PATH)

clean:
    del /Q $(OBJ) $(TARGET) $(BUILD_DIR)\*.ilk $(BUILD_DIR)\*.pdb $(BUILD_DIR)\*.exp $(BUILD_DIR)\*.lib $(RES_FILE)