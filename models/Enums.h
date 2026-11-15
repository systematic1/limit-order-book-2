//
// Created by Kelly Akins on 11/13/26.
//

#ifndef LIMITORDERBOOK2_ENUMS_H
#define LIMITORDERBOOK2_ENUMS_H

enum OrderSide
{
    BUY,
    SELL
};

enum OrderType
{
    LIMIT,
    MARKET
};

enum OrderStatus
{
    UNKNOWN,
    FILLED,
    PARTIALLY_FILLED,
    CANCELLED,
    DUPLICATE,
    NOT_FOUND,
    ACCEPTED_AND_WAITING,
    REJECTED
};

enum TimeToLive
{
    GTC
};

#endif //LIMITORDERBOOK2_ENUMS_H
