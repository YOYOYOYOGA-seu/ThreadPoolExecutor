################ gcc config ######################
CROSS_COMPILE ?= 

INC_DIR := ./ \
						./inc

SRC_DIR := ./ \
					 ./src 

LIB_DIR := ./lib

OBJ_DIR := ./obj

TARGET_NAME := test

LD_FLAGS += 

CC_FLAGS += 

PP_FLAGS += 

LIBS := -lpthread -lstdc++ 
################# format handler #######################
CC := $(CROSS_COMPILE)gcc
PP := $(CROSS_COMPILE)g++
LD := $(CROSS_COMPILE)g++

INCPATH := $(patsubst %, -I %, $(INC_DIR))
SRCPATH := $(SRC_DIR)
OBJPATH := $(OBJ_DIR)
LIBPATH := $(LIB_DIR)

C_FILES := $(notdir $(foreach dir, $(SRCPATH), $(wildcard $(dir)/*.c)))
CPP_FILES := $(notdir $(foreach dir, $(SRCPATH), $(wildcard $(dir)/*.cpp)))

C_OBJS := $(patsubst %.c, $(OBJPATH)/%.o, $(C_FILES))
CPP_OBJS := $(patsubst %.cpp, $(OBJPATH)/%.o, $(CPP_FILES))

OBJS := $(C_OBJS) $(CPP_OBJS)

DEPS := $(OBJS:.o=.d)

TARGET := $(TARGET_NAME)

VPATH := $(SRCPATH)

################# color define #########################
DEFAULT_COLOR = "\e[0m"
BLACK = "\e[30;1m"
RED  =  "\e[31;1m"
GREEN = "\e[32;1m"
YELLOW = "\e[33;1m"
BLUE  = "\e[34;1m"
PURPLE = "\e[35;1m"
CYAN  = "\e[36;1m"
WHITE = "\e[37;1m"

################# rules #########################
default : $(TARGET)

$(TARGET) : $(OBJS)
	@echo $(PURPLE)"\tLD\t"$(DEFAULT_COLOR)"$(TARGET).elf"
	@$(LD)  $(LD_FLAGS) -o $(TARGET).elf $^ -L $(LIBPATH) $(LIBS)

-include $(DEPS) 

$(C_OBJS) : $(OBJPATH)/%.o : %.c
	@echo $(PURPLE)"\tCC\t"$(DEFAULT_COLOR)"$@"
	@$(CC)  $(INCPATH)  -MM -MT $@ -MF $(patsubst %.o, %.d, $@) $<
	$(CC) $(CC_FLAGS)  $(INCPATH) -c -o $@ $< 

$(CPP_OBJS) : $(OBJPATH)/%.o : %.cpp
	@echo $(PURPLE)"\tPP\t"$(DEFAULT_COLOR)"$@"
	@$(PP)  $(INCPATH)  -MM -MT $@ -MF $(patsubst %.o, %.d, $@) $<
	@$(PP) $(PP_FLAGS) $(INCPATH) -c -o  $@ $< 

.PHNOY:
clean:
	rm -f *.bin *.elf
	rm -f $(OBJPATH)/*.o $(OBJPATH)/*.d