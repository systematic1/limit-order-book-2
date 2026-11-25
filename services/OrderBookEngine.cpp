//
// Created by Kelly on 11/13/26.
//

#include "OrderBookEngine.h"
#include "../models/Enums.h"

#include <iostream>
#include <ostream>
#include <sstream>
#include <utility>
#include <filesystem>
#include <chrono>

OrderBookEngine::OrderBookEngine() : Orders(0), AskPriceLevels(0), BidPriceLevels(0)
{
}

OrderBookEngine::OrderBookEngine(const int orderCapacity, const int priceLevelCapacity,
                                 std::function<void(const FillResults&)> notifyFunction) :
    Orders(orderCapacity), AskPriceLevels(priceLevelCapacity), BidPriceLevels(priceLevelCapacity),
    NotifyFunction(std::move(notifyFunction))
{
}

OrderStatus OrderBookEngine::AddOrder(Order& order)
{
    int levelIndex;
    int quantityFilled = 0;
    bool wasFilled = true;
    long price = order.Price;

    OrderStatus status = ValidateOrder(order);
    if (status != OrderStatus::UNKNOWN)
        return status;

    if (DoesOrderViolateRisk(order))
        return OrderStatus::REJECTED;

    if (order.Side == OrderSide::BUY)
    {
        // If market order, find the lowest ask price available to start
        levelIndex = AskPriceLevels.GetNextHigherPriceLevel(LONG_MIN);
        if (levelIndex != -1)
            price = AskPriceLevels.GetLevelByIndex(levelIndex).Price;

        // We keep looking for more orders to fill until our order is completed
        while (order.RemainingQuantity > 0 && wasFilled)
        {
            int index = AskPriceLevels.FindLevelIndexForPrice(price);
            if (index == -1)
            {
                levelIndex = AskPriceLevels.GetNextHigherPriceLevel(price);
                if (levelIndex == -1)   // If no more orders are available, jump out of loop here
                    break;
            }

            // Find a matching order and fill it
            PriceLevel& priceLevel = AskPriceLevels.GetLevelByIndex(levelIndex);
            quantityFilled = FillMatchingOrder(priceLevel, order);
            wasFilled = (quantityFilled > 0);

            if (priceLevel.IsEmpty())
                AskPriceLevels.RemovePriceLevel(priceLevel.Price);
        }
    }
    else    // Sell OrderSide
    {
        // If market order, find the highest bid price available to start
        levelIndex = BidPriceLevels.GetNextLowerPriceLevel(LONG_MAX);
        if (levelIndex != -1)
            price = BidPriceLevels.GetLevelByIndex(levelIndex).Price;

        // We keep looking for more orders to fill until our order is completed
        while (order.RemainingQuantity > 0 && wasFilled)
        {
            int index = BidPriceLevels.FindLevelIndexForPrice(price);
            if (index == -1)
            {
                levelIndex = BidPriceLevels.GetNextLowerPriceLevel(price);
                if (levelIndex == -1)  // If no more orders are available, jump out of loop here
                    break;
            }

            // Find a matching order and fill it
            PriceLevel& priceLevel = BidPriceLevels.GetLevelByIndex(levelIndex);
            quantityFilled = FillMatchingOrder(priceLevel, order);
            wasFilled = (quantityFilled > 0);                                                   // ---

            if (priceLevel.IsEmpty())
                BidPriceLevels.RemovePriceLevel(priceLevel.Price);
        }
    }

    const auto epoch_millis = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    order.Timestamp = epoch_millis;

    // If we filled the entire order quantity, then return "filled" status
    if (order.RemainingQuantity == 0)
        return OrderStatus::FILLED;

    // If we couldn't fill any of a market order, reject it, otherwise partial fill it.
    if (order.Type == OrderType::MARKET)
        return quantityFilled > 0 ? OrderStatus::PARTIALLY_FILLED : OrderStatus::REJECTED_NO_MATCHES;

    // If we run out of qualifying limit orders, store the order in the book (resting)
    if (AddOrderToBook(order))
        return quantityFilled > 0 ? OrderStatus::PARTIALLY_FILLED : OrderStatus::ACCEPTED_AND_WAITING;

    return OrderStatus::REJECTED;
}

OrderStatus OrderBookEngine::CancelOrder(const long orderId)
{
    int orderIndex = Orders.FindOrderIndex(orderId);
    if (orderIndex == -1)
        return OrderStatus::NOT_FOUND;

    Order& storedOrder = Orders.GetOrderByIndex(orderIndex);
    PriceLevelCollection& side = GetSideLevelCollection(storedOrder.Side);

    // Find the price level and adjust the quantity remaining and order count
    int levelIndex = side.FindLevelIndexForPrice(storedOrder.Price);
    if (levelIndex == -1)
        return OrderStatus::NOT_FOUND;

    PriceLevel& priceLevel = side.GetLevelByIndex(levelIndex);
    priceLevel.QuantityAvailable -= storedOrder.RemainingQuantity;
    priceLevel.OrderCount--;

    // If the price level is emptied, remove it
    if (priceLevel.OrderCount <= 0)
        side.RemovePriceLevel(storedOrder.Price);

    // Remove the order from the order list
    storedOrder.PriceLevelIndex = -1;

    if (!Orders.RemoveOrder(storedOrder, priceLevel))
        return OrderStatus::REJECTED;

    return OrderStatus::CANCELLED;
}

PriceLevelCollection& OrderBookEngine::GetSideLevelCollection(OrderSide side)
{
    if (side == OrderSide::BUY)
        return BidPriceLevels;
    else
        return AskPriceLevels;
}

void OrderBookEngine::NotifyOrderFillStatus(const FillResults& fillResults) const
{
    try
    {
        NotifyFunction(fillResults);
    }
    catch (std::exception& e)
    {
        std::cout << "Error while notifying client." << std::endl << e.what() << std::endl << std::endl;
    }
}

OrderStatus OrderBookEngine::ValidateOrder(const Order& order) const
{
    // Validate - Basic
    if (order.TotalQuantity < order.RemainingQuantity)
        return OrderStatus::REJECTED;

    if (order.TotalQuantity <= 0 || order.RemainingQuantity <= 0)
        return OrderStatus::REJECTED;

    if (IsDuplicateOrder(order.OrderId))
        return OrderStatus::DUPLICATE;

    return OrderStatus::UNKNOWN;
}

bool OrderBookEngine::DoesOrderViolateRisk(const Order& order) const
{
    // This will not be implemented - always assume risk check passes
    return false;
}

int OrderBookEngine::FillMatchingOrder(PriceLevel& priceLevel, Order& order)
{
    OrderSide matchSide = (order.Side == OrderSide::SELL ? OrderSide::BUY : OrderSide::SELL);
    OrderStatus matchedStatus = OrderStatus::UNKNOWN;
    OrderStatus currentStatus = OrderStatus::UNKNOWN;
    PriceLevelCollection& priceLevels = GetSideLevelCollection(order.Side);

    int quantityRemaining = order.RemainingQuantity;
    int quantityFilled = 0;
    int matchRemaining = 0;
    int currentRemaining = quantityRemaining;

    const auto epoch_millis = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    // Search the order list for the best available order by time-priority
    int matchedIndex = Orders.FindIndexBestAtPriceLevel(priceLevel);
    if (matchedIndex == -1)
        return 0;

    Order& matchedOrder = Orders.GetOrderByIndex(matchedIndex);
    matchRemaining = matchedOrder.RemainingQuantity;

    if (matchedOrder.AccountId != order.AccountId && matchedOrder.RemainingQuantity > 0)
    {
        if (matchedOrder.RemainingQuantity >= quantityRemaining)
        {
            matchedStatus = (matchedOrder.RemainingQuantity == quantityRemaining ? OrderStatus::FILLED : OrderStatus::PARTIALLY_FILLED);
            currentStatus = OrderStatus::FILLED;
            matchedOrder.RemainingQuantity -= quantityRemaining;
            matchedOrder.Timestamp = epoch_millis;
            order.Timestamp = matchedOrder.Timestamp;
            order.RemainingQuantity = 0;
            quantityFilled = quantityRemaining;
            quantityRemaining = order.RemainingQuantity;
        }
        else       // Not enough to completely fill order
        {
            quantityFilled = matchedOrder.RemainingQuantity;
            quantityRemaining -= quantityFilled;
            order.RemainingQuantity = quantityRemaining;
            order.Timestamp = epoch_millis;
            matchedStatus = OrderStatus::PARTIALLY_FILLED;
            currentStatus = OrderStatus::FILLED;
            matchedOrder.Timestamp = order.Timestamp;
            matchedOrder.RemainingQuantity = 0;
        }

        FillResults matchResults = FillResults(
            matchedOrder.OrderId,
            matchedOrder.Side,
            matchRemaining,
            quantityFilled,
            matchedOrder.RemainingQuantity,
            priceLevel.Price,
            0,
            std::string(),
            matchedStatus,
            matchedOrder.AccountId
        );
        SetStatusMessage(matchResults);
        NotifyOrderFillStatus(matchResults);

        FillResults currentResults = FillResults(
            order.OrderId,
            order.Side,
            currentRemaining,
            quantityFilled,
            order.RemainingQuantity,
            priceLevel.Price,
            0,
            std::string(),
            currentStatus,
            order.AccountId
        );
        SetStatusMessage(currentResults);
        NotifyOrderFillStatus(currentResults);

        // Update price level data
        priceLevel.QuantityAvailable -= quantityFilled;
        priceLevel.LastFillQuantity = quantityRemaining;
        priceLevel.LastFillTimestamp = epoch_millis;

        // If either order was completely filled, it needs to be removed from the orderbook
        if (matchedOrder.RemainingQuantity == 0)
            Orders.RemoveOrder(matchedOrder, priceLevel);
    }

    return quantityFilled;
}

bool OrderBookEngine::AddOrderToBook(Order& order)
{
    PriceLevelCollection& levelSide = GetSideLevelCollection(order.Side);
    int levelIndex = levelSide.FindLevelIndexForPrice(order.Price);

    // If the price level doesn't exist, add it to the price levels
    if (levelIndex == -1)
        levelIndex = levelSide.AddPriceLevel(order.Price, order.Side);
    if (levelIndex == -1)
        return false;

    order.PriceLevelIndex = levelIndex;

    PriceLevel& priceLevel = levelSide.GetLevelByIndex(levelIndex);
    priceLevel.QuantityAvailable += order.RemainingQuantity;
    priceLevel.OrderCount++;

    if (Orders.AddOrder(order, priceLevel) == -1)
        return false;

    return true;
}

void OrderBookEngine::SetStatusMessage(FillResults &fillResults)
{
    const char* sideName = fillResults.Side == OrderSide::BUY ? "Buy" : "Sell";
    std::ostringstream oss;

    switch (fillResults.Status)
    {
        case OrderStatus::CANCELLED:
            oss << "Order " << fillResults.OrderId << " has been cancelled.";
            break;

        case OrderStatus::DUPLICATE:
            oss << "This order (" << fillResults.OrderId << ") has already been submitted.";
            break;

        case OrderStatus::FILLED:
            oss << sideName << " Order " << fillResults.OrderId << " has been filled with quantity "
                << fillResults.FilledQuantity << " at $"
                << static_cast<float>(fillResults.FillPrice) / 100.0;
            break;

        case OrderStatus::NOT_FOUND:
            oss << "This order (" << fillResults.OrderId << ") is invalid or not found.";
            break;

        case OrderStatus::PARTIALLY_FILLED:
            oss << sideName << " Order " << fillResults.OrderId << " has been partially filled with quantity "
                << fillResults.FilledQuantity << " at $"
                << static_cast<float>(fillResults.FillPrice) / 100.0;
            break;

        case OrderStatus::REJECTED:
            oss << "Order " << fillResults.OrderId << " has been rejected for reason: "
                << fillResults.Message;
            break;

        case OrderStatus::ACCEPTED_AND_WAITING:
            oss << "Order " << fillResults.OrderId << " has been accepted (waiting).";
            break;

        default:
            oss << "Order status code " << fillResults.Status << " was returned for order " << fillResults.OrderId;
            break;
    }

    const std::string message = oss.str();
}

inline bool OrderBookEngine::IsDuplicateOrder(const long orderId) const
{
    return Orders.FindOrderIndex(orderId) != -1;
}

const OrderCollection& OrderBookEngine::GetOrderCollection() const
{
    return Orders;
}

const PriceLevelCollection& OrderBookEngine::GetAskPriceLevelCollection() const
{
    return AskPriceLevels;
}

const PriceLevelCollection& OrderBookEngine::GetBidPriceLevelCollection() const
{
    return BidPriceLevels;
}
