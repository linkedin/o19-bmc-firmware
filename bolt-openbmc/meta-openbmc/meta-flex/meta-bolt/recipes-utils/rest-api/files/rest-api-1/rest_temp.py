#!/usr/bin/env python3

import rest_sensors

def get_temp():
    temp_result = []
    sensors = rest_sensors.get_sensors()
    result = sensors["Information"]
    i=0
    for temp_sensor in result:
        i+=1
        for info in temp_sensor.keys():
                if("Temp" in info):
                        temp_data = {}
                        temp_data["TemperatureSensorID"] = i
                        temp_data["Name"] = info
                        components = info.split(' ')
                        str = " "
                        position = str.join(components[:-1])
                        temp_data["Position"] = position
                        temperature = temp_sensor[info].split(" ")[0]
                        temp_data["Temperature"] = temperature
                        if(float(temperature)>0):
                                temp_data["OperationalStatus"] = "UP"
                        else:
                                temp_data["OperationalStatus"] = "DOWM"
                        temp_result.append(temp_data)

    fresult = {
                "TemperatureState": temp_result

              }

    return fresult


if(__name__=="__main__"):
    print('{}'.format(get_temp()))

