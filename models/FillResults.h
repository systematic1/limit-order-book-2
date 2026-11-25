//
// Created by Kelly Akins on 11/13/26.
//

#ifndef LIMITORDERBOOK2_FILLRESULTS_H
#define LIMITORDERBOOK2_FILLRESULTS_H

#include <string>

#include "Enums.h"

struct FillResults
{
public:
    long OrderId;
    OrderSide Side;
    int RequestedQuantity;
    int FilledQuantity;
    int RemainingQuantity;
    long FillPrice;
    long AuxData;
    long AccountId;
    std::string Message;
    OrderStatus Status;

    FillResults() = default;
    FillResults(long orderId, OrderSide side, int requestQty, int filledQty, int remainingQty, long fillPrice,
            long auxData, const std::string& message, OrderStatus status, long accountId);
};

#endif //LIMITORDERBOOK2_FILLRESULTS_H
