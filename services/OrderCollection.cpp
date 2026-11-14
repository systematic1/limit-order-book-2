//
// Created by Kelly on 11/13/26.
//

#include "OrderCollection.h"

#include <stdexcept>

OrderCollection::OrderCollection(int capacityPowerOf2) {
    if (capacityPowerOf2 > 0 && (capacityPowerOf2 & (capacityPowerOf2 - 1)) != 0)
        throw std::invalid_argument("The capacityPowerOf2 parameter must be a positive power of 2.");

    Capacity = capacityPowerOf2;
    CapacityMask = Capacity - 1;
    CountInUse = 0;
    LastIndex = 0;

    Orders = new Order[Capacity];
    Available = new bool[Capacity];
}

OrderCollection::~OrderCollection() {
    delete Orders;
    delete Available;
}

int OrderCollection::FindOrderIndex(const long orderId) const {
    if (CountInUse > 0){
        for (int index = 0; index < Capacity; index++){
            if (!Available[index] && Orders[index].OrderId == orderId)
                return index;
        }
    }

    return -1;
}

Order& OrderCollection::GetOrder(long orderId) {
    int index = FindOrderIndex(orderId);
    if (index == -1)
        return NullOrder;

    return Orders[index];
}

int OrderCollection::AddOrder(const Order& order, PriceLevel& priceLevel) {
/* C#
        if (_CountInUse < _Capacity)
        {
            if (FindOrderIndex(order.OrderId) != -1)
                return -1;

            if (priceLevel.Price != order.Price)
                return -1;

            int loopCount = 0;
            int searchIndex = _LastIndex;

            while (!_Available[searchIndex] && loopCount < _Capacity)
            {
                searchIndex = (searchIndex + 1) & _CapacityMask;
                loopCount++;
            }

            // If we found an open slot, then use it and mark it used
            if (loopCount < _Capacity)
            {
                _LastIndex = searchIndex;
                _Orders[_LastIndex].InitFrom(order);
                _Available[_LastIndex] = false;
                _CountInUse++;

                // Update order time-sort indices
                _Orders[_LastIndex].NextNewerOrderIndexAtLevel = -1;
                _Orders[_LastIndex].NextOlderOrderIndexAtLevel = priceLevel.NewestOrderIndex;

                // Update relevant PriceLevel newest/oldest order indices at that level
                if (priceLevel.NewestOrderIndex != -1)
                    _Orders[priceLevel.NewestOrderIndex].NextNewerOrderIndexAtLevel = _LastIndex;
                else
                    priceLevel.OldestOrderIndex = _LastIndex;

                priceLevel.NewestOrderIndex = _LastIndex;

                return _LastIndex;
            }
        }

        return -1;
*/
}

bool OrderCollection::RemoveOrder(const Order& order, PriceLevel& priceLevel) {
/* C#
        if (order.Price != priceLevel.Price)
            return false;

        if (_CountInUse > 0)
        {
            int index = FindOrderIndex(order.OrderId);
            if (index != -1)
            {
                // Update newer/older order index pointers for same price level
                int olderIndex = _Orders[index].NextOlderOrderIndexAtLevel;
                int newerIndex = _Orders[index].NextNewerOrderIndexAtLevel;

                if (olderIndex != -1)
                    _Orders[olderIndex].NextNewerOrderIndexAtLevel = newerIndex;
                else
                    priceLevel.OldestOrderIndex = newerIndex;

                if (newerIndex != -1)
                    _Orders[newerIndex].NextOlderOrderIndexAtLevel = olderIndex;
                else
                    priceLevel.NewestOrderIndex = olderIndex;

                // Clear it out and mark its slot as available
                _Orders[index].Clear();
                _Available[index] = true;
                _LastIndex = index > 0 ? index - 1 : 0;
                _CountInUse--;

                return true;
            }
        }

        return false;
*/
}

Order& OrderCollection::FindBestAtPriceLevel(PriceLevel& nextLevel, const PriceLevelCollection& levels,
    OrderSide matchSide) {
/* C#
        // Search for the next order by date/time and price (ask=search higher, bid=search lower)
        // * Use the PriceLevel.NextHigherIndex or PriceLevel.NextLowerIndex
        // * Move to next price level and search for orders matching that price level index
        // * Find the oldest order

        //ref PriceLevel nextLevel = ref priceLevel;
        int nextIndex;

        while (!Unsafe.IsNullRef(ref nextLevel) && nextLevel.IsEmpty())
        {
            nextIndex = (matchSide == OrderSide.Buy ? nextLevel.NextLowerLevelIndex : nextLevel.NextHigherLevelIndex);
            if (nextIndex != -1)
                nextLevel = ref levels.GetPriceLevelByIndex(nextIndex);
            else
                nextLevel = ref Unsafe.NullRef<PriceLevel>();
        }

        if (!Unsafe.IsNullRef(ref nextLevel) && nextLevel.OldestOrderIndex != -1)
        {
            if (nextLevel.OldestOrderIndex < _Capacity && !_Available[nextLevel.OldestOrderIndex])
                return ref _Orders[nextLevel.OldestOrderIndex];
        }

        return ref Unsafe.NullRef<Order>();
*/
}
