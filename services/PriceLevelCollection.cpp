//
// Created by Kelly on 11/13/26.
//

#include "PriceLevelCollection.h"

#include <algorithm>
#include <stdexcept>
#include <climits>

PriceLevelCollection::PriceLevelCollection(int capacityPowerOf2)
{
    if (capacityPowerOf2 > 0 && (capacityPowerOf2 & (capacityPowerOf2 - 1)) != 0)
        throw std::invalid_argument("The capacityPowerOf2 parameter must be a power of 2.");

    Capacity = capacityPowerOf2;
    CapacityMask = capacityPowerOf2 - 1;
    CountInUse = 0;
    LastIndex = 0;
    HighestIndex = -1;

    Levels = std::vector<PriceLevel>(Capacity);
    Available = std::vector<bool>(Capacity);

    for (int index = 0; index < Capacity; index++)
    {
        Available[index] = true;
        Levels[index] = PriceLevel();
    }
}

int PriceLevelCollection::MaxIndex() const
{
    return HighestIndex;
}

bool PriceLevelCollection::IsIndexInUse(const int index) const
{
    if (index >= 0 && index <= HighestIndex)
        return !Available[index];

    throw std::invalid_argument("PriceLevelCollection::IsIndexInUse - Invalid index");
}

const PriceLevel& PriceLevelCollection::LevelAtIndex(const int index) const
{
    if (IsIndexInUse(index))
        return Levels[index];

    throw std::invalid_argument("PriceLevelCollection::LevelAtIndex - Invalid index.");
}

int PriceLevelCollection::FindLevelIndexForPrice(const long price) const
{
    if (CountInUse > 0)
    {
        for (int index = 0; index <= HighestIndex; index++)
        {
            if (!Available[index] && Levels[index].Price == price)
                return index;
        }
    }

    return -1;
}

PriceLevel& PriceLevelCollection::GetLevelByIndex(const int index)
{
    if (IsIndexInUse(index))
        return Levels[index];

    throw std::invalid_argument("PriceLevelCollection::GetLevelByIndex - Invalid index.");
}

int PriceLevelCollection::GetNextHigherPriceLevel(const long price) const
{
    long nearestHigherPrice = LONG_MAX;
    int nextHigherIndex = -1;

    for (int index = 0; index <= HighestIndex; index++)
    {
        if (!Available[index] && Levels[index].Price > price && Levels[index].Price < nearestHigherPrice)
        {
            if (Levels[index].Price < nearestHigherPrice)
            {
                nearestHigherPrice = Levels[index].Price;
                nextHigherIndex = index;
            }
        }
    }

    return nextHigherIndex;
}

int PriceLevelCollection::GetNextLowerPriceLevel(const long price) const
{
    long nearestLowerPrice = LONG_MIN;
    int nextLowerIndex = -1;

    for (int index = 0; index <= HighestIndex; index++)
    {
        if (!Available[index] && Levels[index].Price < price && Levels[index].Price > nearestLowerPrice)
        {
            if (Levels[index].Price > nearestLowerPrice)
            {
                nearestLowerPrice = Levels[index].Price;
                nextLowerIndex = index;
            }
        }
    }

    return nextLowerIndex;
}

int PriceLevelCollection::AddPriceLevel(const long price, const OrderSide side)
{
    if (CountInUse < Capacity)
    {
        // 1. Find the price level in the list that has the next higher price above this order [HigherLevel]
        int nextHigherIndex = GetNextHigherPriceLevel(price);
        int nextLowerIndex = GetNextLowerPriceLevel(price);

        // 2. Save the price level index from HigherLevel.NextLowerIndex [LowerLevel]
        //int higherLevel_LowerIndex = (nextHigherIndex == -1 ? -1 : Levels[nextHigherIndex].NextLowerLevelIndex);

        // 3. Insert a new price level [NewLevel] and link HigherLevel.NextLowerIndex to its index
        int loopCount = 0;
        int searchIndex = LastIndex;

        while (!Available[searchIndex] && loopCount < Capacity)
        {
            searchIndex = (searchIndex + 1) & CapacityMask;
            loopCount++;
        }

        // 4. If an available slot was found, use it and mark it as used
        if (loopCount < Capacity)
        {
            LastIndex = searchIndex;
            Levels[LastIndex].Clear();
            Levels[LastIndex].Price = price;
            Levels[LastIndex].Side = side;

            // 5. Link NewLevel.NextHigherIndex to HigherLevel, and link NewLevel.NextLowerIndex to LowerLevel
            // Link LowerLevel.NextHigherIndex to NewLevel
            Levels[LastIndex].NextLowerLevelIndex = nextLowerIndex; // higherLevel_LowerIndex;
            Levels[LastIndex].NextHigherLevelIndex = nextHigherIndex;

            if (nextHigherIndex != -1)
                Levels[nextHigherIndex].NextLowerLevelIndex = LastIndex;
            if (nextLowerIndex != -1)
                Levels[nextLowerIndex].NextHigherLevelIndex = LastIndex;

            //if (higherLevel_LowerIndex > -1)
            //    Levels[higherLevel_LowerIndex].NextHigherLevelIndex = LastIndex;

            // 6. Mark NewLevel's AvailableIndex as 0 and increment CountInUse
            Available[LastIndex] = false;
            CountInUse++;

            if (LastIndex > HighestIndex)
                HighestIndex = LastIndex;

            return LastIndex;
        }
    }

    return -1;
}

bool PriceLevelCollection::RemovePriceLevel(long price)
{
    if (CountInUse > 0)
    {
        // 1. Find the price level for the given price [CurrentLevel]; return false if it doesn't exist
        int index = FindLevelIndexForPrice(price);
        if (index != -1)
        {
            // 2. Link the HigherLevel.NextLowerIndex to LowerLevel index
            int nextHigherIndex = Levels[index].NextHigherLevelIndex;
            int nextLowerIndex = Levels[index].NextLowerLevelIndex;

            if (nextHigherIndex != -1)
                Levels[nextHigherIndex].NextLowerLevelIndex = nextLowerIndex;

            // 3. Link the LowerLevel.NextHigherIndex to HigherLevel index
            if (nextLowerIndex != -1)
                Levels[nextLowerIndex].NextHigherLevelIndex = nextHigherIndex;

            // 4. Clear the CurrentLevel item content
            Levels[index].Clear();

            // 5. Mark NewLevel's AvailableIndex as true and decrement CountInUse
            Available[index] = true;
            LastIndex = index;
            CountInUse--;

            if (index == HighestIndex)
                HighestIndex--;

            return true;
        }
    }

    return false;
}
