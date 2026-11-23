//
// Created by Kelly Akins on 11/16/26.
//

#ifndef LIMITORDERBOOK2_PROGRAM_H
#define LIMITORDERBOOK2_PROGRAM_H

#include <random>
#include <string>

#include "models/Enums.h"
#include "models/Order.h"
#include "services/OrderBookEngine.h"

class Program
{
public:
    Program();
    void RunTest(const std::atomic<bool>& stopRequest);
    void RunWithListener(const std::atomic<bool> &stopRequest);
    void ProcessAndSend(const std::string& message, Order& newOrder);
    void SendNotification(const FillResults& results) const;
    OrderStatus SubmitOrder(Order& newOrder, char requestType);

private:
    OrderBookEngine Engine;
    const char* LobListenerFifo = "/tmp/lob_listen";
    const char* LobClientFifo = "/tmp/lob_client";
    bool ShouldHandleNotifications {false};
    std::random_device RandomDevice;

    int FindVectorData(const std::vector<int>& vec, int value) const;
    void PostResultsToLog(const FillResults& results) const;
};

#endif //LIMITORDERBOOK2_PROGRAM_H
