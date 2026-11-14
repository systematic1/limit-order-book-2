//
// Created by Kelly Akins on 11/13/26.
//

#include "Order.h"

Order::Order(long orderId, long price, OrderSide side, OrderType orderType, int totalQuantity,
    int remainingQuantity, int priceLevelIndex, long timestamp, TimeToLive timeToLive,
    long accountId, long securityId)
{
    OrderId = orderId;
    Price = price;
    Side = side;
    OrderType = orderType;
    TotalQuantity = totalQuantity;
    RemainingQuantity = remainingQuantity;
    PriceLevelIndex = priceLevelIndex;
    TimeToLive = timeToLive;
    AccountId = accountId;
    SecurityId = securityId;

    NextOlderOrderIndexAtLevel = -1;
    NextNewerOrderIndexAtLevel = -1;
}

void Order::Clear() {
    OrderId = 0L;
    Price = 0L;
    Side = OrderSide::BUY;
    OrderType = OrderType::LIMIT;
    TotalQuantity = 0L;
    RemainingQuantity = 0L;
    PriceLevelIndex = 0L;
    TimeToLive = TimeToLive::GTC;
    AccountId = 0L;
    SecurityId = 0L;

    NextOlderOrderIndexAtLevel = -1;
    NextNewerOrderIndexAtLevel = -1;
}
