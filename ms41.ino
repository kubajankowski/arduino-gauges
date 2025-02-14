#include <Wire.h>   // I2C library
#include <Adafruit_GFX.h> // Core graphics library
#include <Adafruit_SSD1306.h> // SSD1306 library
#include <SoftwareSerial.h> // K-LINE library

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1  
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Zmienne
float voltage = 0;      // Napięcie akumulatora
float oilTemp = 0;      // Temperatura oleju
float oilPressure = 0;  // Ciśnienie oleju
int rpm = 0;            // Obroty silnika

// Tablica do przechowywania poprzednich wartości wykresu
int lastRpmPosition[SCREEN_WIDTH / 2];  

// Konfiguracja K-Line
SoftwareSerial kline(10, 11);  // RX, TX

void setup() {
  Serial.begin(9600);
  kline.begin(10400);  // Prędkość komunikacji K-Line
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Inicjalizacja wykresu (wszystkie punkty na dole)
  for (int i = 0; i < SCREEN_WIDTH / 2; i++) {
    lastRpmPosition[i] = 64;  
  }
}

void loop() {
  // Wysyłanie komendy do K-Line
  byte command[] = {0x12, 0x05, 0x0B, 0x00, 0x1C};
  kline.write(command, sizeof(command));

  // Odczyt danych z K-Line
  if (kline.available()) {
    // Przykładowy sposób odczytu danych z K-Line
    // Zakładamy, że dane są przesyłane w formacie: voltage, oilTemp, oilPressure, rpm
    byte buffer[8];
    kline.readBytes(buffer, 8);

    // RPM
    if (buffer[0] == 0x01 && buffer[1] == 0x00 && buffer[2] == 0x00 && buffer[3] == 0xDA && buffer[4] == 0x2A) {
      rpm = (buffer[5] << 8) | buffer[6];
    } 
    //SPEED
    else if (buffer[0] == 0x00 && buffer[1] == 0x00 && buffer[2] == 0x00 && buffer[3] == 0xDA && buffer[4] == 0x63) {
    }
    //ALTERNATOR VOLTAGE
    else if (buffer[0] == 0x02 && buffer[1] == 0x00 && buffer[2] == 0x00 && buffer[3] == 0x00 && buffer[4] == 0x07) {
      voltage = buffer[5] * 0.101;
    }
  }

  // Mapowanie wartości RPM na zakres 0-15, gdzie 64 to "dół" wykresu
  int rpmMapped = map(rpm, 0, 7000, 64, 64 - 15);

  // Przesuwanie wykresu w lewo
  for (int i = 0; i < (SCREEN_WIDTH / 2) - 1; i++) {
    lastRpmPosition[i] = lastRpmPosition[i + 1];  
  }

  // Dodanie nowej wartości na końcu wykresu
  lastRpmPosition[(SCREEN_WIDTH / 2) - 1] = rpmMapped;

  // Czyszczenie ekranu
  display.clearDisplay();  

  //Wyświetlenie woltów
  display.setCursor(0, 0);
  display.print("VOLT: ");
  display.setCursor(0,12);
  display.print(voltage);
  display.println(" V");

  //Wyświetlanie temperatury
  display.setCursor(0, 35);
  display.print("TEMP: ");
  display.setCursor(0, 47);
  display.print(oilTemp);
  display.println(" C");

  //Wyświetlanie ciśnienia oleju
  display.setCursor(64, 0);
  display.print("OIL PRESS.: ");
  display.setCursor(64, 12);
  display.print(oilPressure);
  display.println(" Bar");

  // Wyświetlanie obrotów w postaci wykresów oraz liczbowo
  display.setCursor(64, 35);
  display.print("RPM: ");
  display.print(rpm);

  // Rysowanie wykresu od (64, 64)
  for (int i = 1; i < SCREEN_WIDTH / 2; i++) {
    display.drawLine(64 + i - 1, lastRpmPosition[i - 1], 64 + i, lastRpmPosition[i], WHITE);
  }

  // Aktualizacja ekranu
  display.display();
  
  // Opóźnienie dla płynności wykresu
  delay(100);
}