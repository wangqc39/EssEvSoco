#ifndef __FW_DECRYPTION__
#define __FW_DECRYPTION__

#define SIZE_FW_INFO        120

#define ERROR_FW_SIZE           50


uint32_t image_decryption_copy(uint32_t dst_addr,
                           uint32_t src_addr,
                           uint32_t size,
                           uint32_t progress_update_step);


#endif

