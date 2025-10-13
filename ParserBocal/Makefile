# Makefile for ParserBocal programs

# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra

# Directories
DEST_DIR = ../appliAS/stuffs

# Targets
all: process_JB process_JV process_JC

# Process JB program (Bank Journal)
process_JB:
	$(CC) $(CFLAGS) process_JB/main.c process_JB/process.c -o process_JB/process_JB
	cp process_JB/process_JB $(DEST_DIR)/

# Process JV program (Sales Journal)
process_JV:
	$(CC) $(CFLAGS) process_JV/process.c -o process_JV/process_JV -lm
	cp process_JV/process_JV $(DEST_DIR)/

# Process JC program (Cash Journal)
process_JC:
	$(CC) $(CFLAGS) process_JC/Journal_Caisse.c -o process_JC/process_JC -lm
	cp process_JC/process_JC $(DEST_DIR)/

clean:
	rm -f process_JB/process_JB
	rm -f process_JV/process_JV
	rm -f process_JC/Journal_Caisse

fclean: clean
	
re: fclean all

.PHONY: all clean fclean re process_JB process_JV process_JC
