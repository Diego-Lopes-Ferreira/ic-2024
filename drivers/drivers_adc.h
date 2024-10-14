#include "stm32g474xx.h"

// # ADC controls
void adc_startup(ADC_TypeDef *adc);
void adc_enable(ADC_TypeDef *adc);
void adc_disable(ADC_TypeDef *adc);

// # ADC channel configuration
void adc_config_sample_time(ADC_TypeDef *adc,      //
                            uint8_t channel,       //
                            uint8_t sample_time);  //

// ## Regular conversion
void adc_start_regular(ADC_TypeDef *adc);

void adc_stop_regular(ADC_TypeDef *adc);

void adc_config_regular_channel_position(ADC_TypeDef *adc,   //
                                         uint8_t channel,    //
                                         uint8_t position);  //

void adc_config_regular_channel(ADC_TypeDef *adc,     //
                                uint8_t channel,      //
                                uint8_t sample_time,  //
                                uint8_t position);    //

void adc_config_regular_sequence_length(ADC_TypeDef *adc,    //
                                        uint8_t positions);  //

// ## Injected conversion
void adc_start_injected(ADC_TypeDef *adc);

void adc_stop_injected(ADC_TypeDef *adc);

void adc_dual_12_setup();

void adc_dual_34_setup();
