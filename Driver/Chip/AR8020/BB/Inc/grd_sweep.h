#ifndef __ARLINK_IT_SWEEP_FREQ_SERVICE_H__
#define __ARLINK_IT_SWEEP_FREQ_SERVICE_H__

#ifdef __cplusplus
 extern "C" {
#endif


uint8_t is_it_sweep_finish(void);

void init_sne_average_and_fluct(void);

uint8_t get_best_freq(void);

void clear_snr_sne(void);

uint8_t get_next_best_freq(uint8_t cur_best_ch);

uint8_t is_snr_ok(uint16_t iMCS);

uint8_t is_next_best_freq_pass(uint8_t cur_best_ch,uint8_t next_best_ch);

void grd_set_next_sweep_freq(void);

void grd_add_sweep_result(void);

uint8_t is_init_sne_average_and_fluct(void);

void calu_sne_average_and_fluct(uint8_t ch);

uint8_t get_sweep_freq(void);

void grd_sweep_freq_init(void);

uint32_t get_sweep_value(void);

uint8_t get_sweep_channel(void);

void clear_sweep_results(void);

uint8_t ch_to_index(uint8_t cur_ch);

#ifdef __cplusplus
}
#endif

#endif /* __CONFIG_H__ */

/************************ (C) COPYRIGHT Artosyn *****END OF FILE****/




