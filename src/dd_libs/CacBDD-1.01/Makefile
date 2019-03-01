.SUFFIXES: .o .cc .u

WHERE	= .
CPP	= g++
CFLAGS = -g -O3 -Wno-deprecated

INCLUDE = $(WHERE)/include

####################################################
File1	= ./main
File2	= ./UTable
File3	= ./Manager
File4	= ./CTable
File5	= ./BDDNode
File6   = ./DdNode

SRC1	= $(File1).cpp
SRC2	= $(File2).cpp
SRC3	= $(File3).cpp
SRC4	= $(File4).cpp
SRC5	= $(File5).cpp
SRC6	= $(File6).cpp

OBJ1	= $(File1).o
OBJ2	= $(File2).o
OBJ3	= $(File3).o
OBJ4	= $(File4).o
OBJ5	= $(File5).o
OBJ6    = $(File6).o


OBJS	= $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6)

TARGET	= t
#------------------------------------------------------
$(TARGET): $(OBJS)
	$(CPP) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS) -lm

$(OBJ1): $(SRC1)
	$(CPP) -c $(SRC1) -I$(INCLUDE) $(CFLAGS)
$(OBJ2): $(SRC2)
	$(CPP) -c $(SRC2) -I$(INCLUDE) $(CFLAGS)
$(OBJ3): $(SRC3)
	$(CPP) -c $(SRC3) -I$(INCLUDE) $(CFLAGS)
$(OBJ4): $(SRC4)
	$(CPP) -c $(SRC4) -I$(INCLUDE) $(CFLAGS)
$(OBJ5): $(SRC5)
	$(CPP) -c $(SRC5) -I$(INCLUDE) $(CFLAGS)
$(OBJ6): $(SRC6)
	$(CPP) -c $(SRC6) -I$(INCLUDE) $(CFLAGS)


#------------------------------------------------------
cl: #clean
	rm -f *.o *.u mon.out gmon.out *.pixie *.Addrs *.Counts mnem.* \
	.pure core *.warnings *.cpp~ *.h~ Makefile~

cla: #clean all
	rm -f $(TARGET) *.o *.cpp~ *.h~ Makefile~
#------------------------------------------------------
