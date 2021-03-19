#include "arduino-serial-lib.h"
#include <string.h>
#include <stdlib.h>
#include <array>
#include <iostream>
#include <unistd.h>
#include <chrono>

#define buf_max 256
#define baudrate 2000000
#define timeout 1000
#define eolchar '\n'

auto startTime = std::chrono::high_resolution_clock::now();

template <typename timeType>
long getTime() {
    return std::chrono::duration_cast<timeType>(std::chrono::high_resolution_clock::now() - startTime).count();
}

class Trinket {
    const char* address;
protected:
    
    
    int fd = -1;
    char serialport[buf_max];
    char buf[buf_max];
    int rc,n;
    
    Trinket(const char* address) {
        this->address = address;
        try {
            this->fd = serialport_init(address, baudrate);
        }
        catch (int x) {
            std::cout << "something went REALLY wrong... maybe the trinket isnt plugged in?" << std::endl;
            exit(9);
        }
        this->reset();
    }
    
    void sendRequest(const char* request, char* result) {
    serialWrite:
        try {
            serialport_write(fd, request);
        }
        catch (int x){
            usleep(500);
            goto serialWrite;
        }
        char discard[1];
        serialport_read_until(fd,  result,  eolchar,  buf_max,  timeout);
        serialport_read_until(fd,  discard,  eolchar,  buf_max,  timeout);
    }
public:
    void reset() {
        std::cout << this->address << ": Resetting Device" << std::endl;
        char result[8];
        this->sendRequest("&", result);
        #if 0
        close(this->fd);
        auto adrOLD = "/dev/ttyACM1";
    openAgain:
        try {
            this->fd = serialport_init(this->address, baudrate);
        }
        catch (int x){
            usleep(500);
            goto openAgain;
        }
        #endif
        std::cout << this->address << ": Reset done" << std::endl;
    }
    
};

class EMGTrinket : public Trinket {
public:
    EMGTrinket(const char* address) : Trinket(address) {
        
    }
    
    int getEMG(unsigned short adr) {
        char result[8];
        switch (adr) {
            case 0:
                this->sendRequest("0", result);
                break;
            case 1:
                this->sendRequest("1", result);
                break;
            case 2:
                this->sendRequest("2", result);
                break;
            case 3:
                this->sendRequest("3", result);
                break;
            default:
                break;
        }
        return atoi(result);
    }
    
};

class EncoderTrinket : public Trinket {
    short directionBit = 1;
public:
    
    EncoderTrinket(const char* address) : Trinket(address) {
        
    }

    void flipDirectionBit() {
        this->directionBit *= -1;
    }
    
    void resetMotorCounts() {
        char result[8];
        sendRequest("?", result);
        std::cout << "RESET MESSAGE: " << std::string(result);
#if 0
        if (std::string(result) !=  "RESET\n") {
            std::cout << "ERROR RESETTING MOTOR COUNT: " << result;
        }
        
        else {
            std::cout << "Reset Succesful" << std::endl;
        }
#endif
    }
    
    int getMotor1Value() {
        char buf[12];
        sendRequest("0", buf);
        return atoi(buf) - 16777216;
    }
    
    int getMotor2Value() {
        char buf[12];
        sendRequest("1", buf);
        return atoi(buf) - 16777216;
    }
    
    std::array<int, 2> getMotorValues() {
        return {getMotor1Value(), getMotor2Value()};
    }
};
