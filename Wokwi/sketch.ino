#include <WiFi.h>
#include <HTTPClient.h>

// Definição dos pinos dos LEDs
#define led_verde 41 // LED verde
#define led_vermelho 40 // LED vermelho
#define led_amarelo 9 // LED amarelo

// Definição dos pinos de entrada
#define button_pin 18 // Pino do botão
#define ldr_pin 4 // Pino do sensor LDR

// Parâmetros do sistema
const int LIMIAR_LDR = 600; // Limiar para detectar luz
const int TEMPO_VERDE = 3000; // Tempo do LED verde em ms
const int TEMPO_AMARELO = 2000; // Tempo do LED amarelo em ms
const int TEMPO_VERMELHO = 5000; // Tempo do LED vermelho em ms
const int TEMPO_DEBOUNCE = 50; // Tempo de debounce em ms
const int TEMPO_WIFI = 10000; // Tempo máximo para tentar conectar ao WiFi em ms

int contadorBotoes = 0; // Contador de pressões do botão
unsigned long ultimoTempoBotao = 0; // Tempo da última leitura do botão
bool estadoBotaoAnterior = LOW; // Estado anterior do botão
bool modoNoturno = false; // Indica se o modo noturno está ativo

// Inicializa os LEDs e garante que estejam apagados
void inicializarLeds() {
  pinMode(led_verde, OUTPUT);
  pinMode(led_vermelho, OUTPUT);
  pinMode(led_amarelo, OUTPUT);
  pinMode(button_pin, INPUT_PULLUP);

  digitalWrite(led_verde, LOW);
  digitalWrite(led_vermelho, LOW);
  digitalWrite(led_amarelo, LOW);
}

// Verifica o botão com debounce e executa ações conforme necessário
void verificarBotao() {
  bool estadoAtualBotao = digitalRead(button_pin);

  if (estadoAtualBotao != estadoBotaoAnterior && millis() - ultimoTempoBotao > TEMPO_DEBOUNCE) {

    if (estadoAtualBotao == HIGH) {
      contadorBotoes++;
      Serial.print("Botão pressionado! Contador: ");
      Serial.println(contadorBotoes);

      if (contadorBotoes == 3) {
        enviarAlerta();
        contadorBotoes = 0;
      }
    }
    ultimoTempoBotao = millis();
  }
  estadoBotaoAnterior = estadoAtualBotao;
}

// Envia alerta HTTP após três pressões do botão
void enviarAlerta() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://www.google.com.br/");

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

    } else {
      Serial.print("Erro na requisição: ");
      Serial.println(httpResponseCode);
    }
    http.end();

  } else {
    Serial.println("WiFi desconectado");
  }
}

void setup() {
  Serial.begin(115200);

  inicializarLeds(); // Configura LEDs

  WiFi.begin("Wokwi-GUEST", ""); // Conecta ao WiFi

  unsigned long tempoInicio = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - tempoInicio < TEMPO_WIFI) {
    delay(100);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConectado ao WiFi!");
  } else {
    Serial.println("\nNão foi possível conectar ao WiFi. Wowki é ruim >:( Continuando sem conexão. Renicie para tentar conectar de novo");
  }
}

// Ativa o modo noturno piscando o LED amarelo
void modoNoturnoAtivo() {

  digitalWrite(led_verde, LOW);
  digitalWrite(led_vermelho, LOW);
  digitalWrite(led_amarelo, HIGH);

  delay(500);

  digitalWrite(led_amarelo, LOW);

  delay(500);
}

// Alterna os LEDs no modo convencional
void modoConvencional() {

  digitalWrite(led_verde, HIGH);
  delay(TEMPO_VERDE);
  digitalWrite(led_verde, LOW);

  digitalWrite(led_amarelo, HIGH);
  delay(TEMPO_AMARELO);
  digitalWrite(led_amarelo, LOW);

  digitalWrite(led_vermelho, HIGH);
  delay(TEMPO_VERMELHO);
  digitalWrite(led_vermelho, LOW);
}

void loop() {

    int leituraBotao = digitalRead(button_pin); // Lê o estado do botão

  if (leituraBotao == LOW) { // Verifica se o botão está pressionado
    Serial.println("Botão pressionado!");
    delay(200); // Debounce simples para evitar mensagens repetidas
  }
  
  int leituraLDR = analogRead(ldr_pin);

  if (leituraLDR >= LIMIAR_LDR) { // Verifica se está escuro
    modoNoturno = true;
    modoNoturnoAtivo();

  } else { // Caso contrário, ativa o modo convencional
    modoNoturno = false;
    modoConvencional();
  }

  if (!modoNoturno && digitalRead(led_vermelho) == HIGH) { // Verifica botão no modo claro
    verificarBotao();

    if (contadorBotoes == 1) { // Abre o semáforo após 1 segundo

      delay(1000);

      digitalWrite(led_vermelho, LOW);
      digitalWrite(led_verde, HIGH);

      delay(TEMPO_VERDE);

      digitalWrite(led_verde, LOW);

      contadorBotoes = 0;
    }
  }
}
