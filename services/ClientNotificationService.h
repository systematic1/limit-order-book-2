//
// Created by Kelly on 11/13/26.
//

#ifndef LIMITORDERBOOK2_CLIENTNOTIFICATIONSERVICE_H
#define LIMITORDERBOOK2_CLIENTNOTIFICATIONSERVICE_H

#include "../models/Enums.h"

class ClientNotificationService {
public:
    static void SendNotification(int accountId, long orderId, OrderStatus status, const char* message);
};

#endif //LIMITORDERBOOK2_CLIENTNOTIFICATIONSERVICE_H
