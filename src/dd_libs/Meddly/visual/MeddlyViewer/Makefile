
# $Id$

#
# Right now - proof of concept makefile
#

all: visual.jar

SRC= \
      info/LeafInfo.java      \
      info/ForestInfo.java    \
      logging/Logger.java     \
      logic/ForestInfoParser.java     \
      main/ApplicationExecution.java



OBJ= $(SRC:%.java=%.class)
BINOBJ= $(SRC:%.java=bin/%.class)

visual.jar: $(BINOBJ)
	cd bin; jar cf ../visual.jar $(OBJ)

#
# Generic rule
#

bin/%.class: src/%.java
	javac -classpath bin/ -d bin/ $<


#
# Dependencies
#

bin/logic/ForestInforParser.class:  bin/info/LeafInfo.class \
                                    bin/info/ForestInfo.class

bin/main/ApplicationExecution.class:  bin/info/LeafInfo.class \
                                      bin/logic/ForestInforParser.class
