#!/usr/bin/env python3

import rest_sensors

def get_fan():
    fan_result = []
    sensors = rest_sensors.get_sensors()
    result = sensors["Information"]
    i=0
    for fan_sensor in result:
        i+=1
        for info, rpms in fan_sensor.items():
            if("Fan" in info):
                fan_data = {}
                id = info.split("Fan ")[1].split(" ")[0]
                position = info.split(':')[0].split()[2]
                if(" RPM" in rpms):
                    speed = rpms.split(" RPM")[0]
                    speed = int(speed)
                    if (speed>0):
                        fan_data["OperationalState"] = "UP"
                        fan_data["Status"] = "Present"
                    else:
                        fan_data["OperationalState"] = "DOWN"
                        fan_data["Status"] = "Absent"
#                    print("Id: %s, Position: %s, Speed %d" % (id, position, speed))

                fan_data["FanID"] = id
                fan_data["Position"] = position
                fan_data["OperationalSpeed"] = speed
                fan_result.append(fan_data)

    fresult = {
                "FanState": fan_result

              }

    return fresult


if(__name__=="__main__"):
    print('{}'.format(get_fan()))

