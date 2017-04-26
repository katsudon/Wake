# $Id 
CC = g++

RM = rm
EXE = wake
PROF = -ggdb
# -DIMC -DIMCROM

C_FLAGS = $(PROF) -w
L_FLAGS = $(PROF) -lz -lcrypt -laspell -lmysqlpp -lmysqlclient

# Source Files

SRC_FILES := $(wildcard *.c)

#IMC = 0

# Object Files
OBJ_DIR = obj
OBJ_FILES := $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

wake: $(OBJ_FILES)
	$(RM) -f $(EXE)
	$(CC) $(L_FLAGS) -o $(EXE) $(OBJ_FILES)
		
$(OBJ_DIR)/%.o: %.c
	$(CC) $(C_FLAGS) -c -o $@ $<

clean:
	$(RM) -f $(OBJ_FILES) $(EXE) *~ *.bak *.orig *.rej *.d                                 
