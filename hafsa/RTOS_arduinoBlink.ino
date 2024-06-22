#include <Arduino.h>
#include <Arduino_FreeRTOS.h>

// Pin definitions
#define ledPin LED_BUILTIN

// Task handles
TaskHandle_t rxTaskHandle;
TaskHandle_t txTaskHandle;
TaskHandle_t appTaskHandle;

// Command buffer
char commandBuffer[50];
volatile bool newCommand = false;

void setup() {
  Serial.begin(9600); // Start serial communication at 9600 baud rate
  pinMode(ledPin, OUTPUT);
  
  // Create tasks
  if (xTaskCreate(rxTask, "RX Task", 128, NULL, 1, &rxTaskHandle) != pdPASS) {
    Serial.println("RX Task creation failed");
  }
  if (xTaskCreate(txTask, "TX Task", 128, NULL, 1, &txTaskHandle) != pdPASS) {
    Serial.println("TX Task creation failed");
  }
  if (xTaskCreate(appTask, "App Task", 128, NULL, 1, &appTaskHandle) != pdPASS) {
    Serial.println("App Task creation failed");
  }
  
  // Start scheduler
  vTaskStartScheduler();
}

void loop() {
  // Empty loop, tasks are running in the background
}

void rxTask(void *pvParameters) {
  char incomingChar;
  int idx = 0;
  
  while (1) {
    if (Serial.available() > 0) {
      incomingChar = Serial.read();
      Serial.print(incomingChar); // Debugging print
      if (incomingChar == '\n') {
        commandBuffer[idx] = '\0';
        idx = 0;
        newCommand = true;
        Serial.println("New command received:"); // Debugging print
        Serial.println(commandBuffer); // Debugging print
      } else {
        commandBuffer[idx++] = incomingChar;
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS); // Delay for debounce
  }
}

void txTask(void *pvParameters) {
  while (1) {
    if (newCommand) {
      Serial.print("Echoing command: "); // Debugging print
      Serial.println(commandBuffer);
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void appTask(void *pvParameters) {
  while (1) {
    if (newCommand) {
      Serial.println("Handling command"); // Debugging print
      handleCommand(commandBuffer);
      newCommand = false;
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void handleCommand(char* command) {
  if (strcmp(command, "LED ON") == 0) {
    Serial.println("Turning LED ON"); // Debugging print
    digitalWrite(ledPin, HIGH);
  } else if (strcmp(command, "LED OFF") == 0) {
    Serial.println("Turning LED OFF"); // Debugging print
    digitalWrite(ledPin, LOW);
  } else if (strncmp(command, "BLINK ", 6) == 0) {
    int rate = atoi(command + 6);
    Serial.print("Blinking LED at rate "); // Debugging print
    Serial.println(rate); // Debugging print
    blinkLED(rate);
  } else if (strncmp(command, "ECHO ", 5) == 0) {
    Serial.print("Echoing: "); // Debugging print
    Serial.println(command + 5);
  } else {
    Serial.println("Unknown command");
  }
}

void blinkLED(int rate) {
  for (int i = 0; i < 10; ++i) {
    digitalWrite(ledPin, HIGH);
    vTaskDelay(rate / portTICK_PERIOD_MS);
    digitalWrite(ledPin, LOW);
    vTaskDelay(rate / portTICK_PERIOD_MS);
  }
}
