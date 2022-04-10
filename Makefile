CC=gcc
COMPILEFLAGS=-Wall -Wextra 
SOURCE=./source/*.c
INCLUDE=-I./include
APP=./app
ZIPFILES=./source/* ./include/* ./Makefile
ZIPNAME=./trabalho-3.zip
VALGRINDFLAGS=--leak-check=full --show-leak-kinds=all --track-origins=yes

all:
	$(CC) $(SOURCE) $(INCLUDE) -o $(APP) $(COMPILEFLAGS)

run:
	$(APP)

zip:
	zip $(ZIPNAME) $(ZIPFILES)


clean:
	rm -f $(APP) $(ZIPNAME) ./veiculo*.bin ./linha*.bin ./indice*.bin


valgrind:
	valgrind $(VALGRINDFLAGS) $(APP)
