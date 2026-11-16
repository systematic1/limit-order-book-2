#include <iostream>

#include "models/Enums.h"
#include "models/Order.h"
#include "models/PriceLevel.h"
#include "models/FillResults.h"
#include <csignal>

#include "Program.h"

std::atomic<bool> stopRequest {false};

void sig_handler(int signum)
{
    if (signum == SIGINT)
        stopRequest = true;
}

int main()
{
    std::signal(SIGINT, sig_handler);
    std::cout << "Press Ctrl+C to exit..." << std::endl << std::endl;

    Program p;
    p.RunTest(stopRequest);
    //p.RunWithListener(stopRequest);
}
