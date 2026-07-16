/**
 ******************************************************************************
 * @file    as5048a.c
 * @brief   Driver implementation for the AMS AS5048A absolute encoder.
 ******************************************************************************
 */

#include "as5048a.h"

#define AS5048A_SPI_TIMEOUT_MS  10u

/* ---- Internal helpers ---- */

static inline void CS_Low(AS5048A_HandleTypeDef *dev)
{
    HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_RESET);
}

static inline void CS_High(AS5048A_HandleTypeDef *dev)
{
    HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_SET);
}

/* Even parity over bits 14:0 of a 16-bit word, returned as a bit to OR into bit 15 */
static uint16_t ComputeParityBit(uint16_t word_no_parity)
{
    uint16_t v = word_no_parity & 0x7FFFu;
    uint8_t parity = 0;
    while (v) {
        parity ^= (v & 1u);
        v >>= 1;
    }
    /* AS5048A uses EVEN parity: parity bit makes total number of 1s even */
    return (uint16_t)(parity << 15);
}

static bool CheckParityOk(uint16_t word)
{
    uint8_t parity = 0;
    uint16_t v = word;
    while (v) {
        parity ^= (v & 1u);
        v >>= 1;
    }
    /* If parity is correct, XOR of all 16 bits (including the parity bit) is 0 */
    return (parity == 0);
}

/* ---- Public API ---- */

void AS5048A_Init(AS5048A_HandleTypeDef *dev, SPI_HandleTypeDef *hspi,
                   GPIO_TypeDef *cs_port, uint16_t cs_pin)
{
    dev->hspi = hspi;
    dev->cs_port = cs_port;
    dev->cs_pin = cs_pin;
    CS_High(dev); /* idle state: CS not asserted */
}

/* Command frame bit layout (datasheet Table 9): bit15=parity, bit14=R/W
 * (1=read, 0=write), bits13:0 = register address. Build it explicitly
 * every time so there's no ambiguity about what's being OR'd together. */
#define AS5048A_READ_BIT   (1u << 14)

static inline uint16_t BuildReadCommand(uint16_t reg_addr14)
{
    return (uint16_t)(AS5048A_READ_BIT | (reg_addr14 & 0x3FFFu));
}

/* Minimum CS-high recovery time between frames. AS5048A datasheet specifies
 * this in the ~350ns range; a full microsecond gives comfortable margin and
 * is cheap relative to a 100ms sample loop. Without this, back-to-back HAL
 * calls can toggle CS faster than the sensor's internal shift register can
 * reset, corrupting the very next frame (seen as all-0x0000 or all-0x3FFF
 * reads that still happen to pass the parity check). */
static void CS_RecoveryDelay(void)
{
    /* Crude but effective busy-wait; swap for a DWT cycle-counter delay
     * if you need tighter timing elsewhere in your project. */
    for (volatile uint32_t i = 0; i < 200; i++) { __NOP(); }
}

bool AS5048A_TransferFrame(AS5048A_HandleTypeDef *dev, uint16_t command, uint16_t *response)
{
    uint16_t tx = command | ComputeParityBit(command);
    uint16_t rx = 0;

    /* AS5048A expects big-endian 16-bit words on the wire; HAL_SPI with
     * 16-bit data size handles byte order internally as long as the SPI
     * peripheral is configured for 16-bit data frames (see integration
     * notes / CubeMX config in the accompanying instructions). */
    CS_Low(dev);
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(
        dev->hspi, (uint8_t *)&tx, (uint8_t *)&rx, 1, AS5048A_SPI_TIMEOUT_MS);

    /* Confirm the SPI peripheral is fully idle before releasing CS -- on
     * some F4 revisions HAL_SPI_TransmitReceive can return before the very
     * last bit has finished shifting out. */
    while (HAL_SPI_GetState(dev->hspi) != HAL_SPI_STATE_READY) { /* wait */ }

    CS_High(dev);
    CS_RecoveryDelay();

    if (status != HAL_OK) {
        return false;
    }

    if (response != NULL) {
        *response = rx;
    }
    return true;
}

bool AS5048A_ReadAngle(AS5048A_HandleTypeDef *dev, AS5048A_AngleTypeDef *result)
{
    uint16_t rx = 0;

    /* Frame 1: request the ANGLE register */
    if (!AS5048A_TransferFrame(dev, BuildReadCommand(AS5048A_REG_ANGLE), &rx)) {
        return false;
    }

    /* Frame 2: NOP, clocks out the response to the frame above */
    if (!AS5048A_TransferFrame(dev, BuildReadCommand(AS5048A_REG_NOP), &rx)) {
        return false;
    }

    result->parity_ok  = CheckParityOk(rx);
    result->error_flag = (rx & AS5048A_ERROR_BIT) != 0;
    result->raw_angle   = rx & AS5048A_DATA_MASK;
    result->angle_deg   = (result->raw_angle * 360.0f) / 16384.0f;

    return true;
}

bool AS5048A_ReadDiagAGC(AS5048A_HandleTypeDef *dev, uint16_t *raw_out)
{
    uint16_t rx = 0;
    if (!AS5048A_TransferFrame(dev, BuildReadCommand(AS5048A_REG_DIAG_AGC), &rx)) {
        return false;
    }
    if (!AS5048A_TransferFrame(dev, BuildReadCommand(AS5048A_REG_NOP), &rx)) {
        return false;
    }
    *raw_out = rx & AS5048A_DATA_MASK;
    return true;
}

bool AS5048A_ReadMagnitude(AS5048A_HandleTypeDef *dev, uint16_t *raw_out)
{
    uint16_t rx = 0;
    if (!AS5048A_TransferFrame(dev, BuildReadCommand(AS5048A_REG_MAGNITUDE), &rx)) {
        return false;
    }
    if (!AS5048A_TransferFrame(dev, BuildReadCommand(AS5048A_REG_NOP), &rx)) {
        return false;
    }
    *raw_out = rx & AS5048A_DATA_MASK;
    return true;
}

bool AS5048A_ClearErrorFlag(AS5048A_HandleTypeDef *dev, uint16_t *error_reg_out)
{
    uint16_t rx = 0;
    /* Clearing the error flag register is a READ of address 0x0001 per the
     * datasheet -- reading it clears it and returns the prior error content. */
    if (!AS5048A_TransferFrame(dev, BuildReadCommand(AS5048A_REG_CLEAR_ERROR_FLAG), &rx)) {
        return false;
    }
    if (!AS5048A_TransferFrame(dev, BuildReadCommand(AS5048A_REG_NOP), &rx)) {
        return false;
    }
    if (error_reg_out != NULL) {
        *error_reg_out = rx & AS5048A_DATA_MASK;
    }
    return true;
}
