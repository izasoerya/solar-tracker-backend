// #include <Arduino.h>
// #include <RotaryEncoder.h>

// // Pin definitions
// const uint8_t pinCLK = 11;
// const uint8_t pinDT = 12;
// const uint8_t pinSW = 13;

// // RotaryEncoder instance
// RotaryEncoder encoder(pinDT, pinCLK); // Note: RotaryEncoder expects DT first, then CLK

// void setup()
// {
//     Serial.begin(115200);
//     pinMode(pinSW, INPUT_PULLUP);

//     Serial.println("Encoder test started.");
// }

// void loop()
// {
//     static long lastPos = encoder.getPosition();
//     static bool lastButtonState = HIGH;

//     encoder.tick();

//     long newPos = encoder.getPosition();
//     if (newPos != lastPos)
//     {
//         Serial.print("Encoder moved: ");
//         Serial.println(newPos);
//         lastPos = newPos;
//     }

//     bool buttonState = digitalRead(pinSW);
//     if (lastButtonState == HIGH && buttonState == LOW)
//     {
//         Serial.println("Button tapped!");
//         delay(50); // Debounce
//     }
//     lastButtonState = buttonState;
// }
