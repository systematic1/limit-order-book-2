//
// Created by Kelly Akins on 11/13/26.
//

#include "FillResults.h"

#include "Enums.h"

FillResults::FillResults(const long orderId, const OrderSide side, const int requestQty, const int filledQty,
                         const int remainingQty, const long fillPrice, const long auxData,
                         const std::string& message, const OrderStatus orderStatus, const long accountId)
{
    OrderId = orderId;
    Side = side;
    RequestedQuantity = requestQty;
    FilledQuantity = filledQty;
    RemainingQuantity = remainingQty;
    FillPrice = fillPrice;
    AuxData = auxData;
    Message = message;
    Status = orderStatus;
    AccountId = accountId;
}
