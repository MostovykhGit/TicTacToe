cmake_minimum_required(VERSION 3.10)
project(TicTacToeMultiplayer)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Поиск библиотеки Boost с компонентом asio и system
find_package(Boost REQUIRED COMPONENTS system)

# Проверка, что Boost был найден
if (NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found!")
endif()

# Включаем директорию Boost
include_directories(${Boost_INCLUDE_DIRS})

# Добавляем исходники для сервера и клиента
add_executable(server server.cpp)
add_executable(client client.cpp)

# Линкуем Boost::system к обеим целям
target_link_libraries(server ${Boost_LIBRARIES})
target_link_libraries(client ${Boost_LIBRARIES})
