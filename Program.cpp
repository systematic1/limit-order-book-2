//
// Created by Kelly Akins on 11/16/26.
//

#include "Program.h"

#include <algorithm>
#include <charconv>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <vector>
#include <sys/stat.h>
#include <stdexcept>

#include "models/FillResults.h"

Program::Program()
{
    auto notify = [this](const long a, const long b, const OrderStatus c, const std::string& d) {
        SendNotification(a, b, c, d);
    };
    Engine = OrderBookEngine(1024, 512, notify);
    RandomDevice();
}

void Program::RunTest(const std::atomic<bool>& stopRequest)
{
    std::mt19937 rng(RandomDevice());
    std::uniform_int_distribution<int> randDistInt(1, 1000000);

    std::vector<int> orderNumbers;
    const int numberOfOrders = 500;
    const long highPrice = 1200L;
    const long lowPrice = 1150L;
    const int lowQuantity = 1;
    const int highQuantity = 50;

    OrderSide side = OrderSide::BUY;
    OrderType orderType = OrderType::LIMIT;
    char action = 'A';
    int newOrderId = 0;
    ShouldHandleNotifications = false;

    for (int index = 0; index < numberOfOrders; index++)
    {
        if (randDistInt(rng) % 10 > 5)
            side = OrderSide::BUY;
        else
            side = OrderSide::SELL;

        long price = randDistInt(rng) % (highPrice - lowPrice) + lowPrice;
        int quantity = randDistInt(rng) % (highQuantity - lowQuantity) + lowQuantity;

        if (randDistInt(rng) % 10 > 6)
            orderType = OrderType::LIMIT;
        else
            orderType = OrderType::MARKET;

        if (randDistInt(rng) % 3 > 0 || orderNumbers.empty())
        {
            action = 'A';
            do
            {
                newOrderId = randDistInt(rng);
            }
            while (FindVectorData(orderNumbers, newOrderId) != -1);

            orderNumbers.push_back(newOrderId);
        }
        else
        {
            action = 'C';
            newOrderId = orderNumbers[0];
            orderNumbers.erase(orderNumbers.begin(), orderNumbers.begin());
        }

        Order newOrder(newOrderId, price, side, orderType, quantity, quantity, -1,
            0L, TimeToLive::GTC, 99999L, 888888L);
        SubmitOrder(newOrder, action);

        std::cout << " * [" << index << "] Sent Order #" << newOrderId << ": " <<
            action << " (" << orderType << ") --> " << side << ": $" << "price * 100.0" <<
            ": " << quantity << std::endl;
    }
}

void Program::RunWithListener(const std::atomic<bool>& stopRequest)
{
    std::cout << "Limit Order Book Processing Engine - Running..." << std::endl;
    std::cout << "===========================================================================" << std::endl;

    ShouldHandleNotifications = true;
    auto* newOrders = new Order[1024];
    int lastOrderIndex = -1;

    while (!stopRequest)
    {
        try
        {
            if (!std::filesystem::exists(LobListenerFifo))
                mkfifo(LobListenerFifo, 0666);

            std::cout << "Listening on FIFO file: " << LobListenerFifo << std::endl;

            {
                std::ifstream inputStream(LobListenerFifo);
                std::string inputString;
                while (std::getline(inputStream, inputString))
                {
                    std::cout << "Received Line:   " << inputString << std::endl;

                    Order& newItem = newOrders[++lastOrderIndex];
                    ProcessAndSend(inputString, newItem);
                }
            }
        }
        catch (std::exception& e)
        {
            std::cout << "!! EXCEPTION: " << std::endl;
            std::cout << e.what() << std::endl;
            throw;
        }
    }

    delete[] newOrders;
}

void Program::ProcessAndSend(const std::string& message, Order& newOrder)
{
    std::string value;

    try
    {
        // Convert the char array to an Order object
        // The format of the char buffer data is:
        //  <ORDER-ID>|<REQ-TYPE>|<ORDER-TYPE>|<PRICE>|<SIDE>|<QTY>|<ACCT-ID>|<SECURITY-ID>|<SENT-TIME>

        // Detail descriptions:
        //  <REQ-TYPE> is "A" for add new order or "C" for cancel existing order
        //  <ORDER-TYPE> is "M" (market order) or "L" (limit order) - <PRICE> is ignored for "M"
        //  <PRICE> is mantissa format (real price rounded to four decimals * 10000) long int
        //  <SIDE> is "B" (buy) or "S" (sell)
        //  <SENT-TIME> is unix timestamp
        // All -ID fields are integers (not alphanumeric)

        // Examples:
        //  1234567890123|A|L|1235000|B|200|98765432109|1384|4652763476576474
        //  2345678901234|C|L|140000|S|50|98765432109|417|4673899928307501
        //  3456789012345|A|M|0|B|100|98765432109|1638|4678277000145185

        constexpr char separator {'|'};
        int fieldNumber {0};
        char requestType {'\0'};
        char tempChar {'\0'};
        size_t position {0}, endPosition {0};

        while (fieldNumber < 9 && endPosition < message.length())
        {
            endPosition = message.find(separator, position);
            if (endPosition == -1)
                endPosition = message.length() + 1;

            if (endPosition != std::string::npos)
            {
                value = message.substr(position, endPosition - position - 1);
                std::from_chars_result result;

                switch (fieldNumber)
                {
                    case 0:
                        // Order-ID (long)
                        result = std::from_chars(value.data(), value.data() + value.size(), newOrder.OrderId);
                        if (result.ec != std::errc())
                            throw std::runtime_error("Order-ID must be 64-bit integer");
                        break;

                    case 1:
                        // Request-Type (char)
                        requestType = value[0];
                        if (requestType != 'A' && requestType != 'C')
                            throw std::runtime_error("Req-Type must be 'A' or 'C'");
                        break;

                    case 2:
                        // Order-Type (char)
                        tempChar = value[0];
                        if (tempChar == 'L')
                            newOrder.Type = OrderType::LIMIT;
                        else if (tempChar == 'M')
                            newOrder.Type = OrderType::MARKET;
                        else
                            throw std::runtime_error("Order-Type must be 'L' or 'M'");
                        break;

                    case 3:
                        // Price (long)
                        result = std::from_chars(value.data(), value.data() + value.size(), newOrder.Price);
                        if (result.ec != std::errc())
                            throw std::runtime_error("Price must be 64-bit integer");
                        break;

                    case 4:
                        // Side (char)
                        tempChar = value[0];
                        if (tempChar == 'B')
                            newOrder.Side = OrderSide::BUY;
                        else if (tempChar == 'S')
                            newOrder.Side = OrderSide::SELL;
                        else
                            throw std::runtime_error("Side must be 'B' or 'S'");
                        break;

                    case 5:
                        // Qty (int)
                        result = std::from_chars(value.data(), value.data() + value.size(), newOrder.TotalQuantity);
                        if (result.ec != std::errc())
                            throw std::runtime_error("Quantity must be 32-bit integer");
                        break;

                    case 6:
                        // Acct-ID (long)
                        result = std::from_chars(value.data(), value.data() + value.size(), newOrder.AccountId);
                        if (result.ec != std::errc())
                            throw std::runtime_error("Account-ID must be 64-bit integer");
                        break;

                    case 7:
                        // Security-ID (long)
                        result = std::from_chars(value.data(), value.data() + value.size(), newOrder.SecurityId);
                        if (result.ec != std::errc())
                            throw std::runtime_error("Security-ID must be 64-bit integer");
                        break;

                    case 8:
                        // Sent-Time (long)
                        result = std::from_chars(value.data(), value.data() + value.size(), newOrder.Timestamp);
                        if (result.ec != std::errc())
                            throw std::runtime_error("Sent-Time must be 64-bit integer");
                        break;

                    default:
                        // ignore
                        break;
                }

                fieldNumber++;
                position = endPosition + 1;
            }
        }
    }
    catch (std::exception& e)
    {
        std::cout << "!! EXCEPTION:" << std::endl;
        std::cout << e.what() << std::endl;
        throw;
    }
}

void Program::SubmitOrder(Order& newOrder, const char requestType)
{
    OrderStatus status = OrderStatus::UNKNOWN;
    char type = '\0';
    char side = '\0';

    // Determine the order type and call the appropriate method to handle
    switch (requestType)
    {
        case 'A':
            side = newOrder.Side == OrderSide::SELL ? 'S' : 'B';
            type = newOrder.Type == OrderType::LIMIT ? 'L' : 'M';
            std::cout << "Adding order #" << newOrder.OrderId << " => " <<
                side << ", $" << newOrder.Price << ", QTY=" << newOrder.TotalQuantity <<
                ", " << type << std::endl;
            status = Engine.AddOrder(newOrder);
            break;

        case 'C':
            std::cout << "Canceling order #" << newOrder.OrderId;
            status = Engine.CancelOrder(newOrder);
            break;

        default:
            std::cout << "Invalid request type - request is skipped";
            status = OrderStatus::REJECTED;
            break;
    }

    //  * Send notification back to originator
    Engine.NotifyOrderFillStatus(newOrder, status, NullFillResults);
}

void Program::SendNotification(long accountId, long orderId, OrderStatus status, const std::string& message) const
{
    if (ShouldHandleNotifications)
    {
        const auto epoch_millis = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        long timestamp = epoch_millis;

        // Submits a message to the NamedPipe stream (to client)
        // <ACCT-ID>|<ORDER-ID>|<STATUS>|<TIMESTAMP>|<DETAIL-MESSAGE>
        // 99999|12345678|4|1236417823646|Filled Buy order with 400 at $14.55

        if (!std::filesystem::exists(LobClientFifo))
            mkfifo(LobClientFifo, 0666);

        {
            std::ostringstream oss;
            oss << accountId << '|' << orderId << '|' << status << '|' << timestamp << '|' << message;

            std::ofstream outStream(LobClientFifo);
            outStream << oss.str() << std::endl;
            outStream.flush();
        }
    }
}

int Program::FindVectorData(const std::vector<int>& vec, int value) const
{
    for (int index = 0; index < vec.size(); index++)
    {
        if (vec[index] == value)
            return index;
    }

    return -1;
}
