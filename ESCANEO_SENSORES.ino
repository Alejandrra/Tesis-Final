//Algoritmo de escaneo que lo unico que hace este algoritmo es enlazarse con el
//protoclo I2C y verifica las conexiones que estan conectados al puerto
//No hace más que escanear los dos sensores
#include "Wire.h" ; si no aparece la librería es por motivos del soft "< "

void setup()
 {
  Wire.begin();

  Serial.begin(9600);
  Serial.println("\nI2C Scanner");
}

 void loop()
 {
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++ ) // da valores hexadecimal donde 0X05A para el sensor de temperatura
                                              // 0X57 de frecuencia cardiaca y spO2
  {

  Wire.beginTransmission(address);
  error = Wire.endTransmission();

  if (error == 0)
  {
   Serial.print("Dispositivo I2C en dirección 0x"); 
   if (address<16) 
   Serial.print("0");
   Serial.print(address,HEX);
   Serial.println(" !");

   nDevices++;
  }
  else if (error==4) 
  {
   Serial.print("Error desconocido en dirección 0x");
   if (address<16) 
   Serial.print("0");
   Serial.println(address,HEX);
   } 
  }
  if (nDevices == 0)
  Serial.println("Dispositivo I2C No hallado.\n");
  else
  Serial.println("hecho\n");

  delay(5000);
}
