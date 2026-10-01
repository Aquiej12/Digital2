# Motor de audio — STM32F446RE (DAC1 / PA4)

## 1. Flujo

```
MIDI ──midi_to_c.py──> song_x.h (flash)
                           │
                     music.c  (secuenciador: 1 reloj de ticks para todas las pistas)
                           │  NoteOn(pista, instrumento, nota, vel, duración)
                     synth.c  (12 voces: oscilador DDS + envolvente ADSR)
                           │  suma entera de todas las voces (mezclador digital)
                     audio.c  (escala, recorte, +2048) ──> audio_buffer[512]
                           │
     TIM6 (32 kHz, TRGO) ──┼──> DAC1 ──> PA4 ──> TDA2030A ──> bocina
                           │
                 DMA1 Stream5 (circular) — interrumpe 2 veces por vuelta:
                   mitad A terminada -> se calcula A mientras suena B
                   mitad B terminada -> se calcula B mientras suena A
```

No hay `HAL_Delay`, ni una interrupción por muestra: el DMA entrega las muestras al DAC solo y el CPU recalcula 256 muestras cada 8 ms dentro del callback.

## 2. Archivos

| Archivo | Responsabilidad |
|---|---|
| `Core/Inc/notes.h` | Nombres de nota → número MIDI (`C4` = 60, `A4` = 69, `CS4`/`DB4` = 61, `REST` = 0). |
| `Core/Inc/music.h`, `Core/Src/music.c` | Tipos `Note`, `TrackDef`, `Song`, `Track`; duraciones (`QUARTER`…); reloj global de ticks; dispara notas; volumen por pista; ganchos `Music_OnNoteStart` / `Music_OnSongEnd`. |
| `Core/Inc/synth.h`, `Core/Src/synth.c` | Voces, formas de onda (seno, cuadrada, triangular, sierra, ruido), ADSR, instrumentos, asignación/robo de voces, render. |
| `Core/Inc/audio.h`, `Core/Src/audio.c` | TIM6 + DAC + DMA, callbacks de medio/completo, mezcla final a 12 bits, medición de carga de CPU. |
| `music/song_demo.h` | Canción original de prueba (4 pistas, acordes, silencios). |
| `tools/midi_to_c.py` | Conversor MIDI → `.h`. |
| `referencia_cubemx/` | Cómo deben quedar `main.c`, `stm32f4xx_hal_msp.c` y `stm32f4xx_it.c` después de CubeMX. |

## 3. Decisiones (y por qué)

### Notas como número MIDI (`uint8_t`), no `float`
- 1 byte en vez de 4; `440.0f` además obligaría a calcular el incremento de fase con división flotante en cada nota.
- `Synth_Init` calcula **una sola vez** una tabla `note_inc[128]` (512 B de RAM). Durante la reproducción, tocar una nota es leer `note_inc[nota]`: cero cálculos.
- Sigue siendo legible: escribes `C4`, `FS3`, `BB2`.

### Estructura `Note` (6 bytes)
```c
typedef struct { uint8_t note; uint16_t dur; uint8_t vel; uint8_t flags; } Note;
```
- `{C4, QUARTER}` funciona tal cual (vel 0 = 100 por defecto, sin flags).
- `dur` en **ticks musicales** (`MUSIC_PPQ = 48` por negra), no en ms: así la misma canción sirve a cualquier BPM y las duraciones son enteros exactos (corchea = 24, tresillo de corchea = 16, semicorchea = 12).
- `{nota, dur}` de 4 bytes no alcanza: no hay forma de representar **acordes** ni notas que se enciman. Con `NOTE_CHORD` (“la siguiente empieza al mismo tiempo”) una sola pista puede tocar acordes y notas largas debajo de una melodía, que es justo lo que trae un MIDI.
- Costo: una canción de 3 minutos con 4 pistas ≈ 2000–4000 eventos ≈ 12–24 KB de flash. Hay 512 KB.

### Voces en vez de “nota con varias frecuencias”
Un `Note` con 3 frecuencias desperdicia memoria en el 90 % de notas que son simples y no deja que cada nota del acorde dure distinto. Con un banco de `SYNTH_MAX_VOICES = 12` voces, cualquier pista toma las que necesita; si se acaban, se roba la que está en *release* más bajita (o la más vieja).

### Frecuencia de muestreo: 32 kHz
| Fs | Problema |
|---|---|
| 8 / 16 kHz | Las notas agudas y los armónicos de cuadrada/sierra se “doblan” (aliasing) y suenan sucios. |
| **32 kHz** | Ancho de banda de 16 kHz, más de lo que reproduce una bocina chica con TDA2030A. Con 80 MHz el timer da exacto: 80 000 000 / 32 000 = 2500. |
| 44.1 / 48 kHz | +40–50 % de CPU sin mejora audible en este hardware. |

### Buffer: 512 muestras (2 × 256)
Cada mitad dura 8 ms: el callback tiene 8 ms para calcular 256 muestras. 128 da más interrupciones y menos margen; 1024 agrega 32 ms de latencia (se nota entre que presionas y suena). Memoria: 1 KB (`audio_buffer`) + 1 KB (`mix_buffer`).

### Síntesis DDS (acumulador de fase)
- `phase` es un `uint32_t`: 0 … 2³² representa un ciclo completo y se desborda solo al terminar el ciclo.
- `inc = f · 2³² / Fs`. Ejemplos a 32 kHz:
  - A4 440 Hz → inc = 59 055 800
  - C4 261.63 Hz → inc = 35 114 789
  - E4 329.63 Hz → inc = 44 241 862
  - C5 523.25 Hz → inc = 70 229 578
- Cada muestra: `phase += inc; index = phase >> 24;` (8 bits = tabla de 256). Los 8 bits siguientes se usan para **interpolar** entre dos muestras de la tabla.
- Resolución de frecuencia: Fs / 2³² = 0.0000075 Hz. Medido en la simulación: A4 = 440.000 Hz, distorsión ≈ −60 dB (por debajo del ruido del DAC de 12 bits).
- Tabla de seno: 256 × `int16` = 512 B en flash. Cuadrada, triangular y sierra se calculan directo de la fase (sin tabla); ruido con xorshift32.

### Envolvente ADSR en punto fijo
Nivel en Q24 (1.0 = 2²⁴), un incremento entero por muestra. Evita los “clics” de prender/apagar una onda de golpe y da carácter a cada instrumento (bajo sostenido, batería percusiva).

### Mezclador (todo entero, sin overflow)
```
voz      = onda(Q15) × envolvente(Q15) >> 15            → ±32767
voz      = voz × (velocidad × amplitud_instrumento × volumen_pista) >> 8
mezcla   = Σ voces                                        (int32; 12 voces × 32767 cabe de sobra)
salida   = mezcla × master(224) >> 12                     → una voz a tope ≈ ±1800
salida   = recorte a −2048 … +2047, + 2048                → 0 … 4095 (DAC 12 bits)
```
El +2048 es el offset DC: el DAC solo da 0–3.3 V, así que el silencio es 1.65 V (código 2048). Con la canción demo: rango 172 … 3829, sin recortes.

### Sincronización
- Hay **un solo** contador de ticks para toda la canción. Cada pista guarda `wait` (ticks que faltan para su siguiente nota); en cada tick todas se decrementan juntas. Son enteros: después de 10 minutos siguen exactamente alineadas.
- Los ticks se cuentan en **muestras**: muestras por tick = Fs·60 / (BPM·PPQ) = 333.33 a 120 BPM. La fracción se acumula en punto fijo 16.16, así que no hay deriva.
- `Audio_FillBuffer` parte cada mitad del buffer justo en la muestra donde cae un tick: las notas empiezan con precisión de 1 muestra (31 µs), no de 8 ms.
- Ejemplo del enunciado: pista A `C4 500 ms, REST 200 ms, E4 300 ms` y pista B `G3 1000 ms` → ambas arrancan en el tick 0 y la E4 empieza exactamente 700 ms después, mientras la G3 sigue sonando.

## 4. Límites reales del STM32F446RE
| Recurso | Límite / uso |
|---|---|
| DAC | 12 bits (≈ 72 dB), hasta ~1 Msps con buffer; usamos 32 ksps. |
| RAM | Motor completo ≈ 4.9 KB (buffers 2 KB, voces 480 B, `note_inc` 512 B). |
| Flash | Motor + HAL ≈ 19 KB; cada evento de canción 6 B. |
| DMA | 1 stream (DMA1 Stream5, canal 7). No usa CPU. |
| Timers | TIM6 queda dedicado al DAC (es un timer “básico”: no tiene pines, existe para esto). |
| CPU | (Probable) ~100 ciclos por voz y muestra con `-O0`, ~30 con `-O2`. 8 voces a 32 kHz: ~32 % del CPU a 80 MHz con `-O0`, ~10 % con `-O2`. **Mídelo**: `audio_cpu_load` (en Live Expressions). Si pasa de 70 %, compila con `-O2`, sube el reloj a 180 MHz o baja a 22 050 Hz. |
| Polifonía | 12 voces (cambia `SYNTH_MAX_VOICES`). Cada voz cuesta CPU, no memoria significativa. |

## 5. CubeMX (STM32F446RE)
1. **Clock**: el que ya usas (HSI → PLL → 80 MHz, APB1 /2). `Audio_Init` lee el reloj real y calcula ARR solo; si cambias a 180 MHz no hay que tocar nada.
2. **DAC**: *Analog → DAC* → **OUT1 Configuration** ✔ (PA4 queda como `DAC_OUT1`).
   - Output Buffer: **Enable**
   - Trigger: **Timer 6 Trigger Out event**
   - Pestaña *DMA Settings* → **Add** → `DAC1`, Stream **DMA1 Stream 5**, Direction *Memory To Peripheral*, Mode **Circular**, Increment Address: **Memory** ✔, Data Width **Half Word / Half Word**, Priority High.
3. **TIM6**: *Timers → TIM6* → **Activated** ✔.
   - Prescaler **0**, Counter Period **2499**, auto-reload preload **Enable**
   - Trigger Event Selection: **Update Event**
   - (No hace falta activar la interrupción de TIM6.)
4. **NVIC**: *DMA1 stream5 global interrupt* ✔ (CubeMX lo activa al agregar el DMA). Prioridad 1 o mejor que la de tu lógica de juego.
5. **Project Manager → Code Generator**: ✔ *Generate peripheral initialization as a pair of .c/.h* (opcional).
6. **Compilación**: Project → Properties → C/C++ Build → Settings → MCU GCC Compiler → Optimization: **-O2** (recomendado para audio).

Después de generar:
- Copia `notes.h music.h synth.h audio.h` a `Core/Inc`, `music.c synth.c audio.c` a `Core/Src`, `song_demo.h` a `Core/Inc` (o agrega `music/` a los include paths).
- En `main.c`, dentro de `USER CODE BEGIN 2`:
```c
Audio_Init();
Audio_Start();
Audio_PlaySong(&song_demo, 1);   /* 1 = repetir */
```
- `referencia_cubemx/` muestra cómo deben quedar los archivos generados; compara si algo no suena.

## 6. Agregar una canción
1. `python tools/midi_to_c.py tu_cancion.mid --list` para ver las pistas.
2. `python tools/midi_to_c.py tu_cancion.mid -o song_x.h --out "lead=t1:LEAD:0.4" --out "bass=t3:BASS:0.35" --out "drums=c10:DRUMS:0.25"`
3. `#include "song_x.h"` en un solo `.c` y `Audio_PlaySong(&song_x, 0);`
El motor no se toca. Para escribirla a mano, copia el formato de `song_demo.h`.

## 7. Para el Guitar Hero
- **Fallar una nota** = bajar el volumen de la pista de guitarra: `Music_SetTrackVolume(1, 0)` y al acertar `Music_SetTrackVolume(1, VOL(0.30))`. Es lo que hace el juego original.
- **Banderas por SPI al STM de gráficos (pendiente)**: el gancho ya existe, `Music_OnNoteStart(pista, nota, vel, dur)`. Ojo con dos cosas:
  1. Se llama **dentro de la interrupción del DMA**: solo mete el dato a una cola; el envío SPI hazlo en el `while(1)` o por DMA.
  2. Ese momento es cuando la nota **suena**, pero en pantalla la nota tiene que **aparecer antes** (lo que tarda en caer, ~1 s). Las banderas deben salir con adelanto: un segundo cursor que lee la misma pista `N` ticks antes, o retrasar el audio `N` ticks. Esto se diseña cuando hagamos el protocolo.
- **Pines**: PA4 = DAC1 (audio). PA5 es el LED LD2 de la Nucleo y también `DAC_OUT2` / `SPI1_SCK`: si usas SPI1 para las banderas, no actives DAC2.
