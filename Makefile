CXX 		= g++

CXXFLAGS 		= -O0 -g ${DEFINES} 
DEFINES 	= -DGCW -DBETTER_ACCURACY
LDFLAGS 	=  -lSDL 
OUT	 	= melonds.elf

SRC 		= 	src/libui_sdl/main.cpp \
	src/libui_sdl/Platform.cpp \
	src/ARM.cpp \
	src/ARMInterpreter.cpp \
	src/ARMInterpreter_ALU.cpp \
	src/ARMInterpreter_Branch.cpp \
	src/ARMInterpreter_LoadStore.cpp \
	src/Config.cpp \
	src/CP15.cpp \
	src/DMA.cpp \
	src/GPU.cpp \
	src/GPU2D.cpp \
	src/GPU3D.cpp \
	src/GPU3D_Soft.cpp \
	src/NDS.cpp \
	src/NDSCart.cpp \
	src/RTC.cpp \
	src/SPI.cpp \
	src/SPU.cpp \
	src/Wifi.cpp 
SOURCES 	= ${SRC}

OBJS 		= ${SOURCES:.cpp=.o}

all		: ${OUT} 

${OUT}	: ${OBJS}
		${CXX} -o ${OUT} ${SOURCES} ${CXXFLAGS} ${LDFLAGS}
	
clean	:
		rm ${OBJS}
