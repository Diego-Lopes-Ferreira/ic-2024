#include "qee.h"

// ========== MOVING RMS ==========

MovingRms_t MovingRms() {
  MovingRms_t output;
  uint16_t i;

  output.counter = 0;
  output.sum = 0;
  output.rms = 0;
  output.k = 0;
  for (i = 0; i < SAMPLES_PER_CYCLE; i++) { output.memory[i] = 2048; }

  return output;
}

void MovingRms_CycleUpdate(MovingRms_t *measure,  // measure struct
                           uint16_t input,        // instant value of measure
                           float range,           // Range of measure (500V or 100A)
                           float base,            // Value of measure when ADC reads 0
                           int error) {           // Center vaue of calibration

  measure->k = (((float)(input - error) * range) / 4095) + base;
  float last_measure_k = (((float)measure->memory[measure->counter] * range) / 4095) + base;

  measure->sum += measure->k * measure->k;
  measure->sum -= last_measure_k * last_measure_k;
  measure->rms = sqrt(measure->sum / SAMPLES_PER_CYCLE);

  measure->memory[measure->counter] = input - error;

  measure->counter++;
  if (measure->counter >= SAMPLES_PER_CYCLE) measure->counter = 0;
}

// ========== MOVING AVERAGE ==========

MovingAvg_t MovingAvg() {
  MovingAvg_t output;
  uint16_t i;

  output.counter = 0;
  output.maf = 0;
  output.sum = 0;
  for (i = 0; i < SAMPLES_PER_CYCLE; i++) { output.memory[i] = 0; }

  return output;
}

void MovingAvg_CycleUpdate(MovingAvg_t *measure,  // measure struct
                           float input) {         // instant value of measure
  measure->sum += input - measure->memory[measure->counter];
  measure->memory[measure->counter] = input;
  measure->maf = measure->sum / SAMPLES_PER_CYCLE;

  measure->counter++;
  if (measure->counter >= SAMPLES_PER_CYCLE) measure->counter = 0;
}

// ========== UNBIASED INTEGRAL ==========

UnbiasedIntegral_t UnbiasedIntegral() {
  UnbiasedIntegral_t output;
  uint16_t i;

  output.counter = 0;
  output.maf = 0;
  output.sum_maf = 0;
  output.rms = 0;
  output.sum_rms = 0;
  output.integral_0 = 0;
  output.integral_1 = 0;
  output.value = 0;
  for (i = 0; i < SAMPLES_PER_CYCLE; i++) {
    output.vector_maf[i] = 0;
    output.vector_rms[i] = 0;
  }

  return output;
}

void UnbiasedIntegral_CycleUpdate(UnbiasedIntegral_t *intX,  // integral struct
                                  float input) {             // instant value of measure to be integrated
  float input_squared;

  intX->integral_0 += (input + intX->integral_1) * TS_HALF;
  intX->integral_1 = intX->integral_0;

  // Media da integral instantanea
  intX->sum_maf += intX->integral_0 - intX->vector_maf[intX->counter];
  intX->vector_maf[intX->counter] = intX->integral_0;
  intX->maf = intX->sum_maf / SAMPLES_PER_CYCLE;

  // Valor da integral imparcial
  intX->value = intX->integral_0 - intX->maf;

  // RMS da integral imparcial
  input_squared = intX->value * intX->value;

  intX->sum_rms += input_squared - intX->vector_rms[intX->counter];
  intX->vector_rms[intX->counter] = input_squared;
  intX->rms = sqrt(intX->sum_rms / SAMPLES_PER_CYCLE);
  if (intX->rms <= 0) intX->rms = 0.0001;

  intX->counter++;
  if (intX->counter >= SAMPLES_PER_CYCLE) { intX->counter = 0; }
}

// ========== SINGLE PHASE POWER ==========

PowerSinglePhase_t PowerSinglePhase() {
  PowerSinglePhase_t output;

  output.S = 0.0f;
  output.P = 0.0f;
  output.Q = 0.0f;
  output.PF = 0.0f;

  return output;
}

void PowerSinglePhase_CycleUpdate(uint16_t voltage_adc_read,        // Instant value of voltage
                                  uint16_t current_adc_read,        // Instant value of current
                                  int error,                        // Calibration error
                                  UnbiasedIntegral_t *int_voltage,  // Voltage integral
                                  MovingRms_t *voltage,             // Voltage rms
                                  MovingRms_t *current,             // Current rms
                                  MovingAvg_t *energy_active,       // Active energy
                                  MovingAvg_t *energy_reactive,     // Reactive energy
                                  PowerSinglePhase_t *power) {      // Power (output of calculations)
  MovingRms_CycleUpdate(voltage, voltage_adc_read, 570, -285, error);
  MovingRms_CycleUpdate(current, current_adc_read, 66, -33, error);
  // MovingRms_CycleUpdate(current, current_adc_read, 33, -16, error);
  UnbiasedIntegral_CycleUpdate(int_voltage, voltage->k);
  MovingAvg_CycleUpdate(energy_active, voltage->k * current->k);
  MovingAvg_CycleUpdate(energy_reactive, int_voltage->value * current->k);

  power->S = voltage->rms * current->rms;
  power->P = energy_active->maf;
  power->Q = voltage->rms * (energy_reactive->maf / int_voltage->rms);
  power->PF = power->P / power->S;
}

// ========== FREQUENCY LOCKED LOOP ==========

FrequencyLockedLoop_t FrequencyLockedLoop() {
  FrequencyLockedLoop_t output;
  output.freq_rad = 376.991118431;
  output.par_k_sogi = 1.41;
  output.par_gamma = 0.5;
  output.sogi_backward = 0;
  output.sogi_forward = 0;
  output.integrator = 0;
  return output;
}

void FrequencyLockedLoop_CycleUpdate(FrequencyLockedLoop_t *fll, float input) {
  float error, sogi_input, qv;

  error = input - fll->sogi_forward;
  sogi_input = error * fll->par_k_sogi;
  qv = fll->sogi_backward * fll->freq_rad;

  fll->sogi_forward += (sogi_input - qv) * fll->freq_rad * TS;
  fll->sogi_backward += fll->sogi_forward * TS;

  fll->integrator += -fll->par_gamma * error * qv * TS;

  fll->freq_rad = 376.991118431 + fll->integrator;  // 60 Hz + integrator
}

float FrequencyLockedLoop_GetFrequencyHz(FrequencyLockedLoop_t *fll) { return fll->freq_rad / PI_2; }
