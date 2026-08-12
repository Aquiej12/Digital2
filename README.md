# Sopas Lakymen de Pollo — Red de sensores y actuadores por I²C

Máquina automática que prepara sopas instantáneas de forma secuencial (detecta el
recipiente, lo transporta, dispensa condimentos, vierte agua, coloca la tapa y
espera la cocción). El control se reparte entre **3 Arduino Nano (ATmega328P)**
comunicados por **I²C**, más un **ESP32** para monitoreo remoto en **Adafruit IO**.

Todo el firmware de los Nano está escrito en **C a bajo nivel (bare-metal)**, sin
las bibliotecas de Arduino: acceso directo a registros e I²C/UART/ADC/PWM propios.

> Proyecto de Electrónica Digital 2 — Universidad del Valle de Guatemala.

---

## Arquitectura

Topología **maestro–esclavo** sobre un único bus I²C a 100 kHz (SDA=A4, SCL=A5,
pull-ups de 4.7 kΩ a 5 V, GND común).

| Nodo | Dirección | Rol |
|------|-----------|-----|
| **Maestro** | — | Lógica del proceso (FSM de 12 fases), LCD, panel táctil, telemetría |
| **Esclavo 1** | `0x30` | Corredera: IR, ultrasónico, motor DC (L298), 2 servos |
| **Esclavo 2** | `0x31` | Dispensado: temperatura, 2 steppers, servo cafetera, relé |
| **ESP32** | — | WiFi + dashboard Adafruit IO (puente UART con el maestro) |
| **MPR121** | `0x5A` | Panel táctil capacitivo en el bus I²C (vía level-shifter 3.3/5 V) |

El maestro es el único que decide; los esclavos solo reportan sensores y ejecutan
comandos a través de un **protocolo de registros**.

---

## Estructura del repositorio

```
I2C MASTER/I2C/   -> firmware del maestro (main.c, I2C, MPR121, LCD, UART, Servo360)
I2C Slave1/I2C/   -> firmware del Esclavo 1 (0x30)
I2C Slave2/I2C/   -> firmware del Esclavo 2 (0x31)
Informe_Proyecto1.tex   -> informe del proyecto (LaTeX)
Diagrama_Conexion.tex   -> diagrama de conexión de bloques (TikZ)
Esquematico.tex         -> esquemático por nodo (TikZ)
```

Cada carpeta de nodo es un proyecto independiente de **Microchip/Atmel Studio**
(`I2C.cproj`), y comparte la misma librería `I2C.c` / `I2C.h`.

---

## Mapa de registros

### Esclavo 1 — `0x30` (corredera)

| Reg | Sentido | Descripción |
|-----|---------|-------------|
| r0 | lee | Sensor IR (1 = presencia, 0 = despejado) |
| r1 | lee | Distancia ultrasónico en cm (255 = sin eco/lejos) |
| r2 | escribe | Ángulo servo corredera (0–180) |
| r3 | escribe | Ángulo servo tapadera (0–180) |
| r4 | escribe | Motor DC: 0 = paro, 1 = avanza, 2 = retrocede |

### Esclavo 2 — `0x31` (dispensado)

| Reg | Sentido | Descripción |
|-----|---------|-------------|
| r0 | lee | Temperatura HW-503 (valor crudo) |
| r2 | escribe | Relé (0/1) |
| r4 | escribe | Servo cafetera (0 = paro, 1 = gira) |
| r5 | escribe | Vueltas del stepper de condimento |
| r6 | escribe | Vueltas del stepper de chile |
| r7 | lee | Estado: bit0 = condimento ocupado, bit1 = chile ocupado |

Uso desde el maestro:
```c
I2C_Master_WriteReg(0x30, 4, 1);          // corredera: avanzar
uint8_t d; I2C_Master_ReadReg(0x30, 1, &d, 1);  // leer distancia
```

---

## Conexiones (pinout)

### Maestro
- I²C: **A4 (SDA) / A5 (SCL)** — pull-ups 4.7 kΩ a 5 V
- LCD 2x16 (8 bits): RS-D2, RW-D3, E-D4, D0–D7 → **D5–D12**
- Servo 360 (giro continuo): **D13**
- LED táctil: **A1** · Botón START: **A2**
- MPR121 (0x5A): en el bus I²C, vía level-shifter 3.3/5 V
- UART al ESP32: **D0/D1**

### Esclavo 1 (0x30)
- IR presencia: **D2**
- Ultrasónico HC-SR04: TRIG **D3**, ECHO **D4**
- Motor DC (driver L298): IN1 **D5**, IN2 **D6**
- Servo corredera: **D11** · Servo tapadera: **D10** (PWM por software)

### Esclavo 2 (0x31)
- HW-503 (temperatura): **A1**
- Stepper condimento (ULN2003): **D2–D5**
- Stepper chile (ULN2003): **D6–D9**
- Servo cafetera (giro continuo): **D10** · Relé: **D11**

> Servomotores y motores con **fuente de 5 V independiente** y **GND común** con los Nano.

---

## Máquina de estados (maestro)

| Fase | Acción |
|------|--------|
| 1 | Espera presencia de sopa (IR) |
| 2 | Servo corredera a 90° (sujeta) |
| 3 | Avanza corredera hasta `Distancia_1` |
| 4 | Dispensa condimento (3 vueltas) y chile (2 vueltas) |
| 5 | Avanza hasta `Distancia_2` |
| 6 | Cafetera 2 s + espera `Segundos_agua` |
| 7 | Avanza hasta `Distancia_3` |
| 8 | Servo corredera a 160° (libera) |
| 9 | Corredera a 0° + tapadera a 130° |
| 10 | Cocción (3 minutos) |
| 11 | "Sopa lista" en LCD |
| 12 | Reinicio: nueva sopa + botón START |

Los umbrales de distancia, tiempos y vueltas de stepper son constantes
`#define` ajustables al inicio de `I2C MASTER/I2C/main.c`. Las esperas usan un
temporizador **no bloqueante** (Timer0 = milisegundos).

**Paro de emergencia:** se activa con el electrodo **IN0** del MPR121 (o el comando
`P` desde el ESP32) y detiene todos los actuadores; se libera con el comando `R`.

---

## Compilar y grabar

Cada nodo se abre y compila por separado en **Microchip/Atmel Studio**:

1. Abrir `I2C MASTER/I2C/I2C.cproj` → *Build* → grabar el `.hex` en el Nano maestro.
2. Abrir `I2C Slave1/I2C/I2C.cproj` → *Build* → grabar en el Esclavo 1.
3. Abrir `I2C Slave2/I2C/I2C.cproj` → *Build* → grabar en el Esclavo 2.

El `.hex` queda en `<nodo>/I2C/Debug/` (o `Release/`).

Compilación por línea de comandos (avr-gcc):
```bash
avr-gcc -mmcu=atmega328p -DF_CPU=16000000UL -Os -Wall -c *.c
avr-gcc -mmcu=atmega328p *.o -o firmware.elf
avr-objcopy -O ihex firmware.elf firmware.hex
```

---

## Puesta en marcha y diagnóstico

1. Conectar el bus: **A4↔A4, A5↔A5, GND común** entre los 3 Nano + pull-ups 4.7 kΩ.
2. Al arrancar, el maestro **escanea el bus I²C** e imprime por UART (9600 baud) las
   direcciones que responden (deberían aparecer `0x30`, `0x31`, `0x5A`).
3. Cada esclavo parpadea su **LED de actividad (D13)** al recibir/responder tráfico.

> **Al grabar el Esclavo 2**: sus datos del LCD no aplican, pero si conectas periféricos
> en D0/D1 desconéctalos durante la grabación (son RX/TX del bootloader).

---

## Documentación

- **Informe completo:** `Informe_Proyecto1.tex`
- **Diagrama de conexión (bloques):** `Diagrama_Conexion.tex`
- **Esquemático por nodo:** `Esquematico.tex`

---

## Estado

- [x] Bus I²C con protocolo de registros y recuperación ante fallos (timeout)
- [x] Esclavo 1 (IR, ultrasónico, motor DC, 2 servos)
- [x] Esclavo 2 (temperatura, 2 steppers no bloqueantes, servo cafetera, relé)
- [x] FSM de 12 fases con temporización no bloqueante
- [x] Panel táctil MPR121 y paro de emergencia
- [ ] ESP32 + Adafruit IO (puente UART) — en integración
