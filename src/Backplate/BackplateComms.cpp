#include "BackplateComms.hpp"
#include "CommandMessage.hpp"
#include "ResponseMessage.hpp"
#include <unistd.h>
#include <iostream>
#include <sys/time.h>

bool BackplateComms::Initialize() 
{
    if (!InitializeSerial())
    {
        return false;
    }

    CommandMessage resetMsg(MessageType::Reset);
    SerialPort->Write(resetMsg.GetRawMessage());

    // Wait for 5s for backplate to send a bust of data
    // there will be a bunch of data points, ending with a BRK
    bool burstDone = false;
    struct timeval startTime, currentTime;
    gettimeofday(&startTime, nullptr);
    
    while (true)
    {
        if (IsTimeout(currentTime, startTime))
            break;

        uint8_t readBuffer[256];
        size_t totalBytesRead = 0;
        totalBytesRead = SerialPort->Read(
            reinterpret_cast<char*>(readBuffer), 
            sizeof(readBuffer)
        );

        std::cout << "Read " << totalBytesRead << " bytes" << std::endl;
        
        if (totalBytesRead == 0)
        {
            std::cout << "No data read, continuing..." << std::endl;
            usleep(100000); // 100ms
            continue;
        }

        // Parse the response to find the BRK message
        ResponseMessage responseMsg;
        responseMsg.ParseMessage(readBuffer, totalBytesRead);
        const std::vector<uint8_t> brk = {'B','R','K'};
        if (responseMsg.GetMessageCommand() == MessageType::ResponseAscii
        && responseMsg.GetPayload() == brk)
        {
            burstDone = true;
            break;
        }
    }

    if (!burstDone)
    {
        return false;
    }

    return true;
}

bool BackplateComms::IsTimeout(timeval &currentTime, timeval &startTime)
{
    gettimeofday(&currentTime, nullptr);
    long elapsedUs = (currentTime.tv_sec - startTime.tv_sec) * 1000000 +
                     (currentTime.tv_usec - startTime.tv_usec);

    if (elapsedUs >= BurstTimeoutUs)
    {
        return true;
    }
    return false;
}

bool BackplateComms::InitializeSerial()
{
    const BaudRate baudRate = BaudRate::Baud115200;

    if (!SerialPort->Open(baudRate))
    {
        return false;
    }

    SerialPort->Flush();
    SerialPort->SendBreak(0);
    usleep(250000);
    SerialPort->Flush();

    return true;
}
