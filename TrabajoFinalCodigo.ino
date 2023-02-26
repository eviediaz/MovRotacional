/*
 Guia para blutuh: 
 */


#include <Servo.h>
#include <Stepper.h>
#include <SoftwareSerial.h>
SoftwareSerial BTserial(2, 3);


//Inicializacion de variables principales
//-----servo
const int SERVOPIN = 6;
int servoY;
int y;
Servo servoMot;

//-----stepper


int x;

//botones
int boton1 = 13;
int boton2 = 12;

int estadoFuncion = 0;
int ultimoEstado = 0;

/*
* estado 0 -> Joystick
* estado 1 -> Rutas
* estado 2 -> Sensores
* estado 3 -> Bluetooth
*/

//Variable para las funciones
int ruta = 0;
int opcion = 0;
char blueEstado = ' ';
int t = 0;
int aumento = 1;
int pasito = 1;
int ledJoystick = 5;
int anguloActual = 90;


//-----stepper
const int stepPerRevolution = 360;
Stepper small_stepper(stepPerRevolution, 8, 10, 9, 11);


void setup() {
  //-----stepper
  small_stepper.setSpeed(100); //velocidad del stepper
  
  //-----servo
  servoMot.attach(SERVOPIN);//pasamos el numero de pin al servo
  Serial.begin(9600);

  //-----bluetooth
  BTserial.begin(9600);
  
  //-----botones
  pinMode( boton1, INPUT_PULLUP );
  pinMode( boton2, INPUT_PULLUP );
  pinMode( ledJoystick, OUTPUT );
}


void loop() {
  //Definir el estado
  definirEstado();
  
  switch (estadoFuncion){
    case 0:
      if(opcion == 0){funcionJoystick();}
      if(opcion == 1){funcionBluetooth();}
      break;
  
    case 1:
      funcionRutas();
      break;
  
    default:
    Serial.println("Hubo un error, no deberia llega aca");
    break;
  }

}


 //Esta funcion hace que el motor se controle por Joystick
void funcionJoystick(){
  //-----stepper
  x = analogRead(A0); // lee el valor del eje x
  //Serial.println(x);
  if(x < 450) {small_stepper.step(-1);} //izquierda
  if(x > 600) {small_stepper.step( 1);} //derecha
  
  //-----servo
  y = analogRead(A1);
  servoY = map(y, 0, 1023, -2, 2);
  anguloActual +=  servoY;
  
  if (anguloActual>180) { anguloActual=180; }
  if (anguloActual<0) { anguloActual=0; }
  //Serial.println( anguloActual );
  servoMot.write(anguloActual);
}


void funcionRutas(){

  switch (ruta)
  {

  /*
  Ruta 0
    Movimiento Diagonal
  */
  case 0:
    //Movimiento en Y
    servoMot.write( t );
    
    //Movimiento en X
    small_stepper.setSpeed(50);
    small_stepper.step(-aumento*4);
    
    if (t>180 || t<0){ aumento *= -1; }
    t += aumento;

    break;

  
  /*
  Ruta 1
    Cuadrado Piiola
  */
  case 1:
    if ( t>0 && t<=45 ){
      small_stepper.step(aumento*5);
    } else if ( t > 90 && t<135 ){
      small_stepper.step(-aumento*5);
    }
    
    if ( t==45 ){
      delay(500);
      servoMot.write( 135 );
      delay(500);
    } else  if ( t==135 ) {
      delay(500);
      servoMot.write( 45 );  
      delay(500);
    }

    aumento = 1;
    if (t>180 || t<0){ t = 0; }
    t += aumento;
    break;


  /*
  Ruta 2
    "Cosa"
  */
  case 2:
    if ( t<45 ){
      small_stepper.step(-aumento*4);
    } else if ( t<90 ){
      servoMot.write( t );
    } else if ( t<180 ){
      small_stepper.step(aumento*4);
    } else {
      servoMot.write( t );
    }
    
    if (t>180 || t<0){ aumento *= -1; }
    t += aumento;
    break;
  
  default:
    Serial.println("No deberias estar aca");
    break;
  }
}


void funcionBluetooth(){
  if(Serial.available()>0){
    blueEstado = Serial.read();
    BTserial.print(blueEstado);
  }

  if (BTserial.available()>0){  
    blueEstado = BTserial.read();
    Serial.print(blueEstado);
  }

  switch(blueEstado){
    case '0': //Izquierda
    small_stepper.step(-30);
    blueEstado = ' ';
    break;

    case '1': //Derecha
    small_stepper.step(30);
    blueEstado = ' ';
    break;

    case '2': //Arriba
    anguloActual += 20;
    servoMot.write(anguloActual);
    blueEstado = ' ';
    break;

    case '3': //Abajo
    anguloActual -= 20;
    servoMot.write(anguloActual);
    blueEstado = ' ';
    break;

    default:
    break;
  }
 
}


//Definir el estadoFuncion
//ademas de encender los leds respectivo
void definirEstado(){

  if ( digitalRead(boton2)==0 && ultimoEstado!=0 ){
    estadoFuncion = 0;
    anguloActual = 90;
    digitalWrite( ledJoystick, LOW );

    if ( opcion<1 ){opcion+=1;} 
    else {opcion=0;}
    
    Serial.println("Seteado a modo 0 Joystick con opcion" + (String)opcion);
  }
  
  else if ( digitalRead(boton1)==0 && ultimoEstado!=1 ){
    estadoFuncion = 1;
    digitalWrite( ledJoystick, HIGH );
    if ( ruta<2 ){
      ruta+=1;
    } else {
      ruta =0;
    }
    Serial.println("Seteado a modo 1 con ruta " + (String)ruta);

  }

  ultimoEstado = estadoFuncion;

}
