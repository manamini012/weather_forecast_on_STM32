This is a final project for the subject Embedded System Designing at my school.

The MCU used in this project is STM32F407VET6 and the module for connecting to WiFi is ESP8266 NodeMCU.

There are some noteworthy points in this project:
- Input frequency is 8MHz using Crystal/Ceramic Resonator, change SYSCLK and HCLK accordingly to 168MHZ.
- Use SPI2 for SCK and MISO, MOSI communication for touch.
Source: https://stm32withoutfear.blogspot.com/2019/10/stm32-touchscreen-xpt2046-spi.html
- Use FSMC for communication with TFT LCD.
Source: https://stm32withoutfear.blogspot.com/2019/09/stm32-ili9341-fsmc.html#google_vignette
- Remember that the coordinate outputs from XPT need some extra manipulation by subtract from the height and width of the screen by the amount of XPT. In this project, the default orientation of the screen is in portrait mode so the height is 320 and width is 240.
- The project doesn't use AT commands for communication with ESP8266 but rather just raw USART, sending URLs and receiving JSON files for processing. Using ArduinoIDE to download program onto the ESP is recommended.
- Default RX, TX is utilised on the ESP.

