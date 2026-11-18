//
// Created by Kelly on 11/13/26.
//

#ifndef LIMITORDERBOOK2_PRICELEVELCOLLECTION_H
#define LIMITORDERBOOK2_PRICELEVELCOLLECTION_H

#include "../models/PriceLevel.h"

class PriceLevelCollection {
private:
    int Capacity;
    int CapacityMask;
    PriceLevel* Levels;
    bool* Available;
    int CountInUse;
    int LastIndex;
    int HighestIndex;

    inline static PriceLevel NullPriceLevel = PriceLevel(-1L, -1L, -1L, OrderSide::BUY);

public:
    PriceLevelCollection(int capacityPowerOf2);
    ~PriceLevelCollection();

    int FindLevelIndex(long price) const;
    PriceLevel& GetPriceLevelByPrice(long price) const;
    PriceLevel& GetPriceLevelByIndex(int index) const;
    int GetNextHigherPriceLevel(long price) const;
    int GetNextLowerPriceLevel(long price) const;
    int AddPriceLevel(long price, OrderSide side);
    bool RemovePriceLevel(long price);
};

#endif //LIMITORDERBOOK2_PRICELEVELCOLLECTION_H
