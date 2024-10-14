#include <math.h>
#include <stdint.h>

// #define SAMPLES_PER_CYCLE 512
// #define TS 0.00003255208
// #define TS_HALF 0.00001627604

#define SAMPLES_PER_CYCLE 512
#define TS 0.00006510416
#define TS_HALF 0.00003255208

#define PI 3.1415926535897900
#define PI_2 6.283185307179590
#define PI_OVER_2 1.5707963267949000

// ========== MOVING RMS ==========

typedef struct {
  uint16_t counter;
  float sum;
  float rms;
  float k;  // Instant value (converted from ADC read to actual measure)
  uint16_t memory[SAMPLES_PER_CYCLE];
} MovingRms_t;

MovingRms_t MovingRms();

void MovingRms_CycleUpdate(MovingRms_t *measure,  // measure struct
                           uint16_t input,        // instant value of measure
                           float range,           // Range of measure (500V or 100A)
                           float base,            // Value of measure when ADC reads 0
                           int error);            // Center vaue of calibration

// ========== MOVING AVERAGE ==========

typedef struct {
  uint16_t counter;
  float sum;
  float maf;
  float memory[SAMPLES_PER_CYCLE];
} MovingAvg_t;

MovingAvg_t MovingAvg();

void MovingAvg_CycleUpdate(MovingAvg_t *measure,  // measure struct
                           float input);          // instant value of measure

// ========== UNBIASED INTEGRAL ==========

typedef struct {
  uint16_t counter;
  // Mean of integral
  float maf;
  float sum_maf;
  float vector_maf[SAMPLES_PER_CYCLE];
  // RMS of value
  float rms;
  float sum_rms;
  float vector_rms[SAMPLES_PER_CYCLE];
  float integral_0;  // i[k]
  float integral_1;  // i[k-1]
  float value;
} UnbiasedIntegral_t;

UnbiasedIntegral_t UnbiasedIntegral();

void UnbiasedIntegral_CycleUpdate(UnbiasedIntegral_t *intX,  // integral struct
                                  float input);              // instant value of measure to be integrated

// ========== SINGLE PHASE POWER ==========

typedef struct {
  float S;
  float P;
  float Q;
  float PF;
} PowerSinglePhase_t;

PowerSinglePhase_t PowerSinglePhase();

void PowerSinglePhase_CycleUpdate(uint16_t voltage_adc_read,        // Instant value of voltage
                                  uint16_t current_adc_read,        // Instant value of current
                                  int error,                        // Calibration error
                                  UnbiasedIntegral_t *int_voltage,  // Voltage integral
                                  MovingRms_t *voltage,             // Voltage rms
                                  MovingRms_t *current,             // Current rms
                                  MovingAvg_t *energy_active,       // Active energy
                                  MovingAvg_t *energy_reactive,     // Reactive energy
                                  PowerSinglePhase_t *power);       // Power (output of calculations)

// ========== FREQUENCY LOCKED LOOP ==========

typedef struct {
  float freq_rad;       // Frequency in rad/s
  float par_k_sogi;     // SOGI gain
  float par_gamma;      // Gamma
  float sogi_backward;  // SOGI backward integrator output
  float sogi_forward;   // SOGI forward integrator output
  float integrator;     // FLL integrator
} FrequencyLockedLoop_t;

FrequencyLockedLoop_t FrequencyLockedLoop();

void FrequencyLockedLoop_CycleUpdate(FrequencyLockedLoop_t *fll, float input);

float FrequencyLockedLoop_GetFrequencyHz(FrequencyLockedLoop_t *fll);

// ========== PRODIST ==========

typedef enum {
  NOMINAL,
  INTERRUPCAO_MOMENTANEO,
  AFUNDAMENTO_MOMENTANEO,
  ELEVACAO_MOMENTANEO,
  INTERRUPCAO_TEMPORARIO,
  AFUNDAMENTO_TEMPORARIO,
  ELEVACAO_TEMPORARIO
} PRODIST_vtcd_eventos;

typedef struct {
  // 1. Tensao em regime permanente (DRP e DRC)
  uint16_t DRP;
  uint16_t DRC;
  // 2. Fator de potencia (green flag or red flag)
  uint16_t FP;
  // 3. Distorcoes harmonicas (DTT, DTTp, DTTi, DTT3)
  float DTT;
  float DTTp;
  float DTTi;
  float DTT3;
  // EXCLUIDO: 4. Desequilibrios de tensão (Trifasico)
  // 5. Flutuacao de tensao
  float PST;
  // 6. Variacao de frequencia (green flag or red flag)
  float frequencia;
  // 7. VTCD (INTERRUPCAOm, AFUNDAMENTOm, ELEVACAOm, INTERRUPCAOt, AFUNDAMENTOt, ELEVACAOt) (m: momentaneo, t: temporario)
  uint16_t VTCD_interrupcao;
  uint16_t VTCD_afundamento;
  uint16_t VTCD_elevacao;
  PRODIST_vtcd_eventos VTCD_state;
} PRODIST_t;

void PRODIST_CycleUpdate(MovingRms_t *voltage,       //
                         MovingRms_t *current,       //
                         PowerSinglePhase_t *power,  //
                         PRODIST_t *prodist);        //
