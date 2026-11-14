//
// Created by Kelly Akins on 11/13/26.
//

#include "PriceLevel.h"

PriceLevel::PriceLevel(long price, long quantity, int orderCount, OrderSide side) {
    Price = price;
    QuantityAvailable = quantity;
    OrderCount = orderCount;
    Side = side;

    LastFillQuantity = 0L;
    LastFillTimestamp = 0L;
    NextHigherLevelIndex = -1;
    NextLowerLevelIndex = -1;
    OldestOrderIndex = -1;
    NewestOrderIndex = -1;
}

void PriceLevel::Clear() {
    Price = 0L;
    QuantityAvailable = 0L;
    OrderCount = 0;
    Side = OrderSide::BUY;
    LastFillQuantity = 0L;
    LastFillTimestamp = 0L;

    NextHigherLevelIndex = -1;
    NextLowerLevelIndex = -1;
    OldestOrderIndex = -1;
    NewestOrderIndex = -1;
}

bool PriceLevel::IsEmpty() const {
    return QuantityAvailable == 0L;
}