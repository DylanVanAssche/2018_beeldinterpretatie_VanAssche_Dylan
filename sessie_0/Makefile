CC = g++  
CFLAGS = -g -Wall
SRCS = main.cpp
PROG = sessie0 

OPENCV = `pkg-config opencv --libs`
LIBS = $(OPENCV)

$(PROG):$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)
