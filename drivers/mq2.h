/*
 * ============================================================================
 * MQ2 GAS SENSOR HEADER FILE - mq2.h
 * ============================================================================
 * Mô tả: File header khai báo các hàm điều khiển cảm biến gas MQ2
 * Mục đích: Cung cấp interface để đọc nồng độ khí gas
 * ============================================================================
 */

#ifndef __MQ2_H__
#define __MQ2_H__

/*
 * Hàm: MQ2_Read
 * Mô tả: Đọc giá trị ADC từ cảm biến gas MQ2
 * Tham số: gas_raw - Con trỏ lưu giá trị ADC thô
 * Trả về: Không
 * Lưu ý:
 *   - Cảm biến cần làm nóng 24-48h lần đầu
 *   - Mất ~10ms để đọc (blocking)
 *   - Chuyển đổi sang ppm: ppm = ADC / 33
 * Ví dụ:
 *   unsigned int gas_adc;
 *   MQ2_Read(&gas_adc);
 *   unsigned int gas_ppm = gas_adc / 33;
 */
void MQ2_Read(unsigned int *gas_raw);

#endif
