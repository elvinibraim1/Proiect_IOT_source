Proiect: ESP32 + PIR + MQTT + LCD 1602A
1. Descriere proiect

Acest proiect utilizează un modul ESP32 pentru a detecta mișcarea folosind un senzor PIR (Passive Infrared). La fiecare activare a senzorului:

se incrementează un counter

counter-ul este salvat în memoria flash (Preferences)

counter-ul este afișat pe un LCD 1602A (mod paralel 4-bit)

se publică un mesaj MQTT către un broker (ex. test.mosquitto.org)

există un interval de rearmare de 5 secunde, în care senzorul este ignorat

La fiecare restart al dispozitivului, counter-ul este resetat la 0 (conform cerinței).

2. Placa utilizată: ESP32 (ESP32 DevKit v1)
Descriere

ESP32 este un microcontroller foarte popular, cu:

Wi-Fi

Bluetooth

procesor dual-core

consum redus

numeroase GPIO-uri

Placa folosită în proiect este ESP32 DevKit v1, care oferă:

conector USB pentru programare

pini de alimentare și GPIO

compatibilitate cu Arduino IDE și PlatformIO

3. Senzorul PIR (HC-SR501)
Descriere

Senzorul PIR detectează mișcarea prin schimbarea radiației infraroșii în câmpul său de detecție. Este foarte folosit în proiecte de securitate și automatizări.

Caracteristici:

funcționează pe 5V

ieșire digitală (HIGH/LOW)

stabilizare la pornire (aprox. 30-60 sec)

sensibilitate și timp de trigger ajustabile (potențiometre pe modul)

4. LCD 1602A (mod paralel 4-bit)

LCD-ul afișează counter-ul de activări. În proiect este folosit în modul paralel 4-bit, fără modul I2C.

5. Schema electrică (conexiuni)
5.1 Conexiuni PIR
Pin PIR	ESP32
VCC	5V
GND	GND
OUT	GPIO 27
5.2 Conexiuni LCD 1602A (4-bit)
Pin LCD	ESP32
VSS	GND
VDD	5V
V0	Potențiometru 10k (5V–GND)
RS	GPIO 21
RW	GND
E	GPIO 22
D4	GPIO 18
D5	GPIO 19
D6	GPIO 23
D7	GPIO 5
A	5V (prin rezistor 220Ω)
K	GND
6. Funcționare
6.1 Logic proiect

Când PIR detectează mișcare (LOW → HIGH), counter-ul crește cu 1

În următoarele 5 secunde, senzorul este ignorat

Counter-ul este salvat în flash și afișat pe LCD

Mesajul este publicat pe MQTT

6.2 Mesaj MQTT

Topic:

devices/esp32/pir/raw


#define WIFI_SSID     "DIGI_HT7A"
#define WIFI_PASSWORD "sxFxK9gVk4"

7.2 Upload cod

Deschide proiectul în VS Code

Selectează PlatformIO

Click Build

Click Upload

8. Testare MQTT

În terminal:

mosquitto_sub -h test.mosquitto.org -t devices/esp32/pir/raw -v

9. Observații și recomandări

PIR-ul are nevoie de 30–60 sec după alimentare pentru stabilizare.

Dacă LCD nu afișează nimic, ajustează contrastul (V0).

Dacă ai alte părți ale proiectului (ex. butoane, alte senzori), pot fi adăugate fără probleme.