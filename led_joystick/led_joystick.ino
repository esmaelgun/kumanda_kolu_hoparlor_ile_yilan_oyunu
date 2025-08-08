#include <Deneyap_5x7LedEkran.h>
#include <Deneyap_KumandaKolu.h>
#include "pitches.h" 
#include <Deneyap_OLED.h>   

#define SpeakerPin D0

Joystick KumandaKolu;
DotMatrix LEDMatris;
OLED OLED ;

const int genislik = 5;
const int yukseklik = 7;
const int maksuzunluk = 35;

int yilanX[maksuzunluk];
int yilanY[maksuzunluk];
int yilanUzunluk = 1;

int skor;
int hedefX;
int hedefY;

unsigned long sonHareketZamani = 0;
int hareketGecikmesi = 200; 

// Yılanın hareket yönü (başlangıçta sağa hareket edecek)
int yonX = 1;
int yonY = 0;

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
  bool oledStarted= OLED.begin(0x7A);   

  if (!ledmatrisStarted || !kumandakoluStarted || !oledStarted) {
    Serial.println("I2C bağlantısı başarısız.");
    while (1);
  }

  baslangicSesi(); // Oyun başlama sesi 

  OLED.clearDisplay();
  OLED.setTextXY(3, 2);
  OLED.putString("OYUN BASLIYOR!");
  delay(2000);
  OLED.clearDisplay();
  
  // Başlangıç pozisyonunu rastgele ayarla
  yilanX[0] = random(0, genislik);
  yilanY[0] = random(0, yukseklik);

  rastgeleHedef();

  skor = 1; // skor sıfırla ve OLED'e yaz
  OLED.clearDisplay();
  OLED.setTextXY(3, 3);
  OLED.putString("SKOR:");
  OLED.putNumber(skor);
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
    y = 0;  // çapraz hareket engelle

  // Eğer joystick geçerli bir yön sağlıyorsa hareket yönünü güncelle
  if (x != 0 || y != 0) {
    yonX = x;
    yonY = y;
  }

  // Hareket gecikmesini kontrol edip yılanı son yönünde hareket ettir
  if (millis() - sonHareketZamani > hareketGecikmesi) {
    sonHareketZamani = millis();

    int yeniBasX = yilanX[0] + yonX;
    int yeniBasY = yilanY[0] + yonY;

    // Duvara çarpma kontrolü
    if (yeniBasX < 0 || yeniBasX >= genislik || yeniBasY < 0 || yeniBasY >= yukseklik) {
      bitisSesi();
      LEDMatris.drawLedMatrix(CARPI, 200);
      delay(200);
      OLED.clearDisplay();
      OLED.setTextXY(2, 2);
      OLED.putString("OYUN BITTI!");
      OLED.setTextXY(3, 1);
      OLED.putString("Duvara carptin!");
      delay(2000);
      oyunuSifirla();
      return;
    }

    // Kendi üstüne çarpma kontrolü
    for (int i = 0; i < yilanUzunluk; i++) {
      if (yilanX[i] == yeniBasX && yilanY[i] == yeniBasY) {
        bitisSesi();
        LEDMatris.drawLedMatrix(CARPI, 200);
        delay(200);
        OLED.clearDisplay();
        OLED.setTextXY(2, 2);
        OLED.putString("OYUN BITTI!");
        OLED.setTextXY(4, 1);
        OLED.putString("Kendine carptin!");
        delay(2000);
        oyunuSifirla();
        return;
      }
    }

    // Yılanın gövdesini hareket ettir
    for (int i = yilanUzunluk; i > 0; i--) {
      yilanX[i] = yilanX[i - 1];
      yilanY[i] = yilanY[i - 1];
    }

    // Yeni baş pozisyonunu ata
    yilanX[0] = yeniBasX;
    yilanY[0] = yeniBasY;

    // Hedefe ulaşıldı mı kontrol et
    if (yilanX[0] == hedefX && yilanY[0] == hedefY) {
      if (yilanUzunluk < maksuzunluk) {
        yilanUzunluk++;
        skor = yilanUzunluk;
        oyunSesi();
        rastgeleHedef();

        // OLED'de skoru güncelle
        OLED.clearDisplay();
        OLED.setTextXY(3, 3);
        OLED.putString("SKOR:");
        OLED.putNumber(skor);
      }
      else {
        oyunKazandinSesi();
        OLED.clearDisplay();
        OLED.setTextXY(2, 2);
        OLED.putString("OYUNU KAZANDIN!");
        delay(3000);
        oyunuSifirla();
        return;
      }
    }
  }

  // LED matrisi yılan ve hedefi çiz
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

void oyunSesi() {
  tone(SpeakerPin, 880, 120); 
  delay(140);
  tone(SpeakerPin, 988, 120); 
  delay(140);
  tone(SpeakerPin, 1047, 200); 
  delay(220);
  tone(SpeakerPin, 1319, 300); 
  delay(320);
}
void oyunKazandinSesi() {
  tone(SpeakerPin, 784, 150);  // G5
  delay(180);
  tone(SpeakerPin, 880, 150);  // A5
  delay(180);
  tone(SpeakerPin, 987, 300);  // B5
  delay(350);
  tone(SpeakerPin, 1175, 400); // D6 
  delay(450);
}

void oyunuSifirla() {
  LEDMatris.resetDotRows(); // LED'leri temizle
  yilanUzunluk = 1;
  yilanX[0] = random(0, genislik);
  yilanY[0] = random(0, yukseklik);
  rastgeleHedef();
  skor = 1; // skor sıfırla
  OLED.clearDisplay();
  OLED.setTextXY(3, 3);
  OLED.putString("SKOR:");
  OLED.putNumber(skor);
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

    if (!carpisma) 
      break;
  }
}
