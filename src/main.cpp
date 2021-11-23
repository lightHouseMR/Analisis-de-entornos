#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ESP32Encoder.h>
#include <WiFi.h>
#include <WebServer.h>

U8G2_PCD8544_84X48_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/17, /* dc=*/5, /* reset=*/16);
DHT dht(14, DHT11);
ESP32Encoder encoder;
WebServer server(80);

const char *SSID = "Macaco";
const char *PWD = "98765432";
const int trigPin = 12;
const int echoPin = 13;

int lSensores = 0; //Ultimo tiempo de lectura de los sensores
int lBotones = 0;  //Ultima lectura de los botones
bool pBoton1 = 0;  //Ultimo estado del boton
bool boton1 = 0;   //Ultimo esta el boton presionado

String ip = "No conectado";

bool pantalla = true; //Estado de la pantalla
int scene = 0;        //Escena actual

float humedad;
float temperatura;
float distancia;
int humo;
bool ruido;
bool lumin;

float leerDistancia()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  float duration = pulseIn(echoPin, HIGH);
  if (duration > 30000)
  {
    return distancia;
  }
  return (duration * .0343) / 2;
}

void dispositivo(void *parameter)
{
  dht.begin();
  encoder.attachHalfQuad(26, 25);
  encoder.clearCount();
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(35, INPUT);
  pinMode(33, INPUT_PULLDOWN);
  pinMode(21, OUTPUT);
  pinMode(27, INPUT_PULLUP);
  pinMode(14, INPUT_PULLDOWN);
  u8g2.begin();
  for (;;)
  {
    //Leer cada segundo todos los sensores
    if (millis() > lSensores + 1000)
    {
      lSensores = millis();
      humedad = dht.readHumidity();
      temperatura = dht.readTemperature();
      //distancia = leerDistancia();
      humo = analogRead(35);
      //ruido = digitalRead(33);
      //lumin = digitalRead(14);
    }

    //Leer cada 50ms todos los botones
    if (millis() > lBotones + 50)
    {
      lBotones = millis();
      if (pBoton1 == 1 && digitalRead(27) == 0)
      {
        boton1 = true;
      }
      pBoton1 = digitalRead(27);
    }

    //Actualizar pantalla
    u8g2.clearBuffer();
    switch (scene)
    {
    case 0:
      u8g2.setFont(u8g2_font_luIS14_te);
      u8g2.setFontMode(0); // enable transparent mode, which is faster
      u8g2.setCursor(0, 15);
      u8g2.print("Proyecto");
      u8g2.setCursor(0, 30);
      u8g2.print("Final");
      u8g2.setCursor(0, 45);
      u8g2.print("Materiales");
      u8g2.sendBuffer();
      vTaskDelay(5000 / portTICK_PERIOD_MS);
      encoder.clearCount();
      scene = 1;
      break;
    case 1: //Menu principal
      u8g2.setFont(u8g2_font_6x10_tf);
      if (encoder.getCount() / 2 % 3 == 0)
      {
        u8g2.drawButtonUTF8(42, 10, U8G2_BTN_INV | U8G2_BTN_HCENTER | U8G2_BTN_BW1, 0, 2, 2, "Medir");
        u8g2.drawButtonUTF8(42, 26, U8G2_BTN_HCENTER | U8G2_BTN_BW1, 0, 2, 2, "Crear sala");
        u8g2.drawButtonUTF8(42, 42, U8G2_BTN_HCENTER | U8G2_BTN_BW1, 0, 2, 2, "Ajustes");
      }
      else if (encoder.getCount() / 2 % 3 == 1)
      {
        u8g2.drawButtonUTF8(42, 10, U8G2_BTN_HCENTER | U8G2_BTN_BW1, 0, 2, 2, "Medir");
        u8g2.drawButtonUTF8(42, 26, U8G2_BTN_INV | U8G2_BTN_HCENTER | U8G2_BTN_BW1, 0, 2, 2, "Crear sala");
        u8g2.drawButtonUTF8(42, 42, U8G2_BTN_HCENTER | U8G2_BTN_BW1, 0, 2, 2, "Ajustes");
      }
      else
      {
        u8g2.drawButtonUTF8(42, 10, U8G2_BTN_HCENTER | U8G2_BTN_BW1, 0, 2, 2, "Medir");
        u8g2.drawButtonUTF8(42, 26, U8G2_BTN_HCENTER | U8G2_BTN_BW1, 0, 2, 2, "Crear sala");
        u8g2.drawButtonUTF8(42, 42, U8G2_BTN_INV | U8G2_BTN_HCENTER | U8G2_BTN_BW1, 0, 2, 2, "Ajustes");
      }
      if (boton1)
      {
        int next = encoder.getCount() / 2 % 3;
        encoder.clearCount();
        if (next == 0)
        {
          scene = 2;
        }
        else if (next == 1)
        {
          scene = 3;
        }
        else
        {
          scene = 4;
        }
        boton1 = false;
      }
      u8g2.sendBuffer();
      break;
    case 2: //Medir
      u8g2.setCursor(0, 7);
      u8g2.print("Humo: ");
      u8g2.print(humo);
      u8g2.print("%");
      u8g2.setCursor(0, 15);
      u8g2.print("Temp: ");
      u8g2.print(temperatura);
      u8g2.print("C");
      u8g2.setCursor(0, 23);
      u8g2.print("Humd: ");
      u8g2.print(humedad);
      u8g2.print("cm");
      u8g2.setCursor(0, 31);
      u8g2.print("Dist: ");
      u8g2.print(distancia);
      u8g2.print("%");
      u8g2.setCursor(0, 39);
      u8g2.print("Lumin:");
      u8g2.print(lumin);
      u8g2.setCursor(0, 47);
      u8g2.print("Ruido: ");
      u8g2.print(ruido);
      u8g2.sendBuffer();
      if (boton1)
      {
        pantalla = !pantalla;
        digitalWrite(21, pantalla);
        boton1 = false;
      }
      break;
    case 3: //Crear Sala
      u8g2.setCursor(0, 7);
      u8g2.print("CrearSala");
      u8g2.sendBuffer();
      break;
    case 4: //Ajustes
      u8g2.setCursor(0, 7);
      u8g2.print("Red: ");
      u8g2.print(SSID);
      u8g2.setCursor(0, 15);
      u8g2.print("Psw: ");
      u8g2.print(PWD);
      u8g2.setCursor(0, 23);
      u8g2.print("IP: ");
      u8g2.print(ip);
      u8g2.sendBuffer();
    default:
      break;
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void conectarWifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PWD);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  ip = WiFi.localIP().toString();
}

void conectividad(void *parameter)
{
  conectarWifi();
  for (;;)
  {
    Serial.print("Funciona");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup(void)
{
  Serial.begin(9600);
  xTaskCreatePinnedToCore(
      dispositivo,   // Function that should be called
      "Dispositivo", // Name of the task (for debugging)
      1000,          // Stack size (bytes)
      NULL,          // Parameter to pass
      1,             // Task priority
      NULL,          // Task handle
      0              // Core you want to run the task on (0 or 1)
  );
  xTaskCreatePinnedToCore(
      conectividad,   // Function that should be called
      "Conectividad", // Name of the task (for debugging)
      6000,           // Stack size (bytes)
      NULL,           // Parameter to pass
      1,              // Task priority
      NULL,           // Task handle
      1               // Core you want to run the task on (0 or 1)
  );
}

void loop(void)
{
}
