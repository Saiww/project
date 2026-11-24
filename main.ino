#include <Wire.h>
#include <RTClib.h>
#include <LowPower.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <LiquidCrystal_I2C.h>

// ===== LCD SETUP =====
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ===== PIN CONFIG =====
#define MQ2_PIN A0
#define BATTERY_PIN A1
#define DHT_PIN 2
#define SENSOR_POWER 7
#define LED_PIN 8

#define DHT_TYPE DHT22

// ===== OBJECTS =====
RTC_DS3231 rtc;
DHT dht(DHT_PIN, DHT_TYPE);

// ===== THRESHOLDS =====
#define GAS_THRESHOLD 350
#define BATTERY_LOW 3.7

// ===== VARIABLES =====
float temperature;
float humidity;
int gasValue;
float batteryVoltage;

bool gasAlert = false;
bool batteryAlert = false;

// ===== BATTERY FUNCTION =====
float readBatteryVoltage()
{
  int adcValue = analogRead(BATTERY_PIN);
  float voltage = (adcValue * 5.0) / 1023.0;
  return voltage * 2;
}

// ===== SENSOR READ =====
void readSensors()
{
  digitalWrite(SENSOR_POWER, HIGH);
  digitalWrite(LED_PIN, HIGH);

  Serial.println("Sensors ON");
  delay(3000);

  // MQ-2 analog reading
  gasValue = analogRead(MQ2_PIN);

  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  batteryVoltage = readBatteryVoltage();

  digitalWrite(SENSOR_POWER, LOW);
  digitalWrite(LED_PIN, LOW);

  Serial.println("Sensors OFF");
}

// ===== PROCESS + DISPLAY =====
void processData()
{
  DateTime now = rtc.now();

  gasAlert = (gasValue > GAS_THRESHOLD);
  batteryAlert = (batteryVoltage < BATTERY_LOW);

  lcd.clear();

  if (gasAlert)
  {
    lcd.setCursor(0, 0);
    lcd.print("GAS ALERT ");

    lcd.setCursor(0, 1);
    lcd.print("Lvl:");
    lcd.print(gasValue);

    Serial.println("WARNING: Gas Detected!");
  }
  else if (batteryAlert)
  {
    lcd.setCursor(0, 0);
    lcd.print("LOW BATTERY!");

    lcd.setCursor(0, 1);
    lcd.print("V:");
    lcd.print(batteryVoltage, 1);

    Serial.println("WARNING: Battery Low!");
  }
  else
  {
    lcd.setCursor(0, 0);
    lcd.print(now.hour());
    lcd.print(":");
    lcd.print(now.minute());

    lcd.setCursor(0, 1);
    lcd.print("G:");
    lcd.print(gasValue);
    lcd.print(" T:");
    lcd.print(temperature, 1);
  }

  Serial.println("----- DATA -----");

  Serial.print("Time: ");
  Serial.print(now.hour());
  Serial.print(":");
  Serial.print(now.minute());
  Serial.print(":");
  Serial.println(now.second());

  Serial.print("Gas: ");
  Serial.println(gasValue);

  Serial.print("Temp: ");
  Serial.println(temperature);

  Serial.print("Humidity: ");
  Serial.println(humidity);

  Serial.print("Battery: ");
  Serial.println(batteryVoltage);

  Serial.println("----------------");
}

// ===== SLEEP FUNCTION =====
void enterSleep()
{
  Serial.println("Sleeping...");

  digitalWrite(LED_PIN, HIGH);
  delay(300);
  digitalWrite(LED_PIN, LOW);

  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  delay(2000);

  Serial.println("Wakeup!");
}

// ===== SETUP =====
void setup()
{
  Serial.begin(9600);

  Wire.begin();

  lcd.init();
  lcd.backlight();

  dht.begin();

  if (!rtc.begin())
  {
    Serial.println("RTC not found!");
    while (1);
  }

  if (rtc.lostPower())
  {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  pinMode(SENSOR_POWER, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(SENSOR_POWER, LOW);
  digitalWrite(LED_PIN, LOW);

  lcd.setCursor(0, 0);
  lcd.print("System Starting");
  delay(2000);
  lcd.clear();

  Serial.println("System Started");
}

// ===== LOOP =====
void loop()
{
  readSensors();
  processData();
  enterSleep();
}
