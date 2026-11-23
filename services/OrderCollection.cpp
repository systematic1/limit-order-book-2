//
// Created by Kelly on 11/13/26.
//

#include "OrderCollection.h"

#include <stdexcept>

OrderCollection::OrderCollection(const int capacityPowerOf2)
{
    if (capacityPowerOf2 > 0 && (capacityPowerOf2 & (capacityPowerOf2 - 1)) != 0)
        throw std::invalid_argument("The capacityPowerOf2 parameter must be a positive power of 2.");

    Capacity = capacityPowerOf2;
    CapacityMask = Capacity - 1;
    CountInUse = 0;
    LastIndex = 0;
    HighestIndex = -1;

    Orders = std::vector<Order>(Capacity);
    Available = std::vector<bool>(Capacity);

    for (int index = 0; index < Capacity; index++)
    {
        Available[index] = true;
        Orders[index] = Order();
    }
}

int OrderCollection::MaxIndex() const
{
    return HighestIndex;
}

const Order& OrderCollection::OrderAtIndex(const int index) const
{
    if (IsIndexInUse(index))
        return Orders[index];

    throw std::invalid_argument("OrderCollection::OrderAtIndex - Invalid index");
}

bool OrderCollection::IsIndexInUse(const int index) const
{
    if (index >= 0 && index <= HighestIndex)
        return !Available[index];

    throw std::invalid_argument("OrderCollection::IsIndexInUse - Invalid index");
}

int OrderCollection::FindOrderIndex(const long orderId) const
{
    if (CountInUse > 0)
    {
        for (int index = 0; index <= HighestIndex; index++)
        {
            if (!Available[index] && Orders[index].OrderId == orderId)
                return index;
        }
    }

    return -1;
}

Order& OrderCollection::GetOrderByIndex(const int index)
{
    if (IsIndexInUse(index))
        return Orders[index];

    throw std::invalid_argument("OrderCollection::GetOrderByIndex - Invalid index");
}

int OrderCollection::AddOrder(const Order& order, PriceLevel& priceLevel)
{
    if (CountInUse < Capacity)
    {
        if (FindOrderIndex(order.OrderId) != -1)
            return -1;

        if (priceLevel.Price != order.Price)
            return -1;

        int loopCount = 0;
        int searchIndex = LastIndex;

        while (!Available[searchIndex] && loopCount < Capacity)
        {
            searchIndex = (searchIndex + 1) & CapacityMask;
            loopCount++;
        }

        // If we found an open slot, then use it and mark it used
        if (loopCount < Capacity)
        {
            LastIndex = searchIndex;
            Orders[LastIndex] = order;
            Available[LastIndex] = false;
            CountInUse++;

            // Update order time-sort indices
            Orders[LastIndex].NextNewerOrderIndexAtLevel = -1;
            Orders[LastIndex].NextOlderOrderIndexAtLevel = priceLevel.NewestOrderIndex;

            // Update relevant PriceLevel newest/oldest order indices at that level
            if (priceLevel.NewestOrderIndex != -1)
                Orders[priceLevel.NewestOrderIndex].NextNewerOrderIndexAtLevel = LastIndex;
            else
                priceLevel.OldestOrderIndex = LastIndex;

            priceLevel.NewestOrderIndex = LastIndex;

            if (LastIndex > HighestIndex)
                HighestIndex = LastIndex;

            return LastIndex;
        }
    }

    return -1;
}

bool OrderCollection::RemoveOrder(const Order& order, PriceLevel& priceLevel)
{
    if (order.Price != priceLevel.Price)
        return false;

    if (CountInUse > 0)
    {
        int index = FindOrderIndex(order.OrderId);
        if (index != -1)
        {
            // Update newer/older order index pointers for same price level
            int olderIndex = Orders[index].NextOlderOrderIndexAtLevel;
            int newerIndex = Orders[index].NextNewerOrderIndexAtLevel;

            if (olderIndex != -1)
                Orders[olderIndex].NextNewerOrderIndexAtLevel = newerIndex;
            else
                priceLevel.OldestOrderIndex = newerIndex;

            if (newerIndex != -1)
                Orders[newerIndex].NextOlderOrderIndexAtLevel = olderIndex;
            else
                priceLevel.NewestOrderIndex = olderIndex;

            // Clear it out and mark its slot as available
            Orders[index].Clear();
            Available[index] = true;
            LastIndex = index > 0 ? index - 1 : 0;
            CountInUse--;

            if (index == HighestIndex)
                HighestIndex--;

            return true;
        }
    }

    return false;
}

int OrderCollection::FindIndexBestAtPriceLevel(const PriceLevel& atLevel) const
{
    // Find the oldest order of all those at the given price level
    if (atLevel.Price > -1 && atLevel.OldestOrderIndex != -1)
    {
        if (atLevel.OldestOrderIndex <= HighestIndex && !Available[atLevel.OldestOrderIndex])
            return atLevel.OldestOrderIndex;
    }
    else if (atLevel.Price > -1 && atLevel.OrderCount == 1 && atLevel.NewestOrderIndex != -1)
        return atLevel.NewestOrderIndex;

    return -1;
}
