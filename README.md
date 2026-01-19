Proiect: ESP32 + PIR + MQTT + LCD 1602A

1. Motivatie
Senzorul numara de cate ori pisica a facut la litiera cat timp eu sunt plecat la munca, facultate.

2. Descriere proiect

Acest proiect utilizează un modul ESP32 pentru a detecta mișcarea folosind un senzor PIR (Passive Infrared). La fiecare activare a senzorului:

- se incrementează un counter

- counter-ul este salvat în memoria flash (Preferences)

- counter-ul este afișat pe un LCD 1602A (mod paralel 4-bit)
   -LCD-ul cumparat nu se aprinde. Am imprumutat un LCD de la o prietena si am reusit sa il fac sa functioneze one time :)

- se publică un mesaj MQTT către un broker (ex. test.mosquitto.org)

- există un interval de rearmare de 5 secunde in modul test/debug si in usermode intervalul este de 5 minute + activare de 2 ori pentru a se lua in calcul, în care senzorul este ignorat

- La fiecare restart al dispozitivului, counter-ul este resetat la 0 (conform cerinței).

3. Placa utilizată: ESP WROOM 32
Descriere
ESP32 este un microcontroller foarte popular, cu:
-Wi-Fi
-Bluetooth
-procesor dual-core
-consum redus
-numeroase GPIO-uri

4. Senzorul PIR (HC-SR501)
Descriere

Senzorul PIR detectează mișcarea prin schimbarea radiației infraroșii în câmpul său de detecție. Este foarte folosit în proiecte de securitate și automatizări.

Caracteristici:
funcționează pe 3.3V
ieșire digitală (HIGH/LOW)
stabilizare la pornire (aprox. 30-60 sec) pana acesta este calibrat
sensibilitate și timp de trigger ajustabile (potențiometre pe modul)

5. LCD 1602A (mod paralel 4-bit)
LCD-ul afișează counter-ul de activări. În proiect este folosit în modul paralel 4-bit, fără modul I2C.

6. Schema electrică (conexiuni)
6.1 Conexiuni PIR
VCC	3.3V
GND	GND
OUT	GPIO 16
6.2 Conexiuni LCD 1602A (4-bit)
VSS	GND
VDD	5V
V0	GND
RS	GPIO 23
RW	GND
E	GPIO 18
D4	GPIO 26
D5	GPIO 17
D6	GPIO 25
D7	GPIO 5
A	5V
K	GND
7. Funcționare
7.1 Logica proiect

Când PIR detectează mișcare (LOW → HIGH), counter-ul crește cu 1
În următoarele 5 secunde, senzorul este ignorat
Counter-ul este salvat în flash și afișat pe LCD
Mesajul este publicat pe MQTT

7.2 Mesaj MQTT

Topic:

devices/esp32/pir/raw

#define WIFI_SSID     "UPB_Guest"
#define WIFI_PASSWORD ""


8. Testare MQTT

În terminal:

mosquitto_sub -h test.mosquitto.org -t devices/esp32/pir/raw -v

9. Observații și recomandări

PIR-ul are nevoie de 30–60 sec după alimentare pentru stabilizare.
Dacă LCD nu afișează nimic, ajustează contrastul (V0).
