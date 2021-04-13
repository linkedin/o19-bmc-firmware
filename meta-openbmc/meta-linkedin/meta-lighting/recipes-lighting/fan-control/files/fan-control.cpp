/*
 * config-util
 *
 * Copyright 2019-present LinkedIn. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fstream>
#include <syslog.h>
#include <string>
#include <bits/stdc++.h>
#include <nlohmann/json.hpp>
#include <experimental/filesystem>

extern "C"
{
    #include <facebook/powershelf.h>
    #include <openbmc/gpio.h>
}

using json = nlohmann::json;

#define MAX_RPM 30300 // RPM
#define ADJUSTMENT_PERIOD 60 // seconds
#define UPPER_THRESHOLD 55 // celcius
#define LOWER_THRESHOLD 45 // celcius
#define MAX_DEVIATION 10 // percent from set value

static int upperThreshold;
static int lowerThreshold;

class Fan
{
public:

    Fan(std::string name, std::string path, float a, float b, float c) : name(name), path(path), a(a), b(b), c(c) {}

    std::string name;
    std::string path;
    float a;
    float b;
    float c;

    int getRPM()
    {
        std::ifstream ifile(path+"fan1_input");
        if(!ifile.is_open())
        {
            printf("ERROR: open path to %s failed!\n", name.c_str());
            syslog(LOG_WARNING, "ERROR: open path to %s failed!", name.c_str());
            return 0;
        }

        i2c_master_selector_access(8, 0x7C);
        std::string val;
        ifile >> val;
        ifile.close();
        int rpm = std::stoi(val);
        int percent = rpmToPercent(rpm);
        printf("RPM %s = %d, which is %d%%\n", name.c_str(), rpm, percent);
        return rpm;
    }

    int setPWM(uint8_t percent)
    {
        std::ofstream ofile(path+"pwm1");
        if(!ofile.is_open())
        {
            printf("ERROR: open path to %s failed!\n", name.c_str());
            syslog(LOG_WARNING, "ERROR: open path to %s failed!", name.c_str());
            return -1;
        }

        int p = percentToPwm(percent);
        printf("set %s to %d pwm value, which is %d%%\n", name.c_str(), p, percent);
        i2c_master_selector_access(8, 0x7C);
        ofile << p;
        ofile.close();
        return 0;
    }

private:
    uint8_t percentToPwm(unsigned int percent)
    {
        unsigned int result = 255 * percent / 100;
        return result;
    }

    uint8_t rpmToPercent(unsigned int rpm)
    {
        unsigned int result = rpm * 100 / MAX_RPM;
        return result;
    }
};

class Temp
{
public:
    Temp(std::string name, std::string path) : name(name), path(path) {}

    std::string name;
    std::string path;

    int getTemp()
    {
        std::string val;
        std::ifstream ifile(path+"temp1_input");
        if(!ifile.is_open())
        {
            printf("ERROR: open path to %s failed!\n", name.c_str());
            syslog(LOG_WARNING, "ERROR: open path to %s failed!", name.c_str());
            return INT_MIN;
        }

        i2c_master_selector_access(6, 0x7E);
        ifile >> val;
        ifile.close();
        int temp = std::stoi(val) / 1000;
        printf("temp %s = %d\n", name.c_str(), temp);
        return temp;
    }
};

std::optional<std::string> findDevicePath(std::string bus, std::string address)
{
    std::string devicePath;
    std::string hwmonPath;

    try
    {
        // find the device on the bus
        for (const auto & entry : std::experimental::filesystem::directory_iterator("/sys/class/i2c-adapter/i2c-" + bus))
        {
            std::string foundPath = entry.path();
            // check if path ends with device address
            if(std::equal(address.rbegin(), address.rend(), foundPath.rbegin()))
            {
                devicePath = entry.path();
            }
        }
    }
    catch (const std::exception & ex)
    {
        syslog(LOG_WARNING, "ERROR: could not find the bus %s", bus.c_str());
        return std::nullopt;
    }

    if(devicePath.empty())
    {
        syslog(LOG_WARNING, "ERROR: could not find device %s on the bus %s", address.c_str(), bus.c_str());
        return std::nullopt;
    }

    try
    {
        // find hwmon
        for (const auto & entry : std::experimental::filesystem::directory_iterator(devicePath + "/hwmon"))
        {
            hwmonPath = entry.path();
            std::cout << entry.path() << std::endl;
        }
    }
    catch (const std::exception & ex)
    {
        syslog(LOG_WARNING, "ERROR: could not find hwmon for device %s on the bus %s", address.c_str(), bus.c_str());
        return std::nullopt;
    }

    if(hwmonPath.empty())
    {
        syslog(LOG_WARNING, "ERROR: could not find hwmon for device %s on the bus %s", address.c_str(), bus.c_str());
        return std::nullopt;
    }

    return hwmonPath + "/";
}

class FanCtrlZone
{
public:
    FanCtrlZone(std::string name, std::vector<unsigned int> &thresholds, std::vector<Temp> &tempList, 
                std::vector<Fan> &fanList) : name(name), tempList(tempList), fanList(fanList)
    {
        populateFanState(thresholds);
        setInitialState();
    }

    void checkFansSpeed()
    {
        auto fanSpeed = speedTable.find(state);
        if(fanSpeed == speedTable.end())
        {
            syslog(LOG_WARNING, "Wrong state");
            return;
        }

        for (auto &fan : fanList)
        {
            unsigned int currentRPM = fan.getRPM();

            int adjustedRPM = (fan.a * (fanSpeed->second * fanSpeed->second)) + (fan.b * fanSpeed->second) + fan.c;
            int deviation = std::abs((int(currentRPM) - int(adjustedRPM))) * 100 / (adjustedRPM);
            printf("checkFanSpeed: ""%s"" rpm: %d, set percentage: %d, adjusted rpm: %d, deviation: %d\n", fan.name.c_str(), currentRPM, fanSpeed->second, adjustedRPM, deviation);

            if(deviation > MAX_DEVIATION)
            {
                printf("Deviation of ""%s"" speed is %d%%!\n", fan.name.c_str(), deviation);
                syslog(LOG_WARNING, "Deviation of ""%s"" speed is %d%%!", fan.name.c_str(), deviation);
            }
        }
    }

    int getMaxTemp()
    {
        static int errorCnt = 0;
        int max = INT_MIN;

        for (auto &sensor : tempList)
        {
            int temp = sensor.getTemp();
            if(max < temp)
                max = temp;
        }

        if(max == INT_MIN)
        {
            if(errorCnt < 5)
            {
                errorCnt++;
                syslog(LOG_WARNING, "ERROR: temperature readings in zone %s are invalid. Reporting max value", name.c_str());
            }
            return INT_MAX;
        }

        errorCnt = 0;
        printf("MAX temp = %d\n", max); 
        return max;
    }

    void adjustSpeed()
    {
        printf("\nADJUSTING %s\n", name.c_str());
        int currentTemp = getMaxTemp();

        if(currentTemp > upperThreshold)
        {
            printf("Current temperature > %dC. Increase Speed\n", upperThreshold);
            increaseSpeed();
        }
        else if(currentTemp < lowerThreshold)
        {
            printf("Current temperature < %dC. Decrease Speed\n", lowerThreshold);
            decreaseSpeed();
        }
        else
        {
            printf("Current temperature is between %dC and %dC. Do nothing\n", upperThreshold, lowerThreshold);
        }
    }

private:
    std::string name;
    unsigned int state;
    std::map<unsigned int, unsigned int> speedTable;
    std::vector<Temp> tempList;
    std::vector<Fan> fanList;

    void populateFanState(std::vector<unsigned int> &thresholds)
    {
        int i = 0;
        for (auto &threshold : thresholds)
        {
            speedTable[++i] = threshold;
        }
        state = i;
    }

    void setFansSpeed(uint8_t percent)
    {
        for (auto &fan : fanList)
        {
            fan.setPWM(percent);
        }
    }

    void setInitialState()
    {
        auto fanSpeed = speedTable.find(state);
        if(fanSpeed == speedTable.end())
        {
            return;
        }

        setFansSpeed(fanSpeed->second);
    }

    void increaseSpeed()
    {
        printf("Increasing speed from state %d\n", state);
        auto nextSpeed = speedTable.find(state+1);

        if(nextSpeed == speedTable.end())
        {
            printf("Cant increase speed, max state\n");
            return;
        }

        setFansSpeed(nextSpeed->second);
        printf("Increased speed to %d%%\n", nextSpeed->second);
        state++;
        return;
    }

    void decreaseSpeed()
    {
        printf("Decreasing speed from state %d\n", state);
        auto nextSpeed = speedTable.find(state-1);

        if(nextSpeed == speedTable.end())
        {
            printf("Cant decrease speed, min state\n");
            return;
        }

        setFansSpeed(nextSpeed->second);
        printf("Decreased speed to %d%%\n", nextSpeed->second);
        state--;
        return;
    }
};

struct jsonDevice {
std::string name;
std::string bus;
std::string address;
} jsonDevice;

int main()
{
    std::cout << "FAN CTRL\n" << std::endl;
    std::vector<FanCtrlZone> zonesVec;
    std::vector<unsigned int> fanThresholds;

    if(gpio_get(76))
    {
        return 0;
    }

    printf("BMC0. Running fan-control\n");
    syslog(LOG_WARNING, "BMC0. Running fan-control");

    std::ifstream jsonFile("/etc/fan-control-config.json");
    nlohmann::json configFile;
    jsonFile >> configFile;

    // find adjustment period in config file
    int adjPeriod = configFile.value("AdjustmentPeriod", ADJUSTMENT_PERIOD);
    upperThreshold = configFile.value("UpperThreshold", UPPER_THRESHOLD);
    lowerThreshold = configFile.value("LowerThreshold", LOWER_THRESHOLD);

    // find thresholds in config file
    auto const thresholds = configFile.find("FanSpeedStates");
    if (thresholds == configFile.end())
    {
        std::cerr << "Thresholds Not Found";
        return 1;
    }
    else
    {
        for (auto threshold : *thresholds)
        {
            fanThresholds.push_back(threshold);
        }

        if(fanThresholds.empty())
        {
            std::cerr << "Thresholds not found";
            return 1;
        }
    }

    // find zones in config file
    auto const zones = configFile.find("Zones");
    if (zones == configFile.end())
    {
        std::cerr << "Zones Not Found";
        return 1;
    }

    int i = 1;
    int fanNum = 1;
    int tempNum = 1;
    for (const nlohmann::json& zone : *zones)
    {
        std::vector<struct::jsonDevice> fansVec;
        std::vector<struct::jsonDevice> tempVec;
        std::vector<Fan> fanList;
        std::vector<Temp> tempList;

        auto const fans = zone.find("fans");
        if (fans == zone.end())
        {
            std::cerr << "Fans Not Found";
            return 1;
        }
        else
        {
            for (auto it : *fans)
            {
                fansVec.push_back({"fan" + std::to_string(fanNum), it[0], it[1]});
                fanNum++;
            }
        }

        auto const temp = zone.find("temp");
        if (temp == zone.end())
        {
            std::cerr << "Temperature sensors Not Found";
            return 1;
        }
        else
        {
            for (auto it : *temp)
            {
                tempVec.push_back({"temp" + std::to_string(tempNum), it[0], it[1]});
                tempNum++;
            }
        }

        for (auto vec : fansVec)
        {
            float a, b, c;
            if(vec.address.compare("18") == 0)
            {
                a = -1.9469;
                b = 475.4;
                c = -1829.1;
            }
            else if(vec.address.compare("19") == 0)
            {
                a = -0.8753;
                b = 395;
                c = -498.75;
            }
            else if(vec.address.compare("1a") == 0)
            {
                a = -0.9237;
                b = 398.58;
                c = -628.9;
            }
            else if(vec.address.compare("2c") == 0)
            {
                a = -0.8487;
                b = 389.09;
                c = -413.5;
            }
            else
            {
                a = 0;
                b = 0;
                c = 0;
            }


            std::cout << vec.name << " " << vec.bus << " " << vec.address << " " << a << " " << b << " " << c << '\n';
            std::optional<std::string> path = findDevicePath(vec.bus, vec.address);
            if(path)
                fanList.push_back(Fan(vec.name, *path, a, b, c));
        }
        for (auto vec : tempVec)
        {
            std::cout << vec.name << " " << vec.bus << " " << vec.address  << '\n';
            std::optional<std::string> path = findDevicePath(vec.bus, vec.address);
            if(path)
                tempList.push_back(Temp(vec.name, *path));
        }

        std::string zoneName = "zone" + std::to_string(i++);
        std::cout << zoneName << '\n';
        if(fanList.empty())
        {
            printf("No fans in %s\n", zoneName.c_str());
            continue;
        }
        if(tempList.empty())
        {
            printf("No temp sensors in %s\n", zoneName.c_str());
        }

        zonesVec.push_back(FanCtrlZone(zoneName, fanThresholds, tempList, fanList));
    }

    // Control algorythm
    while(1)
    {
        printf("\n\n\n\n");
        sleep(5);
        printf("\nfan readings:\n");
        for (auto &zone : zonesVec)
        {
            zone.checkFansSpeed();
        }

        for (auto &zone : zonesVec)
        {
            zone.adjustSpeed();
        }
        sleep(adjPeriod - 5);
    }

    return 0;
}