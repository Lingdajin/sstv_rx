#/*HEADER*******************************************************************
#***************************************************************************
#***
#***     Copyright (c) 2002 Sandbridge Technologies, Inc.
#***
#***                     All rights reserved
#***
#*** This software embodies materials and concepts which are confidential
#*** to Sandbridge Technologies, Inc. and is made available
#*** solely pursuant to the terms of a written license agreement with
#*** Sandbridge Technologies, Inc.
#***************************************************************************
#*END*************************************************************************/

# To activate files I/O for validation of UL vs. MatLab must '#define UL_VALIDATION'

BUILD_DIR = .

PROJ_DIR = ..

SRC_DIR_RF = $(BUILD_DIR)/Sources
HEAD_DIR_RF = $(BUILD_DIR)/Headers
HEAD_DIR_COMM = $(PROJ_DIR)/Common_Headers
HEAD_DIR_PSD = $(PROJ_DIR)/PSD/Headers

INCLS = -I$(HEAD_DIR_RF) \
		-I$(HEAD_DIR_COMM) \
		-I$(HEAD_DIR_PSD)

RFSRCS = \
	iris404.c \
	rf_init.c

CSRCS = $(RFSRCS)

RM	= rm -f
EXT	= .a
PROGRAM	= rf${EXT}

OBJS 	= $(CSRCS:.c=.o)

CC	= "$(SBROOT)/bin/sbcc"
AS	= "$(SBROOT)/bin/sbcc"
AR	= "$(SBROOT)/bin/sb-ar"

ARCH_FLAGS =--march=sb3500

CFLAGS= $(INCLS) \
	$(ARCH_FLAGS) \
	$(BOARD_FLAGS) \
	-O3  \
	--inline \
	--restrict \
#	--noclean 

default:	$(PROGRAM)

all:	$(PROGRAM)

$(PROGRAM): $(OBJS)
	$(AR) rf $@ $^

%.o:	$(SRC_DIR_RF)/%.c
	$(CC) -o $@ -c $(CFLAGS) $^

clean:
	$(RM) $(OBJS) $(PROGRAM) *.stackdump *~
	$(RM) *.s *.v *.o *.sbx *.nm *.dis
	$(RM) filteredOut.txt iqout.txt testout.txt

clobber: clean
	$(RM) profile.*
