//
// Created by Kelly Akins on 11/13/26.
//

#include "FillResults.h"

FillResults::FillResults(const long orderId, const int requestQty, const int filledQty,
    const int remainingQty, const long fillPrice, long auxData, const std::string& message)
{
    OrderId = orderId;
    RequestedQuantity = requestQty;
    FilledQuantity = filledQty;
    RemainingQuantity = remainingQty;
    FillPrice = fillPrice;
    AuxData = auxData;
    Message = message;
}

bool FillResults::IsNull() const
{
    return OrderId == -1L && RequestedQuantity == -1L && RemainingQuantity == -1L && AuxData == LONG_MIN;
}
