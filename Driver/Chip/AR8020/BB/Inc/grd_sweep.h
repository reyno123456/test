#ifndef __ARLINK_IT_SWEEP_FREQ_SERVICE_H__
#define __ARLINK_IT_SWEEP_FREQ_SERVICE_H__

#ifdef __cplusplus
 extern "C" {
#endif


int16_t calc_power_db(int8_t bw, uint32_t power_td, 
                      int16_t *power_fd, int16_t *power_db, 
                      int16_t *total_power);

#ifdef __cplusplus
}
#endif

#endif /* __CONFIG_H__ */

/************************ (C) COPYRIGHT Artosyn *****END OF FILE****/




