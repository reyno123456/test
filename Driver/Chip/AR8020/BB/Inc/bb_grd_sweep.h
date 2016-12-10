#ifndef _SWEEP_FREQ_SERVICE_H__
#define _SWEEP_FREQ_SERVICE_H__

#ifdef __cplusplus
 extern "C" {
#endif


int16_t calc_power_db(int8_t bw, uint32_t power_td, 
                      int16_t *power_fd, int16_t *power_db, 
                      int16_t *total_power);


void grd_sweep_freq_init(void);

int8_t grd_add_sweep_result(int8_t bw);

void grd_set_next_sweep_freq(void);

uint8_t is_init_sne_average_and_fluct(void);

void calu_sne_average_and_fluct(uint8_t ch);

uint8_t get_sweep_freq(void);

uint8_t is_it_sweep_finish(void);

void init_sne_average_and_fluct(void);

uint8_t get_best_freq(void);

uint8_t get_next_best_freq(uint8_t cur_best_ch);

uint8_t is_next_best_freq_pass(uint8_t cur_best_ch, uint8_t next_best_ch);

void grd_get_sweep_noise(uint8_t row, int16_t *ptr_noise_power);


#ifdef __cplusplus
}
#endif

#endif
