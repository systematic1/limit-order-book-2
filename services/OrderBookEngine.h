//
// Created by Kelly on 11/13/26.
//

#ifndef LIMITORDERBOOK2_ORDERBOOKENGINE_H
#define LIMITORDERBOOK2_ORDERBOOKENGINE_H

#include "../models/Enums.h"
#include "../models/Order.h"
#include "../models/PriceLevel.h"
#include "../models/FillResults.h"
#include "OrderCollection.h"

class OrderBookEngine
{
private:
    OrderCollection Orders;
    PriceLevelCollection AskPriceLevels;
    PriceLevelCollection BidPriceLevels;

public:
    OrderBookEngine(int orderCapacity, int priceLevelCapacity);
    OrderStatus AddOrder(Order& order);
    OrderStatus CancelOrder(const Order& order);

private:
    void NotifyOrderFillStatus(const Order& order, OrderStatus status, const FillResults& fillResults) const;
    OrderStatus ValidateOrder(const Order& order) const;
    bool DoesOrderViolateRisk(const Order& order) const;
    int FillMatchingOrder(PriceLevel& priceLevel, Order& order);
    bool AddOrderToBook(Order& order);
    bool IsDuplicateOrder(long orderId) const;
};


#endif //LIMITORDERBOOK2_ORDERBOOKENGINE_H
