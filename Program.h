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
    void SubmitOrder(Order& newOrder, char requestType);
    void SendNotification(long accountId, long orderId, OrderStatus status, const std::string& message) const;

private:
    OrderBookEngine Engine;
    std::string StringBuffer;
    const char* LobListenerFifo = "/tmp/lob_listen";
    const char* LobClientFifo = "/tmp/lob_client";
    bool ShouldHandleNotifications {false};
    std::random_device _rd;
    int FindVectorData(const std::vector<int>& vec, int value) const;
};

#endif //LIMITORDERBOOK2_PROGRAM_H
