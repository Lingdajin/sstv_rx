
BUILD_DIR=.

PROJ_DIR=.

ROOT_DIR=..

EXT = .a
PROGRAM = psd$(EXT)

SOURCE_DIR=Sources

SOURCE_FILES= \
	psd_a2d.c \
	psd_d2a.c

OBJS=$(SOURCE_FILES:.c=.o)
LIBS=$(SOURCE_FILES:.c=.a)

ifeq "$(SBROOT)" ""
$(error "SBROOT environment variable is not defined")
endif

# Make sure SBROOT uses '/' path separator characters
SBROOT:=$(subst \,/,$(SBROOT))
SBPRE=$(SBROOT)/bin/sb
XCFLAGS=--asm --no-stdlib --no-debug 
LIBS=
CC=gcc
# SandBlaster DSP tools
SBCC="$(SBROOT)/bin/sbcc"
SBAR="$(SBROOT)/bin/sb-ar"
SBNM="$(SBROOT)/bin/sb-nm"
SBOD="$(SBROOT)/bin/sb-objdump"
SBCCFE="$(SBROOT)/lib/sbcc-lib/sbcc-fe"
SBMAP=$(SBPRE)odmap
AS="$(SBROOT)/bin/sb-as"
AR="$(SBROOT)/bin/sb-ar"
# CPPASM  = cpp -x assembler-with-cpp -traditional -nostdinc
MKBOOT  = "$(SBROOT)/bin/sbmkbootable"
MKSBM   = "$(SBROOT)/bin/srec2sbm"
# Common UNIX command line utilities
MKDIR="$(SBROOT)/msys/bin/mkdir"
RM="$(SBROOT)/msys/bin/rm"
CAT="$(SBROOT)/msys/bin/cat"

ARCH_FLAGS =--march=sb3500 -O4 --inline --restrict #--noclean
CFLAGS=

CINC= \
	-I$(PROJ_DIR)/Headers \
	-I$(ROOT_DIR)/Common_Headers 

#	-I$(SBROOT)/include \

all: $(OBJS) #psd_a2d_dll.la psd_d2a_dll.la
	@echo "'$@' complete"
	
#ar 无法编译成一个.a静态库，报错: two different operation options specified
#编译.la报找不到头文件的错(尽管已添加对应路径) -> 找到了也会报错

%.o: $(SOURCE_DIR)/%.c
	$(SBCC) -o $@ -c $(ARCH_FLAGS) $(CFLAGS) $(CINC) $^

psd_a2d_dll.la: $(SOURCE_DIR)/psd_a2d.c
	gcc -O3 -o $@ -shared $^ 

psd_d2a_dll.la: $(SOURCE_DIR)/psd_d2a.c
	gcc -O3 -o $@ -shared $^
	

clean: 
	-$(RM) -rf $(addprefix $(BUILD_DIR)/*.,la s v o a)
	@echo "'$@' complete"
	
