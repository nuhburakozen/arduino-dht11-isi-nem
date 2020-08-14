
#include <EtherCard.h>
#include "DHT.h"
#include <LiquidCrystal.h>

// Ardunio'ya sensörümüzün hangi pin'e bağlı olduğunu bildiriyoruz
#define DHTPIN 2
// Ardunio'ya kullandığımız Sensörün tipini belirtiyoruz
#define DHTTYPE DHT11   // DHT 11 sensörü bağlı
DHT sensor(DHTPIN, DHTTYPE);
// Ardunio'ya röle modülünün hangi pin'e bağlı olduğunu bildiriyoruz
#define RELAY  A0
// Ardunio'ya buzzer'ın hangi pin'e bağlı olduğunu bildiriyoruz
const int buzzerPin = 9;
// Ardunio'ya LCD ekranımızın hangi pin'lere bağlı olduğunu bildiriyoruz
// LCD RS=3, EN=4, DS4=5, DS5=6, DS6=7, DS7=8
LiquidCrystal lcd(3, 4, 5, 6, 7, 8);
// ENC28J60 Ethernet modülümüzün bilgilerini Ardunio'ya tanımlıyoruz
static byte mymac[] = {0x74, 0x69, 0x69, 0x2D, 0x30, 0x31};
static byte myip[] = {10, 34, 110, 204};
static byte gwip[] = {10, 34, 110, 1};
static byte subnet[] = {255, 255, 254, 0};
byte Ethernet::buffer[500];
BufferFiller bfill;

int t = 0;
int n = 0;
int sure = 0;

// Ethernet kartını, DHT11 sensörünü ve LCD ekranı canlı hale getiriyoruz
void setup() {
    Serial.begin(9600);
    if (ether.begin(sizeof Ethernet::buffer, mymac, 10) == 0)
        Serial.println("Ethernet denetleyecisine erişilemedi");
    ether.staticSetup(myip, gwip, subnet);
    lcd.begin(16, 2);
    sensor.begin();
    pinMode(buzzerPin, OUTPUT);
    pinMode(RELAY, OUTPUT);
}

//  Bu fonksiyon DHT11'den gelen verileri alıyoruz
static void ReadDHT11() {
    n = sensor.readHumidity();
    t = sensor.readTemperature();
}

//  Burada bir web sayfası oluşturup t ve n değerlerini sayfamıza yazdırıyoruz
static word websayfam() {
    bfill = ether.tcpOffset();
    bfill.emit_p(PSTR(
            "<!DOCTYPE html>"
            "<html xmlns='http://www.w3.org/1999/xhtml'>"
            "<head><meta charset='UTF-8'>"
            "<title>Sıcaklık ve Nem Raporu</title></head>"
            "<body style=background-color:#d3d3d3;margin-left:auto;margin-right:auto;text-align:center;>"
            "<p><img src='https://hisarhospital.com/wp-content/uploads/2018/08/Hisar_Hospital_Logo_2.png';width=250px; height=120; /></p>"
            "<h1 style=color:#EE6600;>Sıcaklık ve Nem Raporu</h1>"
            "<table style=margin-left:auto;margin-right:auto;text-align:center;color:#060e9f;font-size:1.9rem;><tr><td>Sıcaklık</td><td>&nbsp Nem</td></tr>"
            "<tr><td>$D &#8451</td><td>&nbsp % $D</td></tr></table></body>"), t, n);
    return bfill.position();
}

void loop() {

    sure = sure + 1;
    if (sure == 1) {
        ReadDHT11();
        lcd.clear();
        // LCD ekranın üst kısmına sıcaklık değerini yazdırıyoruz
        lcd.setCursor(0, 0);
        lcd.print("Sicaklik : ");
        lcd.print(t);
        lcd.print(" C");
        // LCD ekranın alt kısmına nem değerini yazdırıyoruz
        lcd.setCursor(0, 1);
        lcd.print("Nem Orani: ");
        lcd.print(n);
        lcd.print(" %");
    }
    if (sure == 1100) {
        sure = 0;
    }

    if (t >= 30) {
        digitalWrite(buzzerPin, HIGH);
        delay(1000);
        digitalWrite(buzzerPin, LOW);
        delay(1000);
        digitalWrite(RELAY, HIGH);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Sicaklik Ust");
        lcd.setCursor(0, 1);
        lcd.print("Siniri Asildi...");
        delay(1000);
        lcd.clear();
        // LCD ekranın üst kısmına sıcaklık değerini yazdırıyoruz
        lcd.setCursor(0, 0);
        lcd.print("Sicaklik : ");
        lcd.print(t);
        lcd.print(" C");
        // LCD ekranın alt kısmına nem değerini yazdırıyoruz
        lcd.setCursor(0, 1);
        lcd.print("Nem Orani: ");
        lcd.print(n);
        lcd.print(" %");
    } else if (t == 27)
        digitalWrite(RELAY, LOW);
    else if (n >= 85) {
        tone(buzzerPin, 3000);
        delay(1000);
        noTone(buzzerPin);
        delay(1000);
        digitalWrite(RELAY, HIGH);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Nem Seviyesi");
        lcd.setCursor(0, 1);
        lcd.print("Cok Yuksek...");
        delay(1000);
        lcd.clear();
        // LCD ekranın üst kısmına sıcaklık değerini yazdırıyoruz
        lcd.setCursor(0, 0);
        lcd.print("Sicaklik : ");
        lcd.print(t);
        lcd.print(" C");
        // LCD ekranın alt kısmına nem değerini yazdırıyoruz
        lcd.setCursor(0, 1);
        lcd.print("Nem Orani: ");
        lcd.print(n);
        lcd.print(" %");
    } else if (n == 70)
        digitalWrite(RELAY, LOW);

    word len = ether.packetReceive();
    word pos = ether.packetLoop(len);

    if (pos)  // Geçerli TCP verilerinin alınıp alınmadığını kontrol ediyoruz
    {
        ReadDHT11();
        ether.httpServerReply(websayfam()); // buradan web sayfamıza veri gönderiyorz
    }
    return ReadDHT11();
}
