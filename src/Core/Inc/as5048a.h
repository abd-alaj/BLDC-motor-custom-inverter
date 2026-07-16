/**
 ******************************************************************************
 * @file    as5048a.h
 * @brief   Driver for the AMS AS5048A absolute magnetic rotary encoder
 *          (14-bit, SPI interface, daisy-chain capable).
 *
 * @note    Target: STM32F4 (STM32CubeIDE / HAL)
 *
 * HOW THE AS5048A SPI PROTOCOL WORKS (read this before using the driver):
 *  - It is a 16-bit, full-duplex SPI transaction, SPI Mode 1 (CPOL=0, CPHA=1).
 *  - Every transaction is "send a 16-bit command, receive the PREVIOUS
 *    command's 16-bit response". So to read the angle you must send the
 *    "read angle" command frame, then send a second frame (e.g. NOP) to
 *    clock out the actual angle data. This driver hides that from you.
 *  - Bit 15 of every 16-bit word is an even-parity bit over bits 14:0.
 *  - Bit 14 of a response is an Error Flag (set if the sensor's internal
 *    error register has something new logged).
 *  - The angle itself lives in bits 13:0 of the ANGLE register response.
 ******************************************************************************
 */

#ifndef AS5048A_H
#define AS5048A_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"   /* Adjust if your exact HAL header differs -- this is pulled in via main.h normally */
#include <stdint.h>
#include <stdbool.h>

/* ==== AS5048A register addresses (14-bit addresses, per datasheet) ==== */
#define AS5048A_REG_NOP              0x0000u
#define AS5048A_REG_CLEAR_ERROR_FLAG 0x0001u
#define AS5048A_REG_DIAG_AGC         0x3FFDu
#define AS5048A_REG_MAGNITUDE        0x3FFEu
#define AS5048A_REG_ANGLE            0x3FFFu

/* Command/response bit masks */
#define AS5048A_PARITY_BIT           (1u << 15)
#define AS5048A_ERROR_BIT            (1u << 14)
#define AS5048A_DATA_MASK            0x3FFFu   /* 14-bit data field */

/**
 * @brief One handle per physical AS5048A on the bus (or per chain head).
 *        Fill this in yourself in main.c -- no dynamic allocation.
 */
typedef struct {
    SPI_HandleTypeDef *hspi;     /*!< Pointer to the HAL SPI handle, e.g. &hspi1 */
    GPIO_TypeDef       *cs_port; /*!< GPIO port for chip-select (NSS), e.g. GPIOA */
    uint16_t             cs_pin; /*!< GPIO pin for chip-select, e.g. GPIO_PIN_4 */
} AS5048A_HandleTypeDef;

/**
 * @brief Result of a single angle read, already unpacked for you.
 */
typedef struct {
    uint16_t raw_angle;     /*!< Raw 14-bit angle, 0..16383 */
    float    angle_deg;     /*!< Converted to degrees, 0..360 */
    bool     parity_ok;     /*!< true if the response's parity bit checked out */
    bool     error_flag;    /*!< true if AS5048A reported an internal error */
} AS5048A_AngleTypeDef;

/* ==== Public API ==== */

/**
 * @brief Initialize a handle. Just stores references and idles CS high.
 *        Call this once per sensor after MX_SPIx_Init() has run.
 */
void AS5048A_Init(AS5048A_HandleTypeDef *dev, SPI_HandleTypeDef *hspi,
                   GPIO_TypeDef *cs_port, uint16_t cs_pin);

/**
 * @brief Read the current angle. This performs the required two-frame
 *        SPI exchange internally (write ANGLE command, then NOP to
 *        clock the result out) and validates parity for you.
 *
 * @return true on a clean transfer (HAL_OK); check result.parity_ok and
 *         result.error_flag separately -- a "true" return only means the
 *         SPI bus transaction itself succeeded, not that the data is good.
 */
bool AS5048A_ReadAngle(AS5048A_HandleTypeDef *dev, AS5048A_AngleTypeDef *result);

/**
 * @brief Read the DIAG+AGC register (bits: OCF, COF, COMP low/high, AGC value).
 *        Useful for confirming the magnet is placed correctly.
 */
bool AS5048A_ReadDiagAGC(AS5048A_HandleTypeDef *dev, uint16_t *raw_out);

/**
 * @brief Read the CORDIC magnitude register (signal strength, not angle).
 */
bool AS5048A_ReadMagnitude(AS5048A_HandleTypeDef *dev, uint16_t *raw_out);

/**
 * @brief Clears the sensor's internal error flag register. Call this once
 *        after power-up, and any time AS5048A_AngleTypeDef.error_flag is
 *        seen set, then re-read to get the actual error content if needed.
 */
bool AS5048A_ClearErrorFlag(AS5048A_HandleTypeDef *dev, uint16_t *error_reg_out);

/**
 * @brief Low-level: send one 16-bit command frame, receive whatever the
 *        PREVIOUS frame's response was (per AS5048A's pipelined protocol).
 *        Most users should not need to call this directly -- use the
 *        wrapper functions above.
 */
bool AS5048A_TransferFrame(AS5048A_HandleTypeDef *dev, uint16_t command, uint16_t *response);

#ifdef __cplusplus
}
#endif

#endif /* AS5048A_H */
