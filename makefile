CC = g++
LIBS = -lSDL2 -lGL -lm
FLAGS = -g -std=c++20
ASAN_FLAGS = -g -O0 -fsanitize=address -O1 -fno-omit-frame-pointer
SRCS = glp/external/glad.c \
       glp/utils.cc \
       glp/sdl.cc \
       glp/shader.cc \
       glp/model.cc \
       main.cc

NAME = game

all :
	$(CC) $(SRCS) $(FLAGS) $(INCLUDES) $(LIBS) -o $(NAME)

asan :
	$(CC) $(SRCS) $(ASAN_FLAGS) $(INCLUDES) $(LIBS) -o $(NAME)
