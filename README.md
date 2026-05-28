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




