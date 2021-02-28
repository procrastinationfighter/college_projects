cmake_minimum_required(VERSION 3.15)
project(forests C)

set(CMAKE_C_STANDARD 11)

add_executable(forests main.c userInterface.c userInterface.h World.c World.h String.c String.h BeingTree.c BeingTree.h)