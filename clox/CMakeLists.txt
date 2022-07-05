cmake_minimum_required(VERSION 3.22)
project(clox C)

set(CMAKE_C_STANDARD 99)

add_executable(
        clox
        main.c
        common.h
        chunk.c chunk.h
        memory.c memory.h
        debug.c debug.h
        value.c value.h
        vm.c vm.h
        compiler.c compiler.h
        scanner.c scanner.h
)
