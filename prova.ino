#include <WiFi.h>
#include <HTTPClient.h>

#define led_azul 9 // Pino utilizado para controle do led azul
#define led_verde 41 // Pino utilizado para controle do led verda
#define led_vermelho 40 // Pino utilizado para controle do led vermelho
#define led_amarelo 9 // Pino utilizado para controle do led azul

const int pinoBotao = 18;  // número do pino de leitura do botão
int estadoBotao = 0;  // Variavel para armazenar o estado do botão 1 ou 0

const int pinoLdr = 4;  // Número do pino do sendor ldr
int threshold=600;

// Variável de controle do estado do semáforo
enum EstadoSemafaro { VERMELHO, AMARELO, VERDE };
EstadoSemafaro estadoAtual = VERMELHO;  // Estado inicial padrão

// Variáveis para controle de debounce
unsigned long ultimoTempoDebounce = 0;
unsigned long tempoDebounce = 50;
int ultimoEstadoBotao = LOW;
int contadorPressionamentos = 0;

void setup() {

  // Configuração inicial dos pinos para controle dos leds como OUTPUTs (saídas) do ESP32
  pinMode(led_azul,OUTPUT);
  pinMode(led_verde,OUTPUT);
  pinMode(led_vermelho,OUTPUT);

  //Exercicio 3) Inicialização das entradas
  pinMode(pinoBotao, INPUT); // Inicialização do botão
  pinMode(pinoLdr, INPUT); //Inicialização da entrada do ldr

  //Exercício 2) Iniciar os leds sempre apgadas na inicialização
  digitalWrite(led_azul, LOW);
  digitalWrite(led_verde, LOW);
  digitalWrite(led_vermelho, LOW);

  Serial.begin(9600); // Configuração para debug por interface serial entre ESP e computador com baud rate de 9600

  Serial.print("Connecting to WiFi");
  WiFi.begin("Wokwi-GUEST", "", 6); // Utilizar o parametro 6 para pular a varredura de rede Wi-fi, deixando o precesso mais rápido. seguindo a documentação: https://docs.wokwi.com/guides/esp32-wifi
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Conectado!");

  // Verifica estado do botão
  estadoBotao = digitalRead(pinoBotao);
  if (estadoBotao == HIGH) {
    Serial.println("Botão pressionado!");
  } else {
    Serial.println("Botão não pressionado!");
  }

  if(WiFi.status() == WL_CONNECTED){ // Se o ESP32 estiver conectado à Internet
    HTTPClient http;

    String serverPath = "http://www.google.com.br/"; // Endpoint da requisição HTTP

    http.begin(serverPath.c_str());

    int httpResponseCode = http.GET(); // Código do Resultado da Requisição HTTP

    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      // String payload = http.getString(); //Comentar o payload, pois não há necessidade para este código
      // Serial.println(payload);
      }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      }
      http.end();
    }

  else {
    Serial.println("WiFi Disconnected");
  }
}

void modoNoturno(){
   digitalWrite(led_amarelo, HIGH);
   delay(1000); //Delay de 1s
   digitalWrite(led_amarelo, LOW);
}

// Função para verificar botão com debounce
bool verificarBotao() {
  bool mudanca = false;
  int leitura = digitalRead(pinoBotao);

  if (leitura != ultimoEstadoBotao) {
    ultimoTempoDebounce = millis();
  }

  if ((millis() - ultimoTempoDebounce) > tempoDebounce) {
    if (leitura != estadoBotao) {
      estadoBotao = leitura;
      if (estadoBotao == HIGH) {
        mudanca = true;
      }
    }
  }

  ultimoEstadoBotao = leitura;
  return mudanca;
}

// Função para enviar alerta HTTP
void enviarAlerta() {
  Serial.print("Enviar Alerta");
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String serverPath = "http://www.google.com.br/";
    http.begin(serverPath.c_str());
    
    int httpResponseCode = http.GET();
    
    if (httpResponseCode > 0) {
      Serial.print("Código de resposta HTTP: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Erro no código: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
}

void loop() {
  int statusLdr = analogRead(pinoLdr);

  if (statusLdr >= threshold) {
    modoNoturno();
  } else {
    digitalWrite(led_vermelho, LOW);
    digitalWrite(led_amarelo, LOW);
    digitalWrite(led_verde, LOW);

    // Verifica se o botão foi pressionado com debounce
    if (estadoAtual == VERMELHO && verificarBotao()) {
      Serial.println("Botão pressionado!");
      contadorPressionamentos++;
      
      if (contadorPressionamentos >= 3) {
        Serial.println("Enviando alerta HTTP!");
        enviarAlerta();
        contadorPressionamentos = 0;
      } else {
        delay(1000); // Espera 1 segundo após o pressionamento
        estadoAtual = VERDE;
      }
    }

    switch (estadoAtual) {
      case VERMELHO:
        digitalWrite(led_vermelho, HIGH);
        delay(5000);
        estadoAtual = VERDE;
        break;
      case AMARELO:
        digitalWrite(led_amarelo, HIGH);
        delay(2000);
        estadoAtual = VERMELHO;
        break;
      case VERDE:
        digitalWrite(led_verde, HIGH);
        delay(3000);
        estadoAtual = AMARELO;
        break;
    }
  }

  delay(1000); // Reduzido para melhor resposta do debounce
}