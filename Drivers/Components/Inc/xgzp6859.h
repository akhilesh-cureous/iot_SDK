/*
 * xgzp6859.h
 *
 *  Created on: 25-Jun-2026
 *      Author: Akhilesh
 */

#ifndef COMPONENTS_INC_XGZP6859_H_
#define COMPONENTS_INC_XGZP6859_H_

#include <stdint.h>
#include "main.h"

#define XGZP6859D_REG_PDATA_MSB                          	(0x06U)
#define XGZP6859D_REG_PDATA_CSB								(0x07U)
#define XGZP6859D_REG_PDATA_LSB                          	(0x08U)
#define XGZP6859D_REG_TDATA_MSB                          	(0x09U)
#define XGZP6859D_REG_TDATA_LSB                          	(0x0AU)

#define XGZP6859D_REG_CMD                                	(0x30U)
#define XGZP6859D_I2C_BASE_ADDR                          	(0x6DU<<1)



uint32_t read_pressure(void);




#endif /* COMPONENTS_INC_XGZP6859_H_ */
