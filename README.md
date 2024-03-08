# ESP8266 power consumption meter
This is mini project for IoT subject <br/><br/>
<h3>Feature</h3>
<ul>
  <li>measure AC Voltage, Current, Active power, Active energy, Frequency and Power factor</li>
  <li>Voltage range: 80-260V (from pzem-004t datasheet)</li>
  <li>Current: 0-100A (from pzem-004t datasheet)</li>
  <li>Power: 0-23kW (from pzem-004t datasheet)</li>
  <li>Power Factor: 0-1 (from pzem-004t datasheet)</li>
  <li>Frequency: 45-65Hz (from pzem-004t datasheet)</li>
  <li>Active Energy: 0-9,999.99kWh (from pzem-004t datasheet) can reset</li>
  <li>Dashboard and database using InfluxDB</li>
  <li>Line notify interval time</li>
  <li>Change WiFi InfluxDB server and line token in setup mode using WiFi manager</li>
</ul>
<br/>
<h3>Circuit diagram</h3>
![alt text](/image/IoT_Mini_Project_Circuit.jpg)
![alt text](https://github.com/MeRrai333/esp8266_power_consumption/blob/master/image/IoT_Mini_Project_Circuit.jpg?raw=true)
<br/>
<h3>State diagram</h3>
![alt text](https://github.com/MeRrai333/esp8266_power_consumption/blob/master/image/IoT_Mini_Project_State.jpg?raw=true)
<br/>
<h3>Dashboard</h3>
![alt text](https://github.com/MeRrai333/esp8266_power_consumption/blob/master/image/dash1.png?raw=true)
![alt text](https://github.com/MeRrai333/esp8266_power_consumption/blob/master/image/dash2.png?raw=true)
<br/>
<h3>Line notify</h3>
![alt text](https://github.com/MeRrai333/esp8266_power_consumption/blob/master/image/line_notify.jpg?raw=true)
<br/>

