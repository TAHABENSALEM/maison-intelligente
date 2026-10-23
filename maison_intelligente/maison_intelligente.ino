// Maison intelligente — NodeMCU ESP8266
// Serveur web embarqué : suivi de la température, de l'humidité et du gaz,
// alarmes locales (LED et buzzer) et commande de deux lampes via Wi-Fi.

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "DHT.h"

#include "secrets.h"   // WIFI_SSID et WIFI_PASSWORD (copier secrets.example.h)
#include "PageIndex.h" // page web servie sur "/"

// ---------------------------------------- Broches (numéros GPIO)
#define LED1_PIN      5   // D1 : lampe 1 (active à l'état bas)
#define LED2_PIN      4   // D2 : lampe 2 (active à l'état bas)
#define TEMP_LED_PIN 12   // D6 : LED d'alerte température
#define BUZZER_PIN    0   // D3 : buzzer d'alarme gaz
#define DHT_PIN      13   // D7 : capteur DHT11
#define MQ2_PIN      A0   // A0 : capteur de gaz MQ2 (analogique)
#define LED_ON_BOARD  2   // LED de la carte, clignote pendant la connexion Wi-Fi

// ---------------------------------------- Seuils
const int   GAS_THRESHOLD    = 600;   // valeur analogique MQ2 (0 à 1023)
const float TEMP_ALERT_C     = 30.0;  // °C
const unsigned long READ_INTERVAL_MS = 2000; // le DHT11 ne supporte pas plus d'une lecture par seconde

#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);
ESP8266WebServer server(80);

// Dernières mesures, mises à jour dans loop() pour que les alarmes
// fonctionnent même quand aucune page web n'est ouverte.
float temperature = NAN;
float humidity = NAN;
int gasValue = 0;
bool gasAlarm = false;
bool tempAlarm = false;
bool led1On = false;
bool led2On = false;
unsigned long lastRead = 0;

void readSensors() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t)) temperature = t;
  if (!isnan(h)) humidity = h;
  if (isnan(t) || isnan(h)) Serial.println("Lecture DHT11 impossible");

  gasValue = analogRead(MQ2_PIN);
  gasAlarm = gasValue > GAS_THRESHOLD;
  tempAlarm = !isnan(temperature) && temperature > TEMP_ALERT_C;

  digitalWrite(BUZZER_PIN, gasAlarm ? HIGH : LOW);
  digitalWrite(TEMP_LED_PIN, tempAlarm ? HIGH : LOW);

  Serial.printf("T=%.1f C  H=%.1f %%  Gaz=%d  alarme gaz=%d  alerte temp=%d\n",
                temperature, humidity, gasValue, gasAlarm, tempAlarm);
}

void setLamp(int pin, bool &state, bool on) {
  state = on;
  digitalWrite(pin, on ? LOW : HIGH); // LEDs câblées en logique inversée
}

// ---------------------------------------- Routes HTTP
void handleRoot() {
  server.send_P(200, "text/html", MAIN_page);
}

// Toutes les mesures en une seule réponse JSON
void handleData() {
  String json = "{";
  json += "\"temperature\":" + (isnan(temperature) ? String("null") : String(temperature, 1)) + ",";
  json += "\"humidity\":" + (isnan(humidity) ? String("null") : String(humidity, 1)) + ",";
  json += "\"gas\":" + String(gasValue) + ",";
  json += "\"gasThreshold\":" + String(GAS_THRESHOLD) + ",";
  json += "\"gasAlarm\":" + String(gasAlarm ? "true" : "false") + ",";
  json += "\"tempAlarm\":" + String(tempAlarm ? "true" : "false") + ",";
  json += "\"led1\":" + String(led1On ? "true" : "false") + ",";
  json += "\"led2\":" + String(led2On ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

// /led1?state=on|off et /led2?state=on|off
void handleLamp(int pin, bool &state, const char *name) {
  String s = server.arg("state");
  if (s != "on" && s != "off") {
    server.send(400, "text/plain", "Paramètre state attendu : on ou off");
    return;
  }
  setLamp(pin, state, s == "on");
  server.send(200, "text/plain", String(name) + " " + s);
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(TEMP_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_ON_BOARD, OUTPUT);
  setLamp(LED1_PIN, led1On, false);
  setLamp(LED2_PIN, led2On, false);
  digitalWrite(TEMP_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // Connexion Wi-Fi (la LED de la carte clignote pendant l'attente)
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connexion à ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(LED_ON_BOARD, LOW);
    delay(250);
    digitalWrite(LED_ON_BOARD, HIGH);
    delay(250);
  }
  Serial.println();
  Serial.print("Connecté, adresse IP : ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/led1", []() { handleLamp(LED1_PIN, led1On, "LED1"); });
  server.on("/led2", []() { handleLamp(LED2_PIN, led2On, "LED2"); });
  server.begin();
  Serial.println("Serveur HTTP démarré");

  readSensors();
}

void loop() {
  server.handleClient();

  if (millis() - lastRead >= READ_INTERVAL_MS) {
    lastRead = millis();
    readSensors();
  }
}
