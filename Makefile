# CSCI-4530/6530 Advanced Computer Graphics, Spring 2010
# Simple Makefile for Homework 3
# for g++ on unix or cygwin platforms

SHELL = /bin/bash

# ===============================================================

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
UNAME := LINUX
else 
ifeq ($(UNAME), FreeBSD)
UNAME := FREEBSD
endif
endif

ifeq ($(UNAME), LINUX)
# Linux
CC              = g++ -g -O3 -Wall -pedantic -D__LINUX__ -std=c++0x
INCLUDE_PATH    = -I/usr/X11R6/include -L/usr/local/include
LIB_PATH        = -L/usr/X11R6/lib -L/usr/local/lib
LIBS            = -lm -lGL -lGLU -lglut
else
ifeq ($(UNAME), FREEBSD)
# FreeBSD
CC              = g++ -g -O3 -Wall -pedantic -D__FREEBSD__
INCLUDE_PATH    = -I/usr/X11R6/include -L/usr/local/include
LIB_PATH        = -L/usr/X11R6/lib -L/usr/local/lib
LIBS            = -lm -lGL -lGLU -lglut
else
ifeq ($(UNAME), Darwin)
# Mac OS X
CC		= g++ -g -Wall -pedantic -O3 -fopenmp
LIBS		= -framework GLUT -framework OpenGL
else
ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)
# Cygwin with native Windows libraries
CC              = g++ -g -O3 -Wall -pedantic 
INCLUDE_PATH    = -I/usr/include/w32api -I/usr/include/opengl
LIB_PATH        = -L/usr/lib/w32api
LIBS            = -lm -lopengl32 -lglu32 -lglut32 
else
$(error "Unknown OS: " $(UNAME))
endif
endif
endif
endif

# ===============================================================

SRCS	= main.cpp matrix.cpp camera.cpp glCanvas.cpp mesh.cpp edge.cpp \
	  radiosity.cpp face.cpp raytree.cpp raytracer.cpp sphere.cpp \
	  cylinder_ring.cpp material.cpp image.cpp photon_mapping.cpp \
	  kdtree.cpp 
OBJS	= $(SRCS:.cpp=.o)
EXE	= render

# ===============================================================
# targets

.PHONY: all depend clean

all: depend $(EXE) 

depend:
	$(CC) $(INCLUDE_PATH) -E -M $(SRCS) > Makefile.depend

clean:
	rm -f *~ *bak *.o  $(EXE) $(EXE).exe Makefile.depend

# ===============================================================
# compilation rules

$(EXE): Makefile $(OBJS)
	$(CC) $(INCLUDE_PATH) -o $@ $(OBJS) $(LIB_PATH) $(LIBS) 

.cpp.o: Makefile
	$(CC) $(INCLUDE_PATH) $< -c -o $@

-include Makefile.depend
