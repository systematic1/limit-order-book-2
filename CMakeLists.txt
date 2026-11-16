cmake_minimum_required(VERSION 4.3)
project(LimitOrderBook2)

set(CMAKE_CXX_STANDARD 23)

add_executable(LimitOrderBook2 main.cpp
        models/FillResults.cpp
        models/FillResults.h
        models/Order.cpp
        models/Order.h
        models/PriceLevel.cpp
        models/PriceLevel.h
        models/Enums.h
        services/OrderBookEngine.cpp
        services/OrderBookEngine.h
        services/OrderCollection.cpp
        services/OrderCollection.h
        services/PriceLevelCollection.cpp
        services/PriceLevelCollection.h
        Program.cpp
        Program.h)
