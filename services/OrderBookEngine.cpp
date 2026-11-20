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
    PriceLevel& priceLevel = NullPriceLevel;
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
            price = AskPriceLevels.GetPriceLevelByIndex(levelIndex).Price;

        // We keep looking for more orders to fill until our order is completed
        while (order.RemainingQuantity > 0 && wasFilled)
        {
            priceLevel = AskPriceLevels.GetPriceLevelByPrice(price);
            if (priceLevel.IsNull())
            {
                levelIndex = AskPriceLevels.GetNextHigherPriceLevel(price);
                if (levelIndex == -1)   // If no more orders are available, jump out of loop here
                    break;

                priceLevel = AskPriceLevels.GetPriceLevelByIndex(levelIndex);
            }

            // Find a matching order and fill it
            quantityFilled = FillMatchingOrder(priceLevel, order);
            wasFilled = (quantityFilled > 0);
        }
    }
    else    // Sell OrderSide
    {
        // If market order, find the highest bid price available to start
        levelIndex = BidPriceLevels.GetNextLowerPriceLevel(LONG_MAX);
        if (levelIndex != -1)
            price = BidPriceLevels.GetPriceLevelByIndex(levelIndex).Price;

        // We keep looking for more orders to fill until our order is completed
        while (order.RemainingQuantity > 0 && wasFilled)                                        // ---
        {
            priceLevel = BidPriceLevels.GetPriceLevelByPrice(price);
            if (priceLevel.IsNull())
            {
                levelIndex = BidPriceLevels.GetNextLowerPriceLevel(price);
                if (levelIndex == -1)  // If no more orders are available, jump out of loop here
                    break;

                priceLevel = BidPriceLevels.GetPriceLevelByIndex(levelIndex);
                order.PriceLevelIndex = levelIndex;
            }

            // Find a matching order and fill it
            quantityFilled = FillMatchingOrder(priceLevel, order);
            wasFilled = (quantityFilled > 0);                                                   // ---
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

OrderStatus OrderBookEngine::CancelOrder(long orderId)
{
    Order& storedOrder = Orders.GetOrder(orderId);
    if (storedOrder.IsNull())
        return OrderStatus::NOT_FOUND;

    // Find the price level and adjust the quantity remaining and order count
    PriceLevel& priceLevel = NullPriceLevel;
    if (storedOrder.Side == OrderSide::BUY)
        priceLevel = BidPriceLevels.GetPriceLevelByPrice(storedOrder.Price);
    else
        priceLevel = AskPriceLevels.GetPriceLevelByPrice(storedOrder.Price);

    if (priceLevel.IsNull())
        return OrderStatus::NOT_FOUND;

    priceLevel.QuantityAvailable -= storedOrder.RemainingQuantity;
    priceLevel.OrderCount--;

    // Remove the order from the order list
    storedOrder.PriceLevelIndex = -1;

    if (!Orders.RemoveOrder(storedOrder, priceLevel))
        return OrderStatus::REJECTED;

    return OrderStatus::CANCELLED;
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
    PriceLevelCollection& priceLevels = (order.Side == OrderSide::SELL ? BidPriceLevels : AskPriceLevels);
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

    Order& matchedOrder = Orders.GetOrder(matchedIndex);
    if (!matchedOrder.IsNull())
    {
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
            NotifyOrderFillStatus(matchResults);

            // Update price level data
            priceLevel.QuantityAvailable -= quantityFilled;
            priceLevel.LastFillQuantity = quantityRemaining;
            priceLevel.LastFillTimestamp = epoch_millis;

            // If either order was completely filled, it needs to be removed from the orderbook
            if (matchedOrder.RemainingQuantity == 0)
                Orders.RemoveOrder(matchedOrder, priceLevel);
        }
    }

    return quantityFilled;
}

bool OrderBookEngine::AddOrderToBook(Order& order)
{
    PriceLevel& priceLevel = NullPriceLevel;
    int levelIndex;

    if (order.Side == OrderSide::BUY)
    {
        priceLevel = BidPriceLevels.GetPriceLevelByPrice(order.Price);

        // If the price level doesn't exist, add it to the price levels
        if (priceLevel.IsNull())
        {
            levelIndex = BidPriceLevels.AddPriceLevel(order.Price, order.Side);
            if (levelIndex == -1)
                return false;

            priceLevel = BidPriceLevels.GetPriceLevelByIndex(levelIndex);
            order.PriceLevelIndex = levelIndex;
        }
    }
    else       // Sell OrderSide
    {
        priceLevel = AskPriceLevels.GetPriceLevelByPrice(order.Price);

        // If the price level doesn't exist, add it to the price levels
        if (priceLevel.IsNull())
        {
            levelIndex = AskPriceLevels.AddPriceLevel(order.Price, order.Side);
            if (levelIndex == -1)
                return false;

            priceLevel = AskPriceLevels.GetPriceLevelByIndex(levelIndex);
            order.PriceLevelIndex = levelIndex;
        }
    }

    if (priceLevel.IsNull())
        return false;

    priceLevel.QuantityAvailable += order.RemainingQuantity;
    priceLevel.OrderCount++;

    if (Orders.AddOrder(order, priceLevel) == -1)
        return false;

    return true;
}

void OrderBookEngine::SetStatusMessage(FillResults &fillResults)
{
    std::ostringstream oss;

    switch (fillResults.Status)
    {
        case OrderStatus::CANCELLED:
            oss << "Order " << fillResults.OrderId << " has been cancelled.";
            break;

        case OrderStatus::DUPLICATE:
            oss << "This order (" << fillResults.OrderId << " has already been submitted.";
            break;

        case OrderStatus::FILLED:
            if (!fillResults.IsNull())
            {
                oss << "Order " << fillResults.OrderId << " has been filled with quantity "
                    << fillResults.FilledQuantity << " at $"
                    << static_cast<float>(fillResults.FillPrice) / 100.0;
            }
            else
                oss << "Order " << fillResults.OrderId << " has been filled (no details).";
            break;

        case OrderStatus::NOT_FOUND:
            oss << "This order (" << fillResults.OrderId << ") is invalid.";
            break;

        case OrderStatus::PARTIALLY_FILLED:
            if (!fillResults.IsNull())
            {
                oss << "Order " << fillResults.OrderId << " has been partially filled with quantity "
                    << fillResults.FilledQuantity << " at $"
                    << static_cast<float>(fillResults.FillPrice) / 100.0;
            }
            else
                oss << "Order " << fillResults.OrderId << " has been partially filled (no details).";
            break;

        case OrderStatus::REJECTED:
            if (!fillResults.IsNull())
            {
                oss << "Order " << fillResults.OrderId << " has been rejected for reason: "
                    << fillResults.Message;
            }
            else
                oss << "Order " << fillResults.OrderId << " has been rejected (no details).";
            break;

        case OrderStatus::ACCEPTED_AND_WAITING:
            oss << "Order " << fillResults.OrderId << " has been accepted (waiting).";
            break;

        default:
            oss << "Order status " << fillResults.Status << " was returned for order " << fillResults.OrderId;
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
