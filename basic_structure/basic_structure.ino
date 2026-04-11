#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <math.h>
#include "Modelo_Movimiento.h" // Incluimos tu IA generada por Python

Adafruit_MPU6050 mpu;
Eloquent::ML::Port::RandomForest clasificador; // Instanciamos el modelo

String nombreClases[] = {"Bend", "Fall", "Quiet", "Walking"};

// Definimos el tamaño de la ventana (Ej. 45 muestras = ~1 segundo)
const int NUM_MUESTRAS = 45;
float ax_arr[NUM_MUESTRAS], ay_arr[NUM_MUESTRAS], az_arr[NUM_MUESTRAS];
int indice = 0;

void setup() {
  Serial.begin(115200);
  if (!mpu.begin()) {
    while (1) delay(10);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // 1. Llenar el buffer con datos
  ax_arr[indice] = a.acceleration.x;
  ay_arr[indice] = a.acceleration.y;
  az_arr[indice] = a.acceleration.z;
  indice++;

  // 2. Cuando la ventana está llena (pasó 1 segundo), calculamos las características
  if (indice >= NUM_MUESTRAS) {
    float mean_x = 0, mean_y = 0, mean_z = 0;
    float mag_sum = 0;

    // Calcular media y magnitud
    for (int i = 0; i < NUM_MUESTRAS; i++) {
      mean_x += ax_arr[i];
      mean_y += ay_arr[i];
      mean_z += az_arr[i];
      mag_sum += sqrt(pow(ax_arr[i], 2) + pow(ay_arr[i], 2) + pow(az_arr[i], 2));
    }
    mean_x /= NUM_MUESTRAS;
    mean_y /= NUM_MUESTRAS;
    mean_z /= NUM_MUESTRAS;
    float mag_mean = mag_sum / NUM_MUESTRAS;

    // Calcular Desviación Estándar
    float std_x = 0, std_y = 0, std_z = 0;
    for (int i = 0; i < NUM_MUESTRAS; i++) {
      std_x += pow(ax_arr[i] - mean_x, 2);
      std_y += pow(ay_arr[i] - mean_y, 2);
      std_z += pow(az_arr[i] - mean_z, 2);
    }
    std_x = sqrt(std_x / NUM_MUESTRAS);
    std_y = sqrt(std_y / NUM_MUESTRAS);
    std_z = sqrt(std_z / NUM_MUESTRAS);

    // 3. Crear el array de características EXACTAMENTE en el mismo orden que en Python
    float features[] = {mean_x, std_x, mean_y, std_y, mean_z, std_z, mag_mean};

    // 4. ¡HACER LA PREDICCIÓN!
    int prediccion = clasificador.predict(features);
    
    // El modelo devuelve un número (0, 1, 2, 3...), imprime qué significa
    Serial.print("Actividad detectada: ");
    Serial.println(nombreClases[prediccion]);
    
    // Reiniciar el contador para la próxima ventana
    indice = 0;
  }

  // Esperar ~22ms para mantener los 45Hz
  delay(22);
}