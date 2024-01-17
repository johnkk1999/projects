#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/pio.h"

#define   CAPTURE_CHANNEL 2
#define CAPTURE_DEPTH 1000

uint8_t capture_buf[CAPTURE_DEPTH];
uint dma_chan = 0;
int counter = 0;
int clkdiv [] = {0, 2, 4, 8, 16};

void gpio_callback(uint gpio, uint32_t event);
void set_adc_div();
void set_adc(int clkdiv);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  set_adc(16);
  // setting up interrupt to change adc_clk
  gpio_init(15);
  gpio_pull_up(15);
  gpio_set_irq_enabled_with_callback(15, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
}

void loop() {
  adc_run(true);
  dma_channel_wait_for_finish_blocking(dma_chan);
  adc_run(false);
  adc_fifo_drain();

  //transmitting data to the serial port
 // Serial.write(0x7F);
  //Serial.write(255);
  
  for (int i = 0; i < CAPTURE_DEPTH; i++)
  {
    Serial.printf("%-3d",capture_buf[i]);
    Serial.println();
  }


}
void set_adc_div()
{
  if (counter < 12) {
 set_adc(counter);
    counter++;
  }
  else {
   set_adc(counter);
    counter = 0;
  }
}
void gpio_callback(uint gpio, uint32_t event)
{
  if (gpio == 15)
  {
    set_adc_div();
  }
}
void set_adc(int clkdiv)
{
  adc_gpio_init(26 + CAPTURE_CHANNEL); //initialise gpio 26 for analogue use
  adc_init();
  adc_select_input(CAPTURE_CHANNEL);
  adc_fifo_setup( true, //write each completed conversion to fifo
                  true, // Enable DMA data request (DREQ)
                  1,    /// DREQ asserted when at least 1 sample present
                  false, //no error bit
                  true //shift each sample to 8 bits when pushing to FIFO
                );

  adc_set_clkdiv(96 * clkdiv); // setting up adc clock division

  //setting up DMA as soon as it appears at the FIFO
  dma_chan = dma_claim_unused_channel(true);
  dma_channel_config cfg = dma_channel_get_default_config(dma_chan);

  channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);
  channel_config_set_read_increment(&cfg, false);
  channel_config_set_write_increment(&cfg, true);

  //pace transfer base on availability  of ADC samples
  channel_config_set_dreq(&cfg, DREQ_ADC);

  dma_channel_configure(dma_chan, &cfg,
                        capture_buf,
                        &adc_hw->fifo,
                        CAPTURE_DEPTH,
                        true
                       );

  }
