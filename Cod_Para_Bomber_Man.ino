/*
   ================================
   Ligações do RFID RC522 com Arduino UNO
   ================================

   RC522       →     Arduino UNO
   --------------------------------
   SDA (SS)    →     D10
   SCK         →     D13
   MOSI        →     D11
   MISO        →     D12
   RST         →     D9
   3.3V        →     3.3V   (NÃO usar 5V!)
   GND         →     GND
   IRQ         →     (não usar)
*/

#include <SPI.h>
#include <MFRC522.h>
#include <Keyboard.h>
#include <string.h>
#include <Adafruit_NeoPixel.h>


#define SS_PIN 10
#define RST_PIN 9

MFRC522 mfrc522(SS_PIN, RST_PIN);

//Pino da led circular


#define PIN_LED_RGB 7
#define NUM_LED 12
#define LEDS_POR_CHAVE 3
Adafruit_NeoPixel pixels(NUM_LED, PIN_LED_RGB, NEO_GRB + NEO_KHZ800);

// uint32_t cores[] = {
//   pixels.Color(255, 0, 0),    // bloco 0 = vermelho -> amarelo2
//   pixels.Color(0, 255, 0),    // bloco 1 = verde -> vermelho
//   pixels.Color(0, 0, 255),    // bloco 2 = azul -> verde
//   pixels.Color(255, 150, 0),  // bloco 3 = laranja -> azul
// };

uint32_t cores[] = {
  pixels.Color(255, 150, 0),    // bloco 0 = vermelho -> amarelo2
  pixels.Color(255, 0, 0),    // bloco 1 = verde -> vermelho
  pixels.Color(0, 255, 0),    // bloco 2 = azul -> verde
  pixels.Color(0, 0, 255),  // bloco 3 = laranja -> azul
};


// --Definindo pinos de leds
#define LEN_KEY_0 2
#define LEN_KEY_1 3
#define LEN_KEY_2 4
#define LEN_KEY_3 5

// Butao de reset
#define BUTTON_PIN 7

// Vetores contendo strings (char*)
#define LEN_KEY 5
const char* Keys_original[LEN_KEY] = {"2361462A", "436BD126","1353152A","D32C212A","99FC77A"};
char* Keys_not_used[LEN_KEY] = {"2361462A", "436BD126","1353152A","D32C212A","99FC77A"};
char* Keys_used[LEN_KEY]     = {NULL, NULL,NULL,NULL,NULL};


void moveKey(const char* key); //funcao de mover chaves

void resetKeys(); //reseta keys

void printKeys(); // Imprimir no serial estado dos vetores

void update_leds();


void applayKeys(int idx);

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("Aproxime um cartão...");
  printKeys();

  //Definidno leds
  pinMode(LEN_KEY_0, OUTPUT);
  pinMode(LEN_KEY_1, OUTPUT);
  pinMode(LEN_KEY_2, OUTPUT);
  pinMode(LEN_KEY_3, OUTPUT);


  //Inicializando teclado
  Keyboard.begin();
  //INICIALIZA LEDS
  pixels.begin();

}

unsigned long lastPrint = 0;

void loop() {
  
  update_leds();

  // Imprime o estado das chaves a cada 2 segundos
  if (millis() - lastPrint >= 2000) {
    printKeys();
    lastPrint = millis();
  }

  // Se nenhum cartão estiver presente, sai
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    return;

  // Lê UID do cartão e transforma em string HEX
  String uidString = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uidString += String(mfrc522.uid.uidByte[i], HEX);
  }
  uidString.toUpperCase();

  Serial.print("Detectado UID: ");
  Serial.println(uidString);

  // Compara com todas as chaves
  for (int i = 0; i < LEN_KEY; i++) {
    if (Keys_not_used[i] != NULL && uidString == Keys_not_used[i]) {
      Serial.println("Chave encontrada! Movendo para 'used'");
      moveKey(Keys_not_used[i]);
      break;
    }
  }

  mfrc522.PICC_HaltA();

}

void applayKeys(int idx){
  if(Keys_used[idx]!=NULL) Keyboard.write(idx+'1');
}



// ---------- Função para mover chave ----------
void moveKey(const char* key) {
  for (int i = 0; i < LEN_KEY; i++) {
    if((Keys_not_used[i] != NULL && strcmp(Keys_not_used[4], key) == 0)){
      digitalWrite(LEN_KEY_3,LOW);
      delay(400);
      resetKeys();
      digitalWrite(LEN_KEY_3,HIGH);
      break;
    }
    if (Keys_not_used[i] != NULL && strcmp(Keys_not_used[i], key) == 0) {
      Keys_used[i] = Keys_not_used[i]; // mesma posição
      Keys_not_used[i] = NULL;
      applayKeys(i);
      break;
    }
  }
}

// ------- Função para reseta as chaves

void resetKeys() {

  // 1. Reset das chaves
  for (int i = 0; i < LEN_KEY; i++) {
    Keys_used[i] = NULL;
    Keys_not_used[i] = Keys_original[i];

    int pin = i + 2;
    digitalWrite(pin, LOW); // apaga LEDs simples
  }

  // 2. Apagar toda a fita RGB
  pixels.clear();
  pixels.show();
  delay(300); // tempo antes de começar a animação

  // 3. Acender LED por LED com as cores mapeadas
  for (int bloco = 0; bloco < LEN_KEY; bloco++) {

    uint32_t cor = cores[bloco];  // cor do bloco atual

    int inicio = bloco * LEDS_POR_CHAVE;
    int fim = inicio + LEDS_POR_CHAVE;

    // acende LEDs desse bloco um por um
    for (int led = inicio; led < fim; led++) {

      pixels.setPixelColor(led, cor);
      pixels.show();
      delay(150);  // tempo entre cada LED aceso (ajuste como quiser)
    }
  }

  Serial.println("Reset completo e LEDs animadas.");
}


// --Funcao para atualiza estado de leds

void update_leds() {

  pixels.clear(); // limpa a fita a cada atualização

  for (int i = 0; i < LEN_KEY; i++) {

    int pin = i + 2;

    // -------------------------
    // Controle dos LEDs comuns
    // -------------------------
    if (Keys_not_used[i] != NULL) {
      digitalWrite(pin, HIGH);
    } else {
      digitalWrite(pin, LOW);
    }

    // -------------------------
    // Controle da fita RGB
    // -------------------------
    if (Keys_not_used[i] != NULL) {

      int inicio = i * LEDS_POR_CHAVE;
      int fim = inicio + LEDS_POR_CHAVE;

      // pega a cor do bloco i
      uint32_t cor = cores[i];

      for (int led = inicio; led < fim; led++) {
        pixels.setPixelColor(led, cor);
      }
    }
  }

  pixels.show(); // atualiza a fita
}

// ---------- Imprime o estado dos vetores ----------
void printKeys() {
  Serial.println("------ STATUS DAS CHAVES ------");

  Serial.print("Not used: ");
  for (int i = 0; i < LEN_KEY; i++) {
    if (Keys_not_used[i] == NULL) Serial.print("[ ] ");
    else Serial.print(Keys_not_used[i]), Serial.print(" ");
  }
  Serial.println();

  Serial.print("Used:     ");
  for (int i = 0; i < LEN_KEY; i++) {
    if (Keys_used[i] == NULL) Serial.print("[ ] ");
    else Serial.print(Keys_used[i]), Serial.print(" ");
  }
  Serial.println();
  Serial.println("--------------------------------");
}


