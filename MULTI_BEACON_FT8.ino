// Programa para crear una baliza multimodo  JT65/JT9/JT4/FT8/WSPR/FSQ 
// con ESP32 + Si5351 usando las librerias Etherkit https://github.com/etherkit/JTEncode
// basado en los ejemplos de Jason Milldrum NT7S.
//
// Modificado por EA5JTT el 2025/11/25
// - Modifica el sistema de disparo de acción en PIN 12 a continuo 
// - Incluye una sección comentada de tono de prueba en el SETUP
// - Debe corregirse la frecuencia segun cada SI5351
// - Incluido slot de tiempo para FT8 (segundos 0,15,30 y45) en base a NTP
// - Se mantiene la estructura multimodo para facilitar futuros desa

// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject
// to the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
/*
 * Copyright (C) 2025 Juan Pablo Sanchez EA5JTT
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include <si5351.h>
#include <JTEncode.h>
#include <rs_common.h>
#include <int.h>
#include <string.h>

#include "Wire.h"

// NTP / WiFi
#include <WiFi.h>
#include "time.h"

// Configura aqui tu Wi-Fi
const char* ssid     = "XXXXX";
const char* password = "XXXXX";

// NTP settings
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;      // UTC
const int   daylightOffset_sec = 0; // Ajuste horario de verano si aplica

// OLED display
#include <SPI.h>
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"
#include "Fixed8x16.h"
// OLED I2C pins Lilygo T-Beam / T3 
#define OLED_RST NOT_A_PIN
#define OLED_SDA 21
#define OLED_SCL 22
Adafruit_SSD1306 display(OLED_RST);

// Mode defines
#define JT9_TONE_SPACING        174          // ~1.74 Hz
#define JT65_TONE_SPACING       269          // ~2.69 Hz
#define JT4_TONE_SPACING        437          // ~4.37 Hz
//WSPR STANDAR
#define WSPR_TONE_SPACING       146          // ~1.46 Hz
#define FSQ_TONE_SPACING        879          // ~8.79 Hz
#define FT8_TONE_SPACING        625          // ~6.25 Hz

#define JT9_DELAY               576          // Delay value for JT9-1
#define JT65_DELAY              371          // Delay in ms for JT65A
#define JT4_DELAY               229          // Delay value for JT4A
//WSPR STANDAR
#define WSPR_DELAY              683          // Delay value for WSPR
#define FSQ_2_DELAY             500          // Delay value for 2 baud FSQ
#define FSQ_3_DELAY             333          // Delay value for 3 baud FSQ
#define FSQ_4_5_DELAY           222          // Delay value for 4.5 baud FSQ
#define FSQ_6_DELAY             167          // Delay value for 6 baud FSQ
#define FT8_DELAY               159          // Delay value for FT8

/* Frecuencias FT8
Hay que corregir segun cada SI53551 en particular
1840000
3573000
5357000
7074000 
10136000
14074000
18100000
21074000
24915000
28074000
50313000
*/
// DEFINICION DE UNA FRECUENCIA POR MODO
#define JT9_DEFAULT_FREQ        14078700UL
#define JT65_DEFAULT_FREQ       14078300UL
#define JT4_DEFAULT_FREQ        14078500UL
#define WSPR_DEFAULT_FREQ       14095100UL
#define FSQ_DEFAULT_FREQ        7105350ULL     
#define FT8_DEFAULT_FREQ        14072000ULL 
#define DEFAULT_MODE            MODE_JT65

// DEFINICION DE UN CARRUSEL DE FRECUENCIAS PARA UN MODO FT8
/* const unsigned long ft8_bands[] = {
  1840000UL,   // 160 m
  3573000UL,   // 80 m
  5357000UL,   // 60 m
  7074000UL,   // 40 m
  10136000UL,  // 30 m
  14074000UL,  // 20 m
  18100000UL,  // 17 m
  21074000UL,  // 15 m
  24915000UL,  // 12 m
  28074000UL,  // 10 m
  50313000UL   // 6 m
};*/

// Frecuencias solo 20m (varia para no machacar una frecuencia)
const unsigned long ft8_bands[] = {
   14071000ULL,
   14072000ULL,
   14073000ULL
};
const int num_bands = sizeof(ft8_bands) / sizeof(ft8_bands[0]);
int current_band_index = 0;

// Hardware defines
#define LED_PIN                 13

#define SI5351FREQ 0
#define SI5351_CORRECTION 0

// Enumerations
enum mode {
  MODE_JT9,     // 0
  MODE_JT65,    // 1
  MODE_JT4,     // 2
  MODE_WSPR,    // 3
  MODE_FSQ_2,   // 4
  MODE_FSQ_3,   // 5
  MODE_FSQ_4_5, // 6
  MODE_FSQ_6,   // 7
  MODE_FT8      // 8
  };

// Class instantiation
Si5351 si5351;
JTEncode jtencode;

// Global variables
unsigned long freq;
// Real 
// char message[] = "CQ XXXXXX NNNN";
// char message[] = "CQ XXXXXX NNNN"; Pruebas
// Test
char message[] = "CQ XXXXXX NNNN";
char call[] = "XXXXXX";
char loc[] = "NNNN";
// OUTPUT POWER IN dBm
uint8_t dbm = NN;
uint8_t tx_buffer[255];
enum mode cur_mode = DEFAULT_MODE;
uint8_t symbol_count;
uint16_t tone_delay, tone_spacing;

// Función que espera hasta el próximo múltiplo de 15 segundos
/* void waitForNextFT8Slot() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No se pudo obtener hora");
    return;
  }
  int sec = timeinfo.tm_sec;
  int waitSec = 15 - (sec % 15); // segundos a esperar hasta 0,15,30,45
  Serial.printf("Segundos actuales: %d, esperando %d segundos para slot FT8...\n", sec, waitSec);
  delay(waitSec * 1000UL);
}
*/
void waitForNextFT8SlotPrecise() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No se pudo obtener hora");
    return;
  }

  int sec = timeinfo.tm_sec;
  int msec = timeinfo.tm_sec * 1000 + (millis() % 1000); // aprox milisegundos

  int waitMs = ((15 - (sec % 15)) * 1000) - (millis() % 1000);

  if(waitMs < 0) waitMs += 15000; // en caso de pasar el límite

  Serial.printf("Segundos actuales: %d, esperando %d ms para slot FT8...\n", sec, waitMs);
  delay(waitMs);  // espera exacta hasta 0,15,30,45
}



// Loop through the string, transmitting one character at a time.
void encode()
{
  uint8_t i;
  // Reset the tone to the base frequency and turn on the output
  // CLK
  si5351.output_enable(SI5351_CLK1, 1);
  digitalWrite(LED_PIN, HIGH);
  // Now transmit the channel symbols
  if(cur_mode == MODE_FSQ_2 || cur_mode == MODE_FSQ_3 || cur_mode == MODE_FSQ_4_5 || cur_mode == MODE_FSQ_6)
  {
    uint8_t j = 0;
    while(tx_buffer[j++] != 0xff);
    symbol_count = j - 1;
  }
// CLK
  for(i = 0; i < symbol_count; i++)
  {
     //Serial.println("Freq : ");
     //Serial.print(freq);
      si5351.set_freq((freq * 100ULL) + (tx_buffer[i] * tone_spacing), SI5351_CLK1);
    //  corriente entregada 2MA, 4MA , 6MAy 8MA
      si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_8MA);
      delay(tone_delay);
  }
  // Turn off the output
  si5351.output_enable(SI5351_CLK1, 0);
  digitalWrite(LED_PIN, LOW);
}

void set_tx_buffer()
{
  // Clear out the transmit buffer
  memset(tx_buffer, 0, 255);
  // Set the proper frequency and timer CTC depending on mode
  switch(cur_mode)
  {
  case MODE_JT9:
    jtencode.jt9_encode(message, tx_buffer);
    break;
  case MODE_JT65:
    jtencode.jt65_encode(message, tx_buffer);
    break;
  case MODE_JT4:
    jtencode.jt4_encode(message, tx_buffer);
    break;
  case MODE_WSPR:
    jtencode.wspr_encode(call, loc, dbm, tx_buffer);
    break;
  case MODE_FT8:
    jtencode.ft8_encode(message, tx_buffer);
    break;
  case MODE_FSQ_2:
  case MODE_FSQ_3:
  case MODE_FSQ_4_5:
  case MODE_FSQ_6:
    jtencode.fsq_dir_encode(call, "n0call", ' ', "hello world", tx_buffer);
    break;
  }
}

void setup()
{
  // Salida serie
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n[MULTI Beacon] Inicializando...");
  // OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C, true, OLED_SDA, OLED_SCL);
  display.clearDisplay();
  display.setFont(&Fixed8x16);
  display.setTextColor(WHITE);
  display.setCursor(0, 12);
  display.println("MULT TX - EA5JTT");
  display.display();

  // Inicializacion the Si5351
  // si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  if (!si5351.init(SI5351_CRYSTAL_LOAD_8PF, SI5351FREQ, 0)) {
    Serial.println("[ERROR] No se detectó el Si5351.");
    while (1);
  }
  si5351.set_correction(SI5351_CORRECTION, SI5351_PLL_INPUT_XO);
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  // cambiar CLKx por el usado CLK0, CLK1, CLK2
  si5351.output_enable(SI5351_CLK1, 0); // Disable the clock initially
  Serial.println("[Si5351] Inicializado correctamente.");
  // Use the Arduino's on-board LED as a keying indicator.
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  display.setCursor(0, 24);
  display.println("INIT SI5351");
  display.display();

 // Conectar a Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado");
  // Configurar NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  display.setCursor(0, 36);
  display.println("INIT WIFI-NTP");
  display.display();
  /* TONO DE PUEBA
  si5351.set_freq(14000000ULL * 100ULL, SI5351_CLK1);  // 14 MHz
  si5351.output_enable(SI5351_CLK1, 1);
  Serial.println("Generando RF fija en 14 MHz");
  while(1);
  */

  // AJUSTAR EL MODO DE USO
  // cur_mode = MODE_JT65;
  // cur_mode = MODE_WSPR;
  cur_mode = MODE_FT8;
  Serial.println("MODO : ");
  Serial.print(cur_mode);

  // Set the proper frequency, tone spacing, symbol count, and
  // tone delay depending on mode
  switch(cur_mode)
  {
  Serial.println("ENTRÓ EN EL SWITCH");
  case MODE_JT9:
    freq = JT9_DEFAULT_FREQ;
    symbol_count = JT9_SYMBOL_COUNT; // From the library defines
    tone_spacing = JT9_TONE_SPACING;
    tone_delay = JT9_DELAY;
    break;
  case MODE_JT65:
    freq = JT65_DEFAULT_FREQ;
    symbol_count = JT65_SYMBOL_COUNT; // From the library defines
    tone_spacing = JT65_TONE_SPACING;
    tone_delay = JT65_DELAY;
    break;
  case MODE_JT4:
    freq = JT4_DEFAULT_FREQ;
    symbol_count = JT4_SYMBOL_COUNT; // From the library defines
    tone_spacing = JT4_TONE_SPACING;
    tone_delay = JT4_DELAY;
    break;
  case MODE_WSPR:
    freq = WSPR_DEFAULT_FREQ;
    symbol_count = WSPR_SYMBOL_COUNT; // From the library defines
    tone_spacing = WSPR_TONE_SPACING;
    tone_delay = WSPR_DELAY;
    break;
  case MODE_FT8:
    //freq = FT8_DEFAULT_FREQ;
    freq = ft8_bands[0];
    symbol_count = FT8_SYMBOL_COUNT; // From the library defines
    tone_spacing = FT8_TONE_SPACING;
    tone_delay = FT8_DELAY;
    break;
  case MODE_FSQ_2:
    freq = FSQ_DEFAULT_FREQ;
    tone_spacing = FSQ_TONE_SPACING;
    tone_delay = FSQ_2_DELAY;
    break;
  case MODE_FSQ_3:
    freq = FSQ_DEFAULT_FREQ;
    tone_spacing = FSQ_TONE_SPACING;
    tone_delay = FSQ_3_DELAY;
    break;
  case MODE_FSQ_4_5:
    freq = FSQ_DEFAULT_FREQ;
    tone_spacing = FSQ_TONE_SPACING;
    tone_delay = FSQ_4_5_DELAY;
    break;
  case MODE_FSQ_6:
    freq = FSQ_DEFAULT_FREQ;
    tone_spacing = FSQ_TONE_SPACING;
    tone_delay = FSQ_6_DELAY;
    break;
  }

  // Encode the message in the transmit buffer
  // This is RAM intensive and should be done separately from other subroutines
  set_tx_buffer();
}
/*
void loop()
{
  Serial.println("[MULTI Beacon Beacon] Transmitiendo ...  ");
  encode();     // Transmitir automáticamente
  delay(10000); // Espera 10 segundos entre transmisiones
}
*/
/*
void loop() {
  // Espera al próximo slot FT8
  waitForNextFT8Slot();
  // Transmitir
  Serial.println("[MULTI Beacon] Transmitiendo FT8 ...");
  encode();
  // Opcional: espera pequeña antes de volver a comprobar
  delay(100);
  }
  */

  void loop() {
  // Espera al próximo slot FT8
  //waitForNextFT8Slot();
  waitForNextFT8SlotPrecise();
  // Ajusta la frecuencia de la banda actual
  freq = ft8_bands[current_band_index];
  Serial.printf("[MULTI Beacon] Transmitiendo FT8 en %.6f MHz\n", freq / 1e6);
  // OLED SSD1306 de 128×64 píxeles
  display.fillRect(0, 12, 128, 52, BLACK);
  display.display();
  display.setCursor(58, 36);
  display.println("FT8");
  display.setCursor(0,56);
  display.print("f(Hz): ");
  display.println(freq);
  display.display();
  // Transmitir
  encode();

  // Avanzar a la siguiente banda
  current_band_index++;
  if(current_band_index >= num_bands) {
    current_band_index = 0; // Reinicia la secuencia
  }
// OLED SSD1306 de 128×64 píxeles
  display.fillRect(0, 36, 128, 28, BLACK);
  display.display();
  display.setCursor(0,60 );
  display.println("      Espera");
  display.display();
  // Pequeña espera opcional antes de volver a comprobar
  delay(60000);
}
