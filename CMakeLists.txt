cmake_minimum_required(VERSION 3.5)
project(editor)

set(SOURCE
  ./src/main.c
  ./src/service.c
  ./src/render.c
  ./src/terminal.c
  ./src/buffer.c
)

add_executable(editor ${SOURCE})
