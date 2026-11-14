//
// Created by Kelly Akins on 11/13/26.
//

#ifndef LIMITORDERBOOK2_FILLRESULTS_H
#define LIMITORDERBOOK2_FILLRESULTS_H
#include <string>

struct FillResults {
public:
    long OrderId;
    int RequestedQuantity;
    int FilledQuantity;
    int RemainingQuantity;
    long FillPrice;
    long AuxData;
    std::string Message;

    FillResults() = default;
    FillResults(long orderId, int requestQty, int filledQty, int remainingQty,
            long fillPrice, long auxData, const std::string& message);
};

#endif //LIMITORDERBOOK2_FILLRESULTS_H
