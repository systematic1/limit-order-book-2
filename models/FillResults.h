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
    int RequestedQuantity;
    int FilledQuantity;
    int RemainingQuantity;
    long FillPrice;
    long AuxData;
    std::string Message;
    OrderStatus Status;

    FillResults() = default;
    FillResults(long orderId, int requestQty, int filledQty, int remainingQty,
            long fillPrice, long auxData, const std::string& message, OrderStatus status);
    bool IsNull() const;
};

const FillResults NullFillResults = FillResults(-1L, -1, -1, -1,
    -1L, LONG_MIN, std::string(), OrderStatus::UNKNOWN);

#endif //LIMITORDERBOOK2_FILLRESULTS_H
