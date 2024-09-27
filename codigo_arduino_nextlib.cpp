#include "Nextion.h"
#include <Servo.h>

Servo dimmer; // Objeto servo para controlar el dimmer
const int releLUZ = 13; // Pin para controlar la luz

// Pines analógicos para la temperatura y el peso
const int pinTemp = A0;
const int pinPeso = A1;

// Declarar los objetos a utilizar [pagina id : 0, componente id:1, NombreComponente: "Objeto_ObjName"]
NexText pesoTxt = NexText(0, 7, "pesoTxt");
NexText TemperaturaTxt = NexText(0, 8, "TemperaturaTxt");

// Botones para la luz
NexDSButton BtnLuzON = NexDSButton(0, 2, "BtnLuzON");

// Botón para ir a pantalla 1
NexButton BtnCalor = NexButton(0, 3, "BtnCalor");

// Botones de potencia del calentador
NexButton BtnCalor1 = NexButton(1, 6, "BtnCalor1");
NexButton BtnCalor2 = NexButton(1, 7, "BtnCalor2");
NexButton BtnCalor3 = NexButton(1, 8, "BtnCalor3");

// Botones de tiempo del calentador
NexButton BtnTC1 = NexButton(1, 9, "BtnTC1");
NexButton BtnTC2 = NexButton(1, 10, "BtnTC2");
NexButton BtnTC3 = NexButton(1, 11, "BtnTC3");

// Botón de iniciar
NexButton BtnINICIAR = NexButton(1, 14, "BtnINICIAR");

// Botón para regresar a pantalla 0
NexButton b0 = NexButton(1, 13, "b0");

// Lista de eventos touch
NexTouch *nex_listen_list[] = 
  {&BtnLuzON, &BtnCalor, &BtnCalor1, &BtnCalor2, &BtnCalor3, &BtnTC1, &BtnTC2, &BtnTC3, &BtnINICIAR, &b0, NULL};

// Prototipos de funciones de los eventos touch
void LlamadoFuncionBtnLuz(void *ptr);
void LlamadoFuncionBtnIrPantalla1(void *ptr);
void LlamadoFuncionBtnRegresarPantalla0(void *ptr);
void LlamadoFuncionBtnCalor1(void *ptr);
void LlamadoFuncionBtnCalor2(void *ptr);
void LlamadoFuncionBtnCalor3(void *ptr);
void LlamadoFuncionBtnTC1(void *ptr);
void LlamadoFuncionBtnTC2(void *ptr);
void LlamadoFuncionBtnTC3(void *ptr);
void LlamadoFuncionBtnINICIAR(void *ptr);

// Enumeración de estados
enum Estado {SELECCION_POTENCIA, SELECCION_TIEMPO, SELECCION_INICIAR, EJECUCION};
Estado estadoActual = SELECCION_POTENCIA;

int potenciaSeleccionada = 0;
int tiempoSeleccionado = 0;
int angulo = 0;  // Definir el ángulo globalmente

// Variable para temporización no bloqueante
unsigned long tiempoAnterior = 0;
#define intervaloLectura 1000 // 1 segundo //**podria ser DEFINE
uint32_t pantallaActual = 0; // Variable para almacenar el ID de la pantalla actual

void setup(void) {
  dimmer.attach(9); // Conectar el dimmer al pin 9 de Arduino
  pinMode(releLUZ, OUTPUT);
  digitalWrite(releLUZ, LOW); // Inicialmente apagado

  // Iniciar comunicación entre Arduino y la pantalla Nextion
  nexInit();

  // Registrar funciones de eventos para los botones
  BtnLuzON.attachPop(LlamadoFuncionBtnLuz, &BtnLuzON);
  BtnCalor.attachPop(LlamadoFuncionBtnIrPantalla1, &BtnCalor);
  BtnCalor1.attachPop(LlamadoFuncionBtnCalor1, &BtnCalor1);
  BtnCalor2.attachPop(LlamadoFuncionBtnCalor2, &BtnCalor2);
  BtnCalor3.attachPop(LlamadoFuncionBtnCalor3, &BtnCalor3);
  BtnTC1.attachPop(LlamadoFuncionBtnTC1, &BtnTC1);
  BtnTC2.attachPop(LlamadoFuncionBtnTC2, &BtnTC2);
  BtnTC3.attachPop(LlamadoFuncionBtnTC3, &BtnTC3);
  BtnINICIAR.attachPop(LlamadoFuncionBtnINICIAR, &BtnINICIAR);
  b0.attachPop(LlamadoFuncionBtnRegresarPantalla0, &b0);
}

void loop(void) {
  // Detectar eventos de la pantalla táctil
  nexLoop(nex_listen_list);

  // Solo sensar temperatura y peso si estamos en la pantalla 0
  if (pantallaActual == 0) {
    unsigned long tiempoActual = millis();
    if (tiempoActual - tiempoAnterior >= intervaloLectura) {
      tiempoAnterior = tiempoActual;
      //leerTemperaturaYPeso();  // Leer temperatura y peso
    }
  }

  // Ejecutar el calentador si el estado es EJECUCIÓN
  ejecutarCalentador();
}

// Función para leer la temperatura y el peso desde las entradas analógicas
void leerTemperaturaYPeso() {
  // Leer los valores analógicos
  int valorTemp = analogRead(pinTemp);
  int valorPeso = analogRead(pinPeso);

  // Convertir los valores a unidades reales (ajustar según sensores)
  float temperatura = (valorTemp * 5.0 / 1023.0) * 100.0; // Conversión a grados Celsius
  float peso = (valorPeso * 5.0 / 1023.0) * 50.0;  // Suponiendo un rango de 0-50kg

  // Crear cadenas de texto con los valores leídos
  char bufferTemp[10];
  char bufferPeso[10];
  dtostrf(temperatura, 6, 2, bufferTemp);  // Formatear a dos decimales
  dtostrf(peso, 6, 2, bufferPeso);         // Formatear a dos decimales

  // Enviar los valores a la pantalla Nextion
  TemperaturaTxt.setText(bufferTemp);
  pesoTxt.setText(bufferPeso);
}

// Función para encender o apagar la luz
void LlamadoFuncionBtnLuz(void *ptr) {
  uint32_t Estado_luz;
  BtnLuzON.getValue(&Estado_luz);
  
  if (Estado_luz) {
    digitalWrite(releLUZ, LOW); // Encender luz
  } else {
    digitalWrite(releLUZ, HIGH);  // Apagar luz
  }
}

// Función al presionar el botón para ir a pantalla 1
void LlamadoFuncionBtnIrPantalla1(void *ptr) {
  pantallaActual = 1;  // Cambiar a pantalla 1
 // Serial.println("Cambió a pantalla 1: Sensado de temperatura y peso detenido.");
}

// Función al presionar el botón para regresar a pantalla 0
void LlamadoFuncionBtnRegresarPantalla0(void *ptr) {
  pantallaActual = 0;  // Cambiar a pantalla 0
  //resetear los valores de tiempo y potencia
  tiempoSeleccionado = 0;
  potenciaSeleccionada = 0;
 // Serial.println("Regresó a pantalla 0: Reanudando sensado de temperatura y peso.");
}

// Funciones para los botones de potencia
void LlamadoFuncionBtnCalor1(void *ptr) {
  potenciaSeleccionada = 1;
  estadoActual = SELECCION_TIEMPO;
  angulo = 45;  // Ajustar ángulo
}

void LlamadoFuncionBtnCalor2(void *ptr) {
  potenciaSeleccionada = 2;
  estadoActual = SELECCION_TIEMPO;
  angulo = 90;  // Ajustar ángulo
}

void LlamadoFuncionBtnCalor3(void *ptr) {
  potenciaSeleccionada = 3;
  estadoActual = SELECCION_TIEMPO;
  angulo = 135;  // Ajustar ángulo
}

// Funciones para los botones de tiempo
void LlamadoFuncionBtnTC1(void *ptr) {
  tiempoSeleccionado = 30; // Tiempo de 30 minutos
  estadoActual = SELECCION_INICIAR;
}

void LlamadoFuncionBtnTC2(void *ptr) {
  tiempoSeleccionado = 60; // Tiempo de 60 minutos
  estadoActual = SELECCION_INICIAR;
}

void LlamadoFuncionBtnTC3(void *ptr) {
  tiempoSeleccionado = 120; // Tiempo de 120 minutos
  estadoActual = SELECCION_INICIAR;
}

void LlamadoFuncionBtnINICIAR(void *ptr){
  estadoActual = EJECUCION;
}

// Función para ejecutar el calentador (control de relé y temporizador)
void ejecutarCalentador() {
  if (estadoActual == EJECUCION) {
    dimmer.write(angulo);  // Ajustar ángulo del dimmer para controlar potencia
    delay(tiempoSeleccionado * 60000);  // Esperar el tiempo seleccionado (convertido a ms)
    dimmer.write(0);  // Apagar la calefacción
    estadoActual = SELECCION_POTENCIA;  // Regresar al estado inicial
  }
}
