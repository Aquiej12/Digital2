/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Daft Punk Vocoder — STM32F446RE
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "i2s.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BLOCK_SIZE       64
#define I2S_BUF_SIZE     (BLOCK_SIZE * 4 * 2)   /* 512 */
#define DAC_BUF_SIZE     (BLOCK_SIZE * 2)        /* 128 */
#define NUM_BANDS        8
#define SAMPLE_RATE      16000.0f
#define TWO_PI           6.2831853f

#define NUM_PRESETS      5
#define MELODY_MODE      5
#define TOTAL_MODES      6

#define MELODY_LEN       16
#define NOTE_MS          250    /* ~120 BPM */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static int16_t  i2s_rx_buf[I2S_BUF_SIZE];
static uint16_t dac_tx_buf[DAC_BUF_SIZE];
static volatile uint8_t i2s_half_ready = 0;
static volatile uint8_t i2s_full_ready = 0;

/* ── Presets de carrier ── */
static const float carrier_presets[NUM_PRESETS] = {
    100.0f, 130.0f, 150.0f, 200.0f, 250.0f
};
static uint8_t current_mode = 2;   /* arranca en 150 Hz */

/* ── Melodía estilo HBFS (C#3-F3-G#3 zona) ── */
static const float melody_notes[MELODY_LEN] = {
    139, 139, 156, 156,   /* "Har-der  bet-ter"   C#3 D#3 */
    175, 175, 139, 139,   /* "fas-ter  stron-ger"  F3  C#3 */
    139, 139, 156, 156,   /* "more-than ev-er"     C#3 D#3 */
    175, 175, 208, 208,   /* "hour-af   ter-our"   F3  G#3 */
};

/* ── Picos de bandas para LEDs (compartido con while-loop) ── */
static volatile float band_peaks[NUM_BANDS] = {0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* ══════════════════════════════════════════════════════════
 *  FILTRO BIQUAD
 * ══════════════════════════════════════════════════════════ */
typedef struct {
    float b0, b1, b2;
    float a1, a2;
    float z1, z2;
} Biquad;

static float my_sinf(float x);
static float my_cosf(float x);
static float my_expf(float x);

/* sinf/cosf/expf wrappers — evitan dependencia de -lm */
static float my_sinf(float x) {
    /* Reducir a [-PI, PI] */
    while (x > 3.14159265f)  x -= 6.28318530f;
    while (x < -3.14159265f) x += 6.28318530f;
    /* Padé-aproximación rápida */
    float x2 = x * x;
    return x * (1.0f - x2 / 6.0f * (1.0f - x2 / 20.0f * (1.0f - x2 / 42.0f)));
}

static float my_cosf(float x) {
    return my_sinf(x + 1.5707963f);
}

static float my_expf(float x) {
    /* Aproximación rápida para valores pequeños negativos */
    if (x < -4.0f) return 0.0f;
    if (x > 4.0f)  return 54.598f;
    float r = 1.0f + x / 16.0f;
    r *= r; r *= r; r *= r; r *= r;   /* (1+x/16)^16 */
    return r;
}

static void Biquad_InitBandpass(Biquad *f, float fc, float Q) {
    float w0 = TWO_PI * fc / SAMPLE_RATE;
    float alpha = my_sinf(w0) / (2.0f * Q);
    float a0_inv = 1.0f / (1.0f + alpha);

    f->b0 =  alpha * a0_inv;
    f->b1 =  0.0f;
    f->b2 = -alpha * a0_inv;
    f->a1 = -2.0f * my_cosf(w0) * a0_inv;
    f->a2 =  (1.0f - alpha) * a0_inv;
    f->z1 = 0.0f;
    f->z2 = 0.0f;
}

static inline float Biquad_Process(Biquad *f, float in) {
    float out = f->b0 * in + f->z1;
    f->z1 = f->b1 * in - f->a1 * out + f->z2;
    f->z2 = f->b2 * in - f->a2 * out;
    return out;
}

/* ══════════════════════════════════════════════════════════
 *  DETECTOR DE ENVOLVENTE
 * ══════════════════════════════════════════════════════════ */
typedef struct {
    float env;
    float attack;
    float release;
} EnvFollower;

static void EnvFollower_Init(EnvFollower *e, float attack_ms, float release_ms) {
    e->env = 0.0f;
    e->attack  = 1.0f - my_expf(-1.0f / (SAMPLE_RATE * attack_ms / 1000.0f));
    e->release = 1.0f - my_expf(-1.0f / (SAMPLE_RATE * release_ms / 1000.0f));
}

static inline float EnvFollower_Process(EnvFollower *e, float in) {
    float abs_in = (in < 0.0f) ? -in : in;
    if (abs_in > e->env)
        e->env += e->attack * (abs_in - e->env);
    else
        e->env += e->release * (abs_in - e->env);
    return e->env;
}

/* ══════════════════════════════════════════════════════════
 *  OSCILADOR DIENTE DE SIERRA
 * ══════════════════════════════════════════════════════════ */
typedef struct {
    float phase;
    float phase_inc;
} SawOsc;

static void SawOsc_SetFreq(SawOsc *osc, float freq) {
    osc->phase_inc = freq / SAMPLE_RATE;
}

static inline float SawOsc_Process(SawOsc *osc) {
    float out = 2.0f * osc->phase - 1.0f;
    osc->phase += osc->phase_inc;
    if (osc->phase >= 1.0f)
        osc->phase -= 1.0f;
    return out;
}

/* ══════════════════════════════════════════════════════════
 *  VOCODER
 * ══════════════════════════════════════════════════════════ */
typedef struct {
    Biquad      analysis[NUM_BANDS];
    Biquad      synthesis[NUM_BANDS];
    EnvFollower envelope[NUM_BANDS];
    SawOsc      carrier;
    float       band_gain[NUM_BANDS];
} Vocoder;

static Vocoder voc;

static const float band_freqs[NUM_BANDS] = {
    200.0f, 400.0f, 800.0f, 1200.0f,
    2000.0f, 3200.0f, 4800.0f, 6400.0f
};

static const float band_Q[NUM_BANDS] = {
    8.0f, 8.0f, 8.0f, 8.0f,
    8.0f, 8.0f, 8.0f, 8.0f
};

static void Vocoder_Init(Vocoder *v, float carrier_freq) {
    for (int i = 0; i < NUM_BANDS; i++) {
        Biquad_InitBandpass(&v->analysis[i],  band_freqs[i], band_Q[i]);
        Biquad_InitBandpass(&v->synthesis[i], band_freqs[i], band_Q[i]);
        EnvFollower_Init(&v->envelope[i], 0.5f, 10.0f);
        v->band_gain[i] = 1.0f;
    }
    /* Boost medios/agudos para inteligibilidad robótica */
    v->band_gain[2] = 1.5f;   /* 800 Hz  */
    v->band_gain[3] = 2.0f;   /* 1200 Hz */
    v->band_gain[4] = 2.0f;   /* 2000 Hz */
    v->band_gain[5] = 1.8f;   /* 3200 Hz */
    v->band_gain[6] = 1.5f;   /* 4800 Hz */
    v->band_gain[7] = 1.2f;   /* 6400 Hz */

    v->carrier.phase = 0.0f;
    SawOsc_SetFreq(&v->carrier, carrier_freq);
}

static inline float Vocoder_Process(Vocoder *v, float voice_in) {
    float carrier_sample = SawOsc_Process(&v->carrier);
    float output = 0.0f;

    for (int i = 0; i < NUM_BANDS; i++) {
        float band_voice   = Biquad_Process(&v->analysis[i], voice_in);
        float env          = EnvFollower_Process(&v->envelope[i], band_voice);
        float band_carrier = Biquad_Process(&v->synthesis[i], carrier_sample);
        output += band_carrier * env * v->band_gain[i];
    }
    return output;
}

/* ══════════════════════════════════════════════════════════
 *  PROCESAMIENTO DE AUDIO
 * ══════════════════════════════════════════════════════════ */

static void process_audio_block(int16_t *i2s_ptr, uint16_t *dac_ptr) {
    static int32_t gate_level = 0;

    for (int i = 0; i < BLOCK_SIZE; i++) {
        int32_t raw = (int32_t)i2s_ptr[i * 4];
        int32_t abs_r = (raw < 0) ? -raw : raw;

        /* Noise gate */
        if (abs_r > gate_level)
            gate_level += (abs_r - gate_level) >> 4;
        else
            gate_level -= (gate_level - abs_r) >> 8;

        if (gate_level < 400) {
            dac_ptr[i] = 2048;
        } else {
            float voice_in = (float)raw / 16384.0f;
            float voc_out  = Vocoder_Process(&voc, voice_in);

            int32_t out = (int32_t)(voc_out * 8192.0f) + 2048;
            if (out < 0) out = 0;
            if (out > 4095) out = 4095;
            dac_ptr[i] = (uint16_t)out;
        }

        /* Acumular picos para LEDs */
        for (int b = 0; b < NUM_BANDS; b++) {
            float e = voc.envelope[b].env;
            if (e > band_peaks[b])
                band_peaks[b] = e;
        }
    }
}

/* ── CALLBACKS DMA ── */
void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s) {
    if (hi2s->Instance == SPI2) i2s_half_ready = 1;
}
void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s) {
    if (hi2s->Instance == SPI2) i2s_full_ready = 1;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  */
int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2S2_Init();
  MX_DAC_Init();
  MX_ADC1_Init();
  MX_TIM6_Init();
  MX_USART2_UART_Init();

  /* USER CODE BEGIN 2 */
  Vocoder_Init(&voc, 150.0f);

  HAL_I2S_Receive_DMA(&hi2s2, (uint16_t *)i2s_rx_buf, I2S_BUF_SIZE / 2);
  HAL_TIM_Base_Start(&htim6);
  HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1,
                    (uint32_t *)dac_tx_buf, DAC_BUF_SIZE,
                    DAC_ALIGN_12B_R);
  /* USER CODE END 2 */

  /* USER CODE BEGIN WHILE */
  while (1)
  {
      /* ═══ BOTÓN B1: ciclar modos ═══ */
      {
          static uint8_t btn_prev = 0;
          static uint32_t btn_tick = 0;

          if (HAL_GetTick() - btn_tick > 200) {
              uint8_t btn = (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_SET);
              if (btn && !btn_prev) {
                  current_mode = (current_mode + 1) % TOTAL_MODES;

                  if (current_mode < NUM_PRESETS) {
                      /* Modo fijo */
                      SawOsc_SetFreq(&voc.carrier, carrier_presets[current_mode]);
                  }
                  /* Modo melodía: el secuenciador abajo se encarga */

                  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
                  btn_tick = HAL_GetTick();
              }
              btn_prev = btn;
          }
      }

      /* ═══ SECUENCIADOR MELODÍA (solo en modo MELODY) ═══ */
      if (current_mode == MELODY_MODE) {
          static uint32_t mel_tick = 0;
          static uint8_t  mel_step = 0;

          if (HAL_GetTick() - mel_tick >= NOTE_MS) {
              SawOsc_SetFreq(&voc.carrier, melody_notes[mel_step]);
              mel_step = (mel_step + 1) % MELODY_LEN;
              mel_tick = HAL_GetTick();

              /* Parpadear LED al ritmo */
              HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
          }
      }

      /* ═══ UART → Arduino NeoPixel (cada ~33 ms) ═══ */
      {
          static uint32_t led_tick = 0;
          if (HAL_GetTick() - led_tick >= 33) {
              uint8_t pkt[10];
              pkt[0] = 0xFF;                        /* start */
              for (int b = 0; b < 8; b++) {
                  int32_t v = (int32_t)(band_peaks[b] * 3000.0f);
                  if (v > 254) v = 254;
                  if (v < 0)   v = 0;
                  pkt[b + 1] = (uint8_t)v;
                  band_peaks[b] = 0.0f;             /* reset */
              }
              pkt[9] = 0xFE;                        /* end */
              HAL_UART_Transmit(&huart2, pkt, 10, 2);
              led_tick = HAL_GetTick();
          }
      }

      /* ═══ PROCESAMIENTO DE AUDIO ═══ */
      if (i2s_half_ready) {
          i2s_half_ready = 0;
          process_audio_block(&i2s_rx_buf[0], &dac_tx_buf[0]);
      }
      if (i2s_full_ready) {
          i2s_full_ready = 0;
          process_audio_block(&i2s_rx_buf[I2S_BUF_SIZE / 2],
                              &dac_tx_buf[DAC_BUF_SIZE / 2]);
      }

    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  *        HSI 16 MHz -> PLL -> 180 MHz SYSCLK
  *        APB1 = 45 MHz (timers 90 MHz), APB2 = 90 MHz
  *        PLLI2S -> I2S2 clock
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Voltaje Scale 1 para 180 MHz */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** HSI -> PLL: 16/8 * 180 / 2 = 180 MHz */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Over-Drive para alcanzar 180 MHz */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** AHB=180, APB1=45 (/4), APB2=90 (/2), Flash 5 wait states */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }

  /** PLLI2S para I2S2 (SPI2 esta en APB1) */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S_APB1;
  PeriphClkInitStruct.PLLI2S.PLLI2SM = 8;
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 192;
  PeriphClkInitStruct.PLLI2S.PLLI2SP = RCC_PLLI2SP_DIV2;
  PeriphClkInitStruct.PLLI2S.PLLI2SQ = 2;
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
  PeriphClkInitStruct.I2sApb1ClockSelection = RCC_I2SAPB1CLKSOURCE_PLLI2S;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  (void)file;
  (void)line;
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
