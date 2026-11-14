//
// Created by Kelly Akins on 11/13/26.
//

#include "Order.h"

Order::Order(const long orderId, const long price, const OrderSide side, OrderType orderType,
    const int totalQuantity, const int remainingQuantity, const int priceLevelIndex, const long timestamp,
    TimeToLive timeToLive, const long accountId, const long securityId)
{
    OrderId = orderId;
    Price = price;
    Side = side;
    Type = orderType;
    TotalQuantity = totalQuantity;
    RemainingQuantity = remainingQuantity;
    PriceLevelIndex = priceLevelIndex;
    TTL = timeToLive;
    AccountId = accountId;
    SecurityId = securityId;

    NextOlderOrderIndexAtLevel = -1;
    NextNewerOrderIndexAtLevel = -1;
}

void Order::Clear() {
    OrderId = 0L;
    Price = 0L;
    Side = OrderSide::BUY;
    Type = OrderType::LIMIT;
    TotalQuantity = 0L;
    RemainingQuantity = 0L;
    PriceLevelIndex = 0L;
    TTL = TimeToLive::GTC;
    AccountId = 0L;
    SecurityId = 0L;

    NextOlderOrderIndexAtLevel = -1;
    NextNewerOrderIndexAtLevel = -1;
}
