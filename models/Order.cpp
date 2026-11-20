//
// Created by Kelly Akins on 11/13/26.
//

#include "Order.h"
#include <climits>

Order::Order(const long orderId, const long price, const OrderSide side, OrderType orderType,
             const int totalQuantity, const int remainingQuantity, const int priceLevelIndex, const long timestamp,
             const TimeToLive timeToLive, const long accountId, const long securityId)
{
    OrderId = orderId;
    Price = price;
    Side = side;
    Type = orderType;
    TotalQuantity = totalQuantity;
    RemainingQuantity = remainingQuantity;
    PriceLevelIndex = priceLevelIndex;
    TTL = timeToLive;
    Timestamp = timestamp;
    AccountId = accountId;
    SecurityId = securityId;

    NextOlderOrderIndexAtLevel = -1;
    NextNewerOrderIndexAtLevel = -1;
}

void Order::Clear()
{
    OrderId = 0L;
    Price = 0L;
    Side = OrderSide::BUY;
    Type = OrderType::LIMIT;
    TotalQuantity = 0L;
    RemainingQuantity = 0L;
    PriceLevelIndex = 0;
    TTL = TimeToLive::GTC;
    Timestamp = -1L;
    AccountId = 0L;
    SecurityId = 0L;

    NextOlderOrderIndexAtLevel = -1;
    NextNewerOrderIndexAtLevel = -1;
}

bool Order::IsNull() const
{
    return OrderId < 0L && Price < 0L && TotalQuantity < 0L && AccountId == LONG_MIN;
}
