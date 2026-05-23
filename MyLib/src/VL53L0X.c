/**
 * @file VL53L0X.c
 * @brief Implementación de la librería del sensor VL53L0X
 */

#include "VL53L0X.h"
#include <string.h>

//============================================================================
// REGISTROS DEL SENSOR
//============================================================================
#define SYSRANGE_START 0x00
#define SYSTEM_SEQUENCE_CONFIG 0x01
#define SYSTEM_INTERMEASUREMENT_PERIOD 0x04
#define SYSTEM_INTERRUPT_CONFIG_GPIO 0x0A
#define SYSTEM_INTERRUPT_CLEAR 0x0B
#define RESULT_INTERRUPT_STATUS 0x13
#define RESULT_RANGE_STATUS 0x14
#define I2C_SLAVE_DEVICE_ADDRESS 0x8A
#define VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV 0x89
#define MSRC_CONFIG_CONTROL 0x60
#define FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT 0x44
#define PRE_RANGE_CONFIG_VCSEL_PERIOD 0x50
#define FINAL_RANGE_CONFIG_VCSEL_PERIOD 0x70
#define DYNAMIC_SPAD_REF_EN_START_OFFSET 0x4F
#define DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD 0x4E
#define GLOBAL_CONFIG_SPAD_ENABLES_REF_0 0xB0
#define GLOBAL_CONFIG_REF_EN_START_SELECT 0xB6
#define OSC_CALIBRATE_VAL 0xF8
#define GPIO_HV_MUX_ACTIVE_HIGH 0x84
#define FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI 0x71
#define FINAL_RANGE_CONFIG_TIMEOUT_MACROP_LO 0x72
#define PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI 0x51
#define PRE_RANGE_CONFIG_VALID_PHASE_HIGH 0x57
#define PRE_RANGE_CONFIG_VALID_PHASE_LOW 0x56
#define MSRC_CONFIG_TIMEOUT_MACROP 0x46
#define GLOBAL_CONFIG_VCSEL_WIDTH 0x32
#define ALGO_PHASECAL_CONFIG_TIMEOUT 0x30
#define ALGO_PHASECAL_LIM 0x30

// Macros auxiliares
#define encodeVcselPeriod(period_pclks) (((period_pclks) >> 1) - 1)

//============================================================================
// FUNCIONES I2C PRIVADAS
//============================================================================

static void write_reg(VL53L0X_t *dev, uint8_t reg, uint8_t value)
{
    HAL_I2C_Mem_Write(dev->hi2c, dev->address, reg, 1, &value, 1, 10);
}

static uint8_t read_reg(VL53L0X_t *dev, uint8_t reg)
{
    uint8_t value = 0;
    HAL_I2C_Mem_Read(dev->hi2c, dev->address, reg, 1, &value, 1, 10);
    return value;
}

static uint16_t read_reg16(VL53L0X_t *dev, uint8_t reg)
{
    uint8_t buf[2];
    HAL_I2C_Mem_Read(dev->hi2c, dev->address, reg, 1, buf, 2, 10);
    return (buf[0] << 8) | buf[1];
}

static void write_reg16(VL53L0X_t *dev, uint8_t reg, uint16_t value)
{
    uint8_t buf[2] = {value >> 8, value & 0xFF};
    HAL_I2C_Mem_Write(dev->hi2c, dev->address, reg, 1, buf, 2, 10);
}

static void write_multi(VL53L0X_t *dev, uint8_t reg, uint8_t *src, uint8_t count)
{
    HAL_I2C_Mem_Write(dev->hi2c, dev->address, reg, 1, src, count, 10);
}

static void read_multi(VL53L0X_t *dev, uint8_t reg, uint8_t *dst, uint8_t count)
{
    HAL_I2C_Mem_Read(dev->hi2c, dev->address, reg, 1, dst, count, 10);
}

//============================================================================
// FILTRO DE MEDIANA
//============================================================================

static uint16_t filter_median(VL53L0X_t *dev, uint16_t new_value)
{
    dev->filter_buffer[dev->filter_index] = new_value;
    dev->filter_index = (dev->filter_index + 1) % 5;

    if (dev->filter_count < 5)
    {
        dev->filter_count++;
    }

    uint16_t temp[5];
    for (int i = 0; i < dev->filter_count; i++)
    {
        temp[i] = dev->filter_buffer[i];
    }

    for (int i = 0; i < dev->filter_count - 1; i++)
    {
        for (int j = i + 1; j < dev->filter_count; j++)
        {
            if (temp[i] > temp[j])
            {
                uint16_t aux = temp[i];
                temp[i] = temp[j];
                temp[j] = aux;
            }
        }
    }

    dev->last_filtered = temp[dev->filter_count / 2];
    return dev->last_filtered;
}

//============================================================================
// FUNCIONES DE INICIALIZACIÓN PRIVADAS
//============================================================================

static bool get_spad_info(VL53L0X_t *dev, uint8_t *count, bool *aperture)
{
    uint8_t tmp;
    uint32_t start = millis();

    write_reg(dev, 0x80, 0x01);
    write_reg(dev, 0xFF, 0x01);
    write_reg(dev, 0x00, 0x00);
    write_reg(dev, 0xFF, 0x06);
    write_reg(dev, 0x83, read_reg(dev, 0x83) | 0x04);
    write_reg(dev, 0xFF, 0x07);
    write_reg(dev, 0x81, 0x01);
    write_reg(dev, 0x80, 0x01);
    write_reg(dev, 0x94, 0x6b);
    write_reg(dev, 0x83, 0x00);

    while (read_reg(dev, 0x83) == 0x00)
    {
        if ((millis() - start) > 500)
            return false;
    }

    write_reg(dev, 0x83, 0x01);
    tmp = read_reg(dev, 0x92);

    *count = tmp & 0x7F;
    *aperture = (tmp >> 7) & 0x01;

    write_reg(dev, 0x81, 0x00);
    write_reg(dev, 0xFF, 0x06);
    write_reg(dev, 0x83, read_reg(dev, 0x83) & ~0x04);
    write_reg(dev, 0xFF, 0x01);
    write_reg(dev, 0x00, 0x01);
    write_reg(dev, 0xFF, 0x00);
    write_reg(dev, 0x80, 0x00);

    return true;
}

static bool perform_calibration(VL53L0X_t *dev, uint8_t vhv_byte)
{
    uint32_t start = millis();

    write_reg(dev, SYSRANGE_START, 0x01 | vhv_byte);

    while ((read_reg(dev, RESULT_INTERRUPT_STATUS) & 0x07) == 0)
    {
        if ((millis() - start) > 500)
            return false;
    }

    write_reg(dev, SYSTEM_INTERRUPT_CLEAR, 0x01);
    write_reg(dev, SYSRANGE_START, 0x00);

    return true;
}

//============================================================================
// CONFIGURACIÓN DE MODO
//============================================================================

void VL53L0X_SetHighSpeedMode(VL53L0X_t *dev)
{
    // Timing budget de 20ms (HIGH SPEED)
    write_reg16(dev, FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI, 0x0019);

    // Pre-range: periodo 12 (mínimo)
    write_reg(dev, PRE_RANGE_CONFIG_VCSEL_PERIOD, encodeVcselPeriod(12));

    // Final-range: periodo 8 (mínimo)
    write_reg(dev, FINAL_RANGE_CONFIG_VCSEL_PERIOD, encodeVcselPeriod(8));

    // Reducir límite de señal para mediciones más rápidas
    write_reg16(dev, FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, 0x0010);

    // Reducir timeout del pre-range
    write_reg16(dev, PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI, 0x0009);

    dev->timeout_ms = 30;
}

void VL53L0X_SetDefaultMode(VL53L0X_t *dev)
{
    // Timing budget de 33ms (DEFAULT)
    write_reg16(dev, FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI, 0x0022);

    // Pre-range: periodo 14 (estándar)
    write_reg(dev, PRE_RANGE_CONFIG_VCSEL_PERIOD, encodeVcselPeriod(14));

    // Final-range: periodo 10 (estándar)
    write_reg(dev, FINAL_RANGE_CONFIG_VCSEL_PERIOD, encodeVcselPeriod(10));

    // Restaurar límite de señal
    write_reg16(dev, FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, 0x0020);

    write_reg16(dev, PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI, 0x0019);

    dev->timeout_ms = 50;
}

//============================================================================
// FUNCIONES PÚBLICAS
//============================================================================

bool VL53L0X_Init(VL53L0X_t *dev, I2C_HandleTypeDef *hi2c, bool io_2v8)
{
    memset(dev, 0, sizeof(VL53L0X_t));
    dev->hi2c = hi2c;
    dev->address = 0x52;
    dev->io_2v8 = io_2v8;
    dev->timeout_ms = 50;
    dev->is_measuring = false;
    dev->last_timeout = false;
    dev->last_measurement_time = 0;

    // Pequeña pausa
    uint32_t start = millis();
    while ((millis() - start) < 50)
        ;

    // Configurar modo 2V8
    if (dev->io_2v8)
    {
        uint8_t val = read_reg(dev, VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV);
        write_reg(dev, VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV, val | 0x01);
    }

    // Configuración básica
    write_reg(dev, 0x88, 0x00);
    write_reg(dev, 0x80, 0x01);
    write_reg(dev, 0xFF, 0x01);
    write_reg(dev, 0x00, 0x00);
    dev->stop_variable = read_reg(dev, 0x91);
    write_reg(dev, 0x00, 0x01);
    write_reg(dev, 0xFF, 0x00);
    write_reg(dev, 0x80, 0x00);

    // Deshabilitar límites
    write_reg(dev, MSRC_CONFIG_CONTROL, read_reg(dev, MSRC_CONFIG_CONTROL) | 0x12);

    // Límite de señal
    write_reg16(dev, FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, 0x0020);

    // Configurar secuencia
    write_reg(dev, SYSTEM_SEQUENCE_CONFIG, 0xFF);

    // Obtener información SPAD
    uint8_t spad_count;
    bool spad_aperture;
    if (!get_spad_info(dev, &spad_count, &spad_aperture))
    {
        return false;
    }

    // Configurar SPADs
    uint8_t ref_spad_map[6];
    read_multi(dev, GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);

    write_reg(dev, 0xFF, 0x01);
    write_reg(dev, DYNAMIC_SPAD_REF_EN_START_OFFSET, 0x00);
    write_reg(dev, DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD, 0x2C);
    write_reg(dev, 0xFF, 0x00);
    write_reg(dev, GLOBAL_CONFIG_REF_EN_START_SELECT, 0xB4);

    uint8_t first_spad = spad_aperture ? 12 : 0;
    uint8_t spads_enabled = 0;

    for (uint8_t i = 0; i < 48; i++)
    {
        if (i < first_spad || spads_enabled == spad_count)
        {
            ref_spad_map[i / 8] &= ~(1 << (i % 8));
        }
        else if ((ref_spad_map[i / 8] >> (i % 8)) & 0x01)
        {
            spads_enabled++;
        }
    }

    write_multi(dev, GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);

    // Configurar interrupciones
    write_reg(dev, SYSTEM_INTERRUPT_CONFIG_GPIO, 0x04);
    uint8_t gpio_val = read_reg(dev, GPIO_HV_MUX_ACTIVE_HIGH);
    write_reg(dev, GPIO_HV_MUX_ACTIVE_HIGH, gpio_val & ~0x10);
    write_reg(dev, SYSTEM_INTERRUPT_CLEAR, 0x01);

    // Deshabilitar MSRC y TCC
    write_reg(dev, SYSTEM_SEQUENCE_CONFIG, 0xE8);

    // Calibración VHV
    write_reg(dev, SYSTEM_SEQUENCE_CONFIG, 0x01);
    if (!perform_calibration(dev, 0x40))
        return false;

    // Calibración de fase
    write_reg(dev, SYSTEM_SEQUENCE_CONFIG, 0x02);
    if (!perform_calibration(dev, 0x00))
        return false;

    // Restaurar configuración
    write_reg(dev, SYSTEM_SEQUENCE_CONFIG, 0xE8);

    // Inicializar filtro
    dev->filter_index = 0;
    dev->filter_count = 0;
    dev->last_filtered = 0;

    return true;
}

void VL53L0X_StartMeasurement(VL53L0X_t *dev)
{
    if (dev->is_measuring)
        return;

    write_reg(dev, 0x80, 0x01);
    write_reg(dev, 0xFF, 0x01);
    write_reg(dev, 0x00, 0x00);
    write_reg(dev, 0x91, dev->stop_variable);
    write_reg(dev, 0x00, 0x01);
    write_reg(dev, 0xFF, 0x00);
    write_reg(dev, 0x80, 0x00);
    write_reg(dev, SYSRANGE_START, 0x01);

    dev->measurement_start_time = millis();
    dev->is_measuring = true;
}

bool VL53L0X_StartMeasurementIfReady(VL53L0X_t *dev, uint32_t interval_ms)
{
    uint32_t ahora = millis();

    if (!dev->is_measuring && (ahora - dev->last_measurement_time) >= interval_ms)
    {
        VL53L0X_StartMeasurement(dev);
        dev->last_measurement_time = ahora;
        return true;
    }
    return false;
}

bool VL53L0X_IsReady(VL53L0X_t *dev)
{
    if (!dev->is_measuring)
        return false;

    if ((millis() - dev->measurement_start_time) >= dev->timeout_ms)
    {
        dev->is_measuring = false;
        dev->last_timeout = true;
        return true;
    }

    if (!(read_reg(dev, SYSRANGE_START) & 0x01))
    {
        dev->is_measuring = false;
        dev->last_timeout = false;
        return true;
    }

    return false;
}

uint16_t VL53L0X_GetRawDistance(VL53L0X_t *dev)
{
    uint16_t distance = read_reg16(dev, RESULT_RANGE_STATUS + 10);
    write_reg(dev, SYSTEM_INTERRUPT_CLEAR, 0x01);
    return distance;
}

uint16_t VL53L0X_GetDistance(VL53L0X_t *dev)
{
    uint16_t raw = VL53L0X_GetRawDistance(dev);

    if (raw < 20)
        raw = 20;
    if (raw > 1200)
        raw = dev->last_filtered > 0 ? dev->last_filtered : 1200;

    return filter_median(dev, raw);
}

bool VL53L0X_TimeoutOccurred(VL53L0X_t *dev)
{
    bool result = dev->last_timeout;
    dev->last_timeout = false;
    return result;
}

void VL53L0X_SetTimeout(VL53L0X_t *dev, uint16_t timeout_ms)
{
    dev->timeout_ms = timeout_ms;
}