## Инструкция по развертыванию:
1. Установить [st-flash][https://github.com/stlink-org/stlink/releases].
2. Разархивировать в любое место.
3. Скачать файл [INsoulAudio.bin][INsoulAudio.bin].
4. Подключить плату через программатор к компьютеру.
  - 3v3 = 3v3
  - SWIO = SWDIO
  - SWCLK = SWCLK
  - GND = GND
5. Открыть терминал от имени администратора.
6. В терминале выполнить:
```shell
C:\\path-to\st-flash.exe --reset write "C:\path-to\INsoulAudio.bin" 0x08000000
```

Программа будет загружена на чип. После этого подключите чип через usb к компьютеру. Устройство должно распознаться как аудиовыход.
## Карта пинов
- A5 - 48 generator enable
- A7 - 41 generator enable
- D6 - I2S Clock
- E4 - I2S Word Select
- E6 - I2S Data 





