#define USE_ARDUINO_INTERRUPTS true    // Habilita interrupções para maior precisão na detecção de batimentos
#include <PulseSensorPlayground.h>    // Biblioteca PulseSensorPlayground
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Configuração do display LCD
LiquidCrystal_I2C lcd(0x27, 16, 2); // Endereço I2C do display LCD

// Variáveis
const int PulseWire = A0;           // Sensor de Pulso (fio roxo) conectado ao pino analógico A0
const int LED13 = 13;               // LED embutido na placa Arduino, pino 13
const int LED12 = 12;               // LED no pino 12, que será aceso quando detectar o batimento
int Threshold = 777 ;               // Limiar para determinar o batimento cardíaco

PulseSensorPlayground pulseSensor; // Criação do objeto PulseSensorPlayground

// Variáveis para controle do tempo
unsigned long lastBeatTime = 0;   // Variável para armazenar o último tempo de batimento
unsigned long pulseDelay = 1000;   // Atraso de 1 segundo entre os batimentos

// Parâmetros para o filtro de média móvel
const int numReadings = 10;         // Número de leituras para o filtro
int readings[numReadings];          // Buffer para armazenar as leituras
int readIndex = 0;                  // Índice atual do buffer
int total = 0;                      // Soma total das leituras
int averageBPM = 0;                 // Média dos BPMs

void setup() {
  // Inicialização do LCD
  lcd.init();
  lcd.begin(16, 2);  // Definir as dimensões corretas do LCD
  lcd.backlight();    // Liga a luz de fundo do LCD
  lcd.setCursor(0, 0);
  lcd.print("Monitor BPM");

  // Configuração do objeto PulseSensor
  pulseSensor.analogInput(PulseWire);   
  pulseSensor.blinkOnPulse(LED13);       // Faz o LED piscar com o batimento cardíaco
  pulseSensor.setThreshold(Threshold);  // Define o limiar de detecção de batimento

  // Configuração dos LEDs
  pinMode(LED12, OUTPUT);  // Define o pino 12 como saída

  Serial.begin(9600); // Inicializa a comunicação serial para depuração

  // Inicializa o buffer de leituras com 0
  for (int i = 0; i < numReadings; i++) {
    readings[i] = 0;
  }

  // Verificação de inicialização do PulseSensor
  if (pulseSensor.begin()) {
    Serial.println("Sensor de Pulso iniciado com sucesso!");
  } else {
    Serial.println("Erro ao inicializar o Sensor de Pulso.");
  }
}

void loop() {
  int myBPM = pulseSensor.getBeatsPerMinute();  // Obtém o valor de BPM

  if (pulseSensor.sawStartOfBeat()) {  // Verifica se um batimento ocorreu
    // Mensagem no monitor serial
    Serial.println("Batimento detectado!");
    Serial.print("BPM: ");
    Serial.println(myBPM);  // Imprime o BPM no monitor serial

    // Atualiza o buffer de leituras
    total = total - readings[readIndex];    // Remove a leitura mais antiga do total
    readings[readIndex] = myBPM;            // Adiciona a nova leitura
    total = total + readings[readIndex];    // Atualiza o total com a nova leitura
    readIndex = readIndex + 1;              // Avança para o próximo índice

    if (readIndex >= numReadings) {         // Se ultrapassou o número de leituras
      readIndex = 0;                        // Reseta o índice
    }

    averageBPM = total / numReadings;       // Calcula a média das leituras

    // Exibir a mensagem e BPM no LCD
    lcd.init();
    lcd.setCursor(0, 0);  // Posiciona o cursor no início
    lcd.print("Pulso Cardiaco");
    lcd.setCursor(0, 1);  // Move para a segunda linha do LCD
    lcd.print("BPM: ");
    lcd.setCursor(4, 1);  // Define posição do número
    lcd.print(averageBPM);     // Exibe o valor de BPM (filtrado)

    // Acende o LED no pino 12 quando um batimento for detectado
    digitalWrite(LED12, HIGH);

    // Marca o tempo do último batimento
    lastBeatTime = millis();
  }

  // Se o sensor não detectar um batimento (aguardando pulso), apaga o LED no pino 12
  else {
    // Atualiza o LCD para exibir "Aguardando Pulso..."
    if (millis() - lastBeatTime > pulseDelay) {  // Após 1 segundo de espera
      lcd.setCursor(0, 0);
      lcd.print("Aguardando Pulso...");

      // Desliga o LED no pino 12
      digitalWrite(LED12, LOW);
    }
  }

  delay(10);  // Pequeno atraso para estabilizar
}
