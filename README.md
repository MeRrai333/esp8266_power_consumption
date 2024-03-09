# ESP8266 power consumption meter
This is mini project for IoT subject <br/>
Code in ESP8266_POWER_CONSUMTION written in platformIO <br/>
file iot_group_8.json is dashboard file for InfluxDB, InfluxDB can import this file as a dashboard
<br/>
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
<h3>How to use</h3>
<ul>
  <li>Power on, if esp8266 can't connected wifi will enter setup mode and OLED display "Setup WiFi"</li>
  <li>Setup mode esp8266 will be WiFi ap mode ssid default name is "IoT_G8"</li>
  <li>After connect IoT_G8 wifi will redriect to WiFi manager page as image WiFi manager configuration below</li>
  <li>Enter SSID password InfluxDB server config line token and click save</li>
  <li>ESP8266 will reconnect wifi if wifi is connected will notify to line and try to connect influxDB server if influxDB server is connected OLED will display value from PZEM-004T that mean it working</li>
  <li>In working mode OLED will display value from PZEM-004T and every 5 sec. will change value dispaly if press button will immediately change value</li>
  <li>If want to change configuration just pressed button before power on, OLED will display "Reset WiFi" after release esp8266 will enter setup mode</li>
  <li>During working if WiFi or InfluxDB is lost OLED will dispaly "WiFi/IFDB Lost Reconn." and automatic reconnect</li>
</ul>
<br/>
<h3>Circuit diagram</h3>
<img width="800px" src="https://github.com/MeRrai333/esp8266_power_consumption/blob/85b848b1c0ccac09a3339cbfdc387db82868f0eb/image/IoT_Mini_Project_Circuit.jpg?raw=true"/>
<br/>
<h3>State diagram</h3>
<img width="800px" src="https://github.com/MeRrai333/esp8266_power_consumption/blob/master/image/IoT_Mini_Project_State.jpg?raw=true"/>
<br/>
<h3>Dashboard</h3>
<img width="700px" src="https://github.com/MeRrai333/esp8266_power_consumption/blob/master/image/dash1.png?raw=true"/>
<br/>
<img width="700px" src="https://github.com/MeRrai333/esp8266_power_consumption/blob/master/image/dash2.png?raw=true"/>
<br/>
<h3>Line notify</h3>
<img height="500px" src="https://github.com/MeRrai333/esp8266_power_consumption/blob/master/image/line_notify.jpg?raw=true">
<br/>
<h3>WiFi manager configuration</h3>
<div>
  <img height="600px" src="https://github.com/MeRrai333/esp8266_power_consumption/blob/42b229a37f9358253196c48cdcca5c32ae4ecad9/image/wifimanager1.jpg?raw=true">
  <img height="600px" src="https://github.com/MeRrai333/esp8266_power_consumption/blob/42b229a37f9358253196c48cdcca5c32ae4ecad9/image/wifimanager2.jpg?raw=true">
  <img height="600px" src="https://github.com/MeRrai333/esp8266_power_consumption/blob/42b229a37f9358253196c48cdcca5c32ae4ecad9/image/wifimanager3.jpg?raw=true">
</div>
