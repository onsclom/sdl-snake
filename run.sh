rm a.out
clang-format -i snake.c
cc snake.c -ansi -lSDL2 && ./a.out
