//-------LIBRERÍAS PARA COMUNICACION CON IOT---------

#include <ESP8266WiFi.h> // libreria del microcontrolador esp8266
#include <Wire.h> // para la comunicacion I2C
#include <Adafruit_MLX90614.h> // libreria del sensor de temperatura
#include <BlynkSimpleEsp8266.h> // libreria de la app blynk
#include "MAX30100_PulseOximeter.h" // libreria del sensor de frecuencia cardiaca y spo2
#include <Adafruit_SSD1306.h> //Librería para Pantalla OLED
#include <CayenneMQTTESP8266.h> // libreria del servidor cayenne mqtt


#define SCREEN_WIDTH 128    // ancho de pantalla OLED, en píxeles
#define SCREEN_HEIGHT 64    // altura de la pantalla OLED, en píxeles
#define OLED_RESET -1       // Restablecer pin # (o -1 si se comparte el pin de reinicio de Arduino)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


//-------------------VARIABLES GLOBALES--------------------------
//definir el objeto
PulseOximeter pox;
Adafruit_MLX90614 termometroIR = Adafruit_MLX90614(0x5A); 
float BPM, SpO2, temp;
uint32_t tsLastReport = 0;
unsigned long previousMillis = 0;

//Credenciales de Cayenne 
char username[50] = "";    
char password[50] = "";   
char clientID[50] = ""; 

float temperaturaObjeto;

//Credenciales de Blynk
char auth[] = "";

//Conexion a la red WI-FI
char ssid[] = "";
char pass[] = "";

void onBeatDetected()
{
    Serial.println("Leyendo datos");
}

// Iniciar comunicación serie

void setup() {  
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass); // Comunicacion Blynk
  Wire.begin(); 
  pox.begin();
  Cayenne.begin(username, password, clientID); //Comunicacion Cayenne
  termometroIR.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //Comunicación con lcd SSD1306 OLED 128*64
  display.display(); //Ejecuta el OLED cada 2 segundos
  delay(20000); 
  //-----inicia sensor max30100-----
  if (!pox.begin()){ 
   Serial.println("Conexion Fallida");
    for(;;);
  }
  else{
    Serial.println("Conexion Establecida"); 
  }
  pox.setOnBeatDetectedCallback(onBeatDetected); //detecta pulso el sensor max30100              
  pox.setIRLedCurrent(MAX30100_LED_CURR_24MA);// iluminicidad del sensor max30100
  
  
}

void loop() { //lee en cada momento los sensores, un ciclo repetetivo 
  
  pox.update();
  
  sensores();   
  }
    

void oledView(){ 
  unsigned long currentMillis = millis(); //se lo hace para que se actualice los datos cada 3 segundos en Blynk y Cayenne
  if (currentMillis - previousMillis >= 3000) { //
    previousMillis = currentMillis;

    //variables globales para la app Blynk
    int spo2=SpO2;
    int bpm=BPM; 

   //----Configuraciones de la pantalla OLED----------
    display.clearDisplay(); //Borra la pantalla
    display.setTextSize(1); //Dimencion de texto
    display.setTextColor(WHITE); //Color del texto
    display.setCursor(0,0); //Posiciona el cursor
    display.print("SPO2: ");
    display.setTextSize(2); // tamaño de la letra
    display.setCursor(29,0);
    display.print(SpO2,0); //valor del sensor
    display.print("%");
    
    display.setTextSize(1); 
    display.setCursor(0,25); //Posiciona del cursor
    display.print("BPM: ");
    display.setTextSize(2); 
    display.setCursor(28,25); 
    display.print(BPM,0);

    display.setTextSize(1); 
    display.setCursor(0,50); //Posiciona del cursor
    display.print("TEMP: ");

 //----Muestra los datos en el monitor serial----   
 Serial.print("Temperatura Corporal: ");  Serial.println(temp);
 Serial.print("Heart rate: ");  Serial.print(BPM);          
 Serial.print(" bpm , SpO2:");  Serial.print(SpO2);  Serial.println(" %");

 //Envio de datos a Blynk y Cayenne 
 Blynk.virtualWrite(V0,"\n" "SPO2:" + String(spo2) + "% BPM:" + String(bpm));
 Cayenne.virtualWrite(V3, SpO2);
 Cayenne.virtualWrite(V4, BPM); 

 //Definir el rango minimo y maximo del sensor temperatura
if(temp >=30 && temp <=35.9 || temp > 40.0){ //mide temperatura de 36 a 40, si pasa de 40 devuelve ... en oled
   
   //Envio de datos a Blynk y Cayenne 
   Blynk.virtualWrite(V2, temp);
   Cayenne.virtualWrite(V0, temp); 
   
    display.setTextSize(2); 
    display.setCursor(30,50);
    display.print("....");
    display.display(); 
  }else{
    Blynk.virtualWrite(V2, temp);
    display.setTextSize(2); 
    display.setCursor(30,50);
    display.print(temp,1); //muestra un decimal, si no le hace cambio muestra mas decimales
    display.print((char)247); //grafica el circulo que representa el grado en centigrados °C
    display.print("C");
    display.display(); //Ejecuta lo anterior 
   
  }
    
} }
void sensores(){
  //Inicia lectura de sensores para que obtenga los datos
  BPM = pox.getHeartRate();
  SpO2 = pox.getSpO2();
  temperaturaObjeto = termometroIR.readObjectTempC();
  calibracion();
 
  //Definir el rango minimo y maximo del sensor max30100
  if(BPM > 50 and BPM < 220 and SpO2 > 30 and SpO2 <= 100){
    oledView();
  }
  else{
    BPM = 0; SpO2 = 0; temp = 0;
   oledView();
  }
}

//---Calibracion del sensor de temperatura
void calibracion (){ 
  
  if (temperaturaObjeto >= 31.0 && temperaturaObjeto <= 31.9){ //obtiene el valor del objeto sin calibrar
      temp = temperaturaObjeto + 5.0; //suma cierta cantidad para tener ese rango al termometro real, para calibrarlo
      }else if (temperaturaObjeto >= 32.0 && temperaturaObjeto <= 32.8){
     temp = temperaturaObjeto + 4.0; // llega a 36.8
    }else {
      temp = temperaturaObjeto + 3.5; //llega 38 a 39
      }
  }
