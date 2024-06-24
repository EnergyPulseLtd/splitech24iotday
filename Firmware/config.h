// Define WiFi SSID (access point name) and password
#define WIFI1_SSID "IoTWorkshop"
#define WIFI1_PASS "SpliTech2024"

// Define InfluxDB connection parameters.
#define INFLUXDB_URL "http://192.168.0.10:8086"
// Here put the name of the database that will store the recorded data.
#define INFLUXDB_NAME "rwd_demo"
// Here put the username and password for the database.
#define INFLUXDB_USER "grafana"
#define INFLUXDB_PASS "grafana"

// Define the tags for sensor data point.
#define INFLUXDB_TAG_DEVICE "esp32iottest"
#define INFLUXDB_TAG_SENSOR_LIST "aht20,bmp280"

// Specify the desired delay between each data point in milliseconds.
#define DELAY_BEFORE 1000
