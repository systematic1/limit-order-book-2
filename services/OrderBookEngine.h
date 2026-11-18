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
    OrderBookEngine();
    OrderBookEngine(int orderCapacity, int priceLevelCapacity, std::function<void(long, long, OrderStatus, const std::string&)>);
    OrderStatus AddOrder(Order& order);
    OrderStatus CancelOrder(long orderId);
    void NotifyOrderFillStatus(const FillResults& fillResults) const;
    std::function<void(long, long, OrderStatus, const std::string&)> NotifyFunction;

    const OrderCollection& GetOrderCollection() const;
    const PriceLevelCollection& GetAskPriceLevelCollection() const;
    const PriceLevelCollection& GetBidPriceLevelCollection() const;

private:
    OrderStatus ValidateOrder(const Order& order) const;
    bool DoesOrderViolateRisk(const Order& order) const;
    int FillMatchingOrder(PriceLevel& priceLevel, Order& order);
    bool AddOrderToBook(Order& order);
    bool IsDuplicateOrder(long orderId) const;
    void SetStatusMessage(FillResults& fillResults);
};

#endif //LIMITORDERBOOK2_ORDERBOOKENGINE_H
