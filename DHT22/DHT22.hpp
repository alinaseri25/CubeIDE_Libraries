#ifndef DHT22_HPP
#define DHT22_HPP

#include "stm32f1xx_hal.h"
#include <stdint.h>

#ifndef DHTREADTIME
#define DHTREADTIME 1000U
#endif

typedef enum
{
    DHT2X = 0x00,
    DHT1X = 0x01
} DHT22Type;

struct Temp_Hum
{
    float Temp;
    float Hum;
};

class DHT22
{
public:
    enum class State : uint8_t
    {
        Idle = 0,
        StartLow,
        StartHigh,
        WaitResponseLow,
        WaitResponseHigh,
        WaitBitLow,
        WaitBitHigh,
        Complete,
        Error
    };

public:
    DHT22(GPIO_TypeDef* gpio,
          uint16_t pin,
          DHT22Type type = DHT2X);

    /*
     * Starts a new asynchronous measurement.
     *
     * Returns:
     *   true  - measurement started
     *   false - sensor is busy or minimum interval has not elapsed
     */
    bool StartRead();

    /*
     * Advances the state machine.
     *
     * Must be called frequently from the main loop.
     * It never waits and never blocks.
     */
    void Process();

    bool IsBusy() const;
    bool IsIdle() const;
    bool HasNewData() const;
    void ClearNewData();

    State GetState() const;
    HAL_StatusTypeDef GetStatus() const;

    Temp_Hum GetData() const;
    float GetTemperature(bool fahrenheit = false) const;
    float GetHumidity() const;

    DHT22Type GetDHTType() const;
    void SetDHTType(DHT22Type type);

private:
    static constexpr uint32_t DHT22_START_LOW_US       = 500U;
    static constexpr uint32_t DHT22_START_HIGH_US      = 30U;
    static constexpr uint32_t DHT11_START_LOW_MS       = 18U;

    static constexpr uint32_t RESPONSE_TIMEOUT_US      = 200U;
    static constexpr uint32_t BIT_TIMEOUT_US           = 200U;
    static constexpr uint32_t BIT_THRESHOLD_US         = 40U;

private:
    void ConfigureAsOutput();
    void ConfigureAsInput();

    void SetPinLow();
    void SetPinHigh();

    bool ReadPin() const;

    void EnterState(State newState);
    void Fail(HAL_StatusTypeDef status);

    void ResetFrame();
    void StoreBit(bool bit);
    void DecodeFrame();

    static void InitializeDWT();
    static uint32_t Micros();
    static uint32_t ElapsedMicros(uint32_t start);

    static float CelsiusToFahrenheit(float celsius);

private:
    GPIO_TypeDef* m_gpio;
    uint16_t m_pin;

    GPIO_InitTypeDef m_gpioInit;

    DHT22Type m_type;

    State m_state;
    HAL_StatusTypeDef m_status;

    Temp_Hum m_data;

    uint32_t m_lastReadTick;
    uint32_t m_stateStartMicros;
    uint32_t m_bitStartMicros;

    uint8_t m_frame[5];
    uint8_t m_bitCount;

    bool m_hasValidData;
    bool m_newData;
};

#endif
