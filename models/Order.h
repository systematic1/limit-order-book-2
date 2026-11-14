//
// Created by Kelly Akins on 11/13/26.
//

#ifndef LIMITORDERBOOK2_ORDER_H
#define LIMITORDERBOOK2_ORDER_H


struct Order {
public:
    long OrderId;
    long Price;
    OrderSide Side;
    OrderType OrderType;
    int TotalQuantity;
    int RemainingQuantity;
    int PriceLevelIndex;
    long Timestamp;
    TimeToLive TimeToLive;
    long AccountId;
    long SecurityId;

    int NextOlderOrderIndexAtLevel {-1};
    int NextNewerOrderIndexAtLevel {-1};

    Order() = default;
    Order(const Order& order) = default;
    Order(long orderId, long price, OrderSide side, OrderType type, int totalQuantity,
        int remainingQuantity, int priceLevelIndex, long timestamp, TimeToLive timeToLive,
        long accountId, long securityId);
    void Clear();
};


#endif //LIMITORDERBOOK2_ORDER_H
