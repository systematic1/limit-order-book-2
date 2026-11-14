//
// Created by Kelly Akins on 11/13/26.
//

#ifndef LIMITORDERBOOK2_PRICELEVEL_H
#define LIMITORDERBOOK2_PRICELEVEL_H


struct PriceLevel {
public:
    long Price;
    long QuantityAvailable;
    int OrderCount;
    OrderSide Side;
    long LastFillQuantity;
    long LastFillTimestamp;

    int NextHigherLevelIndex {-1};
    int NextLowerLevelIndex {-1};
    int OldestOrderIndex {-1};
    int NewestOrderIndex {-1};

    PriceLevel() = default;
    PriceLevel(const PriceLevel&) = default;
    PriceLevel(long price, long quantity, int orderCount, OrderSide side);
    void Clear();
    bool IsEmpty() const;
};


#endif //LIMITORDERBOOK2_PRICELEVEL_H
