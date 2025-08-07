#include <Deneyap_5x7LedEkran.h>
#include <Deneyap_KumandaKolu.h>
#include "pitches.h" 

#define SpeakerPin D0

Joystick KumandaKolu;
DotMatrix LEDMatris;

const int genislik = 5;
const int yukseklik = 7;
const int maksuzunluk = 35;

int yilanX[maksuzunluk];
int yilanY[maksuzunluk];
int yilanUzunluk = 1;

int hedefX;
int hedefY;

unsigned long sonHareketZamani = 0;
const int hareketGecikmesi = 100; 

const byte CARPI[35] = {
  0, 0, 0, 0, 0,
  1, 0, 0, 0, 1,
  0, 1, 0, 1, 0,
  0, 0, 1, 0, 0,
  0, 1, 0, 1, 0,
  1, 0, 0, 0, 1,
  0, 0, 0, 0, 0
};

void setup() {
  Serial.begin(115200);
  delay(1000);

  bool ledmatrisStarted = LEDMatris.begin(0x0A);
  bool kumandakoluStarted = KumandaKolu.begin(0x1B);

  if (!ledmatrisStarted || !kumandakoluStarted) {
    Serial.println("I2C bağlantısı başarısız.");
    while (1);
  }

  baslangicSesi(); // Oyun başlama sesi 

  yilanX[0] = 2;   //Yılanın baş kısmının konumları ayarlandı . 
  yilanY[0] = 3;

  rastgeleHedef();
}

void loop() {
  
  uint16_t xValue = KumandaKolu.xRead();
  uint16_t yValue = KumandaKolu.yRead();

  int x = 0;
  int y = 0;

  if (xValue > 520)
   x = 1;
  else if (xValue < 470)
   x = -1;

  if (yValue > 520)
   y = -1;
  else if (yValue < 470) 
  y = 1;

  if (x != 0 && y != 0) 
  y = 0;                //Tek yöne hareket sağlandı .

  if (x != 0 || y != 0) {
    if (millis() - sonHareketZamani > hareketGecikmesi) {
      sonHareketZamani = millis();

      int yeniBasX = yilanX[0] + x;
      int yeniBasY = yilanY[0] + y;

      if (yeniBasX < 0 || yeniBasX >= genislik || yeniBasY < 0 || yeniBasY >= yukseklik) {
        bitisSesi();
        Serial.println("OYUN BİTTİ: Duvara çarptın!");
        LEDMatris.drawLedMatrix(CARPI, 200);
        delay(300);  // X işareti görünür kalsın biraz
        oyunuSifirla();
        return;
      }

      for (int i = 0; i < yilanUzunluk; i++) {
        if (yilanX[i] == yeniBasX && yilanY[i] == yeniBasY) {
          bitisSesi();
          Serial.println("OYUN BİTTİ: Kendine çarptın!");
          LEDMatris.drawLedMatrix(CARPI, 200);
          delay(300);  // X işareti görünür kalsın biraz
          oyunuSifirla();
          return;
        }
      }

      for (int i = yilanUzunluk; i > 0; i--) {
        yilanX[i] = yilanX[i - 1];
        yilanY[i] = yilanY[i - 1];
      }

      yilanX[0] = yeniBasX;
      yilanY[0] = yeniBasY;

      if (yilanX[0] == hedefX && yilanY[0] == hedefY) {
        if (yilanUzunluk < maksuzunluk) {
          yilanUzunluk++;
          oyunkazandinSesi();
          rastgeleHedef();
        }
      }
    }
  }

  byte ekran[35] = {0};
  for (int i = 0; i < yilanUzunluk; i++) {
    ekran[yilanY[i] * genislik + yilanX[i]] = 1;
  }
  ekran[hedefY * genislik + hedefX] = 1;
  LEDMatris.drawLedMatrix(ekran);
}

void baslangicSesi() {
  tone(SpeakerPin, NOTE_C5, 150); 
  delay(200);
  tone(SpeakerPin, NOTE_G5, 150); 
  delay(200);
  tone(SpeakerPin, NOTE_E5, 200); 
  delay(250);
}

void bitisSesi() {
  tone(SpeakerPin, NOTE_E5, 200); 
  delay(250);
  tone(SpeakerPin, NOTE_C5, 200); 
  delay(250);
  tone(SpeakerPin, NOTE_A4, 400); 
  delay(400);
}

void oyunkazandinSesi() {
  tone(SpeakerPin, 880, 120); 
  delay(140);
  tone(SpeakerPin, 988, 120); 
  delay(140);
  tone(SpeakerPin, 1047, 200); 
  delay(220);
  tone(SpeakerPin, 1319, 300); 
  delay(320);
}

void oyunuSifirla() {
  LEDMatris.resetDotRows(); // LED'leri temizle
  yilanUzunluk = 1;
  yilanX[0] = 2;
  yilanY[0] = 3;
  rastgeleHedef();
  sonHareketZamani = millis();
}


void rastgeleHedef() {
  while (true) {
    hedefX = random(0, genislik);
    hedefY = random(0, yukseklik);

    bool carpisma = false;

    for (int i = 0; i < yilanUzunluk; i++) {
      if (yilanX[i] == hedefX && yilanY[i] == hedefY) {
        carpisma = true;
        break;
      }
    }

    if (carpisma == false) 
    break;
  }
}
