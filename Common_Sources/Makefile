
BUILD_DIR=.

PROJ_DIR=..

HEAD_DIR_COMM=$(PROJ_DIR)/Common_Headers

INCLS=-I$(HEAD_DIR_COMM)

CSRCS = \
	function.c \
	shell.c	\
	threadutil.c

OBJS=$(CSRCS:.c=.o)

CC="$(SBROOT)/bin/sbcc"
AS="$(SBROOT)/bin/sbcc"
RM="$(SBROOT)/msys/bin/rm"

ARCH_FLAGS =--march=sb3500

CFLAGS= $(INCLS) \
	$(ARCH_FLAGS) \
	$(BOARD_FLAGS) \
	-O3  \
	--inline \
	--restrict \
#	--noclean 

all: $(OBJS)
	@echo "'$@' complete"

#无法编译成静态库，报错同PSD

%.o:	$(SRC_DIR_RF)/%.c
	$(CC) -o $@ -c $(CFLAGS) $^

clean:
	-$(RM) -rf $(addprefix $(BUILD_DIR)/*.,la s v o a)
	@echo "'$@' complete"
