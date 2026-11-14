//
// Created by Kelly on 11/13/26.
//

#ifndef LIMITORDERBOOK2_ORDERCOLLECTION_H
#define LIMITORDERBOOK2_ORDERCOLLECTION_H

#include "../models/Order.h"
#include "../models/PriceLevel.h"
#include "PriceLevelCollection.h"

class OrderCollection {
private:
    int Capacity;
    int CapacityMask;
    Order* Orders;
    bool* Available;
    int CountInUse;
    int LastIndex;

    Order NullOrder = Order(-1, -1L, OrderSide::BUY, OrderType::MARKET, 0,
        0, -1, 0L, TimeToLive::GTC, -1L, -1L);

public:
    OrderCollection(int capacityPowerOf2);
    ~OrderCollection();

    int FindOrderIndex(long orderId) const;
    Order& GetOrder(long orderId);
    int AddOrder(const Order& order, PriceLevel& priceLevel);
    bool RemoveOrder(const Order& order, PriceLevel& priceLevel);
    Order& FindBestAtPriceLevel(PriceLevel& nextLevel, const PriceLevelCollection& levels,
        OrderSide matchSide);
};

#endif //LIMITORDERBOOK2_ORDERCOLLECTION_H
