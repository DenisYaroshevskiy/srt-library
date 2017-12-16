clang++ -O1 -fsanitize=address -fno-omit-frame-pointer --std=c++14\
        -Werror -Wall -g $1
