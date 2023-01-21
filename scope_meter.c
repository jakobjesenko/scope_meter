#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "pico/bootrom.h"
#include "measure.pio.h"
#include "pulse.pio.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "pico/multicore.h"


#define LED_PIN 25
#define FLASH_PIN 16

#define ADC_BASE_PIN 26
#define ANALOGUE_CHANNEL 0
#define REFRESH_RATE 1000
#define CONVERSION_FACTOR 3.3f / (1 << 12)

#define TRIGGER_PIN 1
#define ECHO_PIN 0


uint16_t value = 0;
float tosend = 0;

void flash(uint pin, uint32_t events){
    reset_usb_boot(0, 0);
}

void dma_handler(){
    //tosend = value * CONVERSION_FACTOR;
    dma_hw->ints0 = 1u;
}

void adc_handler(){
    dma_channel_start(ANALOGUE_CHANNEL);
}

void core1_main(){
    // setup za adc
    adc_init();
    adc_gpio_init(ADC_BASE_PIN + ANALOGUE_CHANNEL);
    adc_select_input(ANALOGUE_CHANNEL);
    adc_fifo_setup(true, false, 2, false, false);
    adc_set_round_robin(0);
    adc_set_clkdiv(0);
    irq_set_exclusive_handler(ADC_IRQ_FIFO, adc_handler);
    adc_irq_set_enabled(true);
    irq_set_enabled(ADC_IRQ_FIFO, true);
    adc_run(true);
    while (1){
        tight_loop_contents();
    }
}

int main(){
    // setup za USB
    //stdio_init_all();
    stdio_usb_init();

    // enter core1
    multicore_launch_core1(core1_main);

    // setup za nalaganje kode
    gpio_init(FLASH_PIN);
    gpio_set_dir(FLASH_PIN, GPIO_IN);
    gpio_pull_up(FLASH_PIN);
    gpio_set_irq_enabled_with_callback(FLASH_PIN, GPIO_IRQ_EDGE_FALL, true, &flash);

    // setup za LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    float delta_time = 1000 / REFRESH_RATE;

    // setup za DMA
    dma_channel_claim(ANALOGUE_CHANNEL);
    dma_channel_config c = dma_channel_get_default_config(ANALOGUE_CHANNEL);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, false);
    dma_channel_configure(
        ANALOGUE_CHANNEL,
        &c,
        &value,
        &adc_hw->fifo,
        1,
        false
    );

    dma_channel_set_irq0_enabled(ANALOGUE_CHANNEL, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);
    

    while (1){
        sleep_ms(delta_time);        
        printf("%d\n", value);
    }
}
