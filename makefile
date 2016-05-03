
SHELL = C:/Windows/system32/cmd.exe
SOURCEDIR = ./
HEADERS = $(wildcard $(SOURCEDIR)/*.h)
SOURCES = $(wildcard $(SOURCEDIR)/*.cpp)

MSB_FLAGS = /nologo /m:3 /property:Configuration=Debug;Platform=Win32

.PHONY: clean vc

all: bin/TestWare.exe
	

bin/TestWare.exe: Testware.vcxproj $(SOURCES) $(HEADERS)
	MSBuild $< $(MSB_FLAGS) /property:OutDir=".\bin\\"

clean:
	MSBuild Testware.vcxproj $(MSB_FLAGS) /target:Clean

