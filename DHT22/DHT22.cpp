#include "DHT22.hpp"

DHT22::DHT22(GPIO_TypeDef* gpio,
             uint16_t pin,
             DHT22Type type)
    : m_gpio(gpio),
      m_pin(pin),
      m_type(type),
      m_state(State::Idle),
      m_status(HAL_OK),
      m_data{0.0f, 0.0f},
      m_lastReadTick(0U),
      m_stateStartMicros(0U),
      m_bitStartMicros(0U),
      m_bitCount(0U),
      m_hasValidData(false),
      m_newData(false)
{
    InitializeDWT();

    for (uint8_t i = 0U; i < 5U; ++i)
    {
        m_frame[i] = 0U;
    }

    ConfigureAsInput();
}

void DHT22::InitializeDWT()
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t DHT22::Micros()
{
    /*
     * SystemCoreClock must contain the final CPU frequency.
     *
     * For STM32F103 at 72 MHz:
     *   72 CPU cycles = 1 microsecond
     */
    const uint32_t cyclesPerMicrosecond =
        SystemCoreClock / 1000000U;

    if (cyclesPerMicrosecond == 0U)
    {
        return 0U;
    }

    return DWT->CYCCNT / cyclesPerMicrosecond;
}

uint32_t DHT22::ElapsedMicros(uint32_t start)
{
    /*
     * Unsigned subtraction is safe across 32-bit timer overflow.
     */
    return Micros() - start;
}

void DHT22::ConfigureAsOutput()
{
    m_gpioInit.Pin = m_pin;
    m_gpioInit.Mode = GPIO_MODE_OUTPUT_OD;
    m_gpioInit.Pull = GPIO_NOPULL;
    m_gpioInit.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(m_gpio, &m_gpioInit);
}

void DHT22::ConfigureAsInput()
{
    m_gpioInit.Pin = m_pin;
    m_gpioInit.Mode = GPIO_MODE_INPUT;
    m_gpioInit.Pull = GPIO_NOPULL;
    m_gpioInit.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(m_gpio, &m_gpioInit);
}

void DHT22::SetPinLow()
{
    HAL_GPIO_WritePin(m_gpio, m_pin, GPIO_PIN_RESET);
}

void DHT22::SetPinHigh()
{
    /*
     * In open-drain mode, writing SET releases the line.
     * The external pull-up resistor raises it to VCC.
     */
    HAL_GPIO_WritePin(m_gpio, m_pin, GPIO_PIN_SET);
}

bool DHT22::ReadPin() const
{
    return HAL_GPIO_ReadPin(m_gpio, m_pin) == GPIO_PIN_SET;
}

void DHT22::EnterState(State newState)
{
    m_state = newState;
    m_stateStartMicros = Micros();
}

void DHT22::ResetFrame()
{
    for (uint8_t i = 0U; i < 5U; ++i)
    {
        m_frame[i] = 0U;
    }

    m_bitCount = 0U;
}

void DHT22::Fail(HAL_StatusTypeDef status)
{
    m_status = status;

    /*
     * Release the bus even if the transaction fails.
     */
    ConfigureAsInput();

    m_lastReadTick = HAL_GetTick();
    m_state = State::Idle;
}

bool DHT22::StartRead()
{
    if (m_state != State::Idle)
    {
        return false;
    }

    /*
     * Correct SysTick-based minimum interval check.
     *
     * The first read is allowed immediately because m_lastReadTick
     * is initialized to DHTREADTIME.
     */
    const uint32_t now = HAL_GetTick();

    if ((now - m_lastReadTick) < DHTREADTIME)
    {
        return false;
    }

    ResetFrame();

    m_status = HAL_BUSY;
    m_newData = false;

    ConfigureAsOutput();
    SetPinLow();

    m_stateStartMicros = Micros();

    if (m_type == DHT2X)
    {
        EnterState(State::StartLow);
    }
    else
    {
        /*
         * For DHT11, StartLow is measured using HAL_GetTick().
         * We keep the start tick in m_lastReadTick temporarily.
         */
        m_lastReadTick = now;
        EnterState(State::StartLow);
    }

    return true;
}

void DHT22::Process()
{
    if (m_state == State::Idle)
    {
        return;
    }

    switch (m_state)
    {
        case State::StartLow:
        {
            if (m_type == DHT2X)
            {
                if (ElapsedMicros(m_stateStartMicros) >=
                    DHT22_START_LOW_US)
                {
                    SetPinHigh();
                    EnterState(State::StartHigh);
                }
            }
            else
            {
                /*
                 * DHT11 requires at least 18 ms low.
                 * This check is non-blocking.
                 */
                if ((HAL_GetTick() - m_lastReadTick) >=
                    DHT11_START_LOW_MS)
                {
                    SetPinHigh();
                    EnterState(State::StartHigh);
                }
            }

            break;
        }

        case State::StartHigh:
        {
            if (ElapsedMicros(m_stateStartMicros) >=
                DHT22_START_HIGH_US)
            {
                ConfigureAsInput();
                EnterState(State::WaitResponseLow);
            }

            break;
        }

        case State::WaitResponseLow:
        {
            /*
             * Sensor should pull the line low.
             */
            if (!ReadPin())
            {
                EnterState(State::WaitResponseHigh);
            }
            else if (ElapsedMicros(m_stateStartMicros) >
                     RESPONSE_TIMEOUT_US)
            {
                Fail(HAL_TIMEOUT);
            }

            break;
        }

        case State::WaitResponseHigh:
        {
            /*
             * Sensor should then pull the line high.
             */
            if (ReadPin())
            {
                EnterState(State::WaitBitLow);
            }
            else if (ElapsedMicros(m_stateStartMicros) >
                     RESPONSE_TIMEOUT_US)
            {
                Fail(HAL_TIMEOUT);
            }

            break;
        }

        case State::WaitBitLow:
        {
            /*
             * Every data bit starts with approximately 50 us low.
             */
            if (ReadPin())
            {
                m_bitStartMicros = Micros();
                EnterState(State::WaitBitHigh);
            }
            else if (ElapsedMicros(m_stateStartMicros) >
                     BIT_TIMEOUT_US)
            {
                Fail(HAL_TIMEOUT);
            }

            break;
        }

        case State::WaitBitHigh:
        {
            /*
             * The duration of the high pulse represents the bit:
             *
             *   approximately 26..28 us -> 0
             *   approximately 70 us     -> 1
             */
            if (!ReadPin())
            {
                const uint32_t highTime =
                    ElapsedMicros(m_bitStartMicros);

                StoreBit(highTime > BIT_THRESHOLD_US);

                if (m_bitCount >= 40U)
                {
                    EnterState(State::Complete);
                }
                else
                {
                    EnterState(State::WaitBitLow);
                }
            }
            else if (ElapsedMicros(m_bitStartMicros) >
                     BIT_TIMEOUT_US)
            {
                Fail(HAL_TIMEOUT);
            }

            break;
        }

        case State::Complete:
        {
            DecodeFrame();

            ConfigureAsInput();

            m_lastReadTick = HAL_GetTick();
            m_state = State::Idle;

            break;
        }

        case State::Error:
        case State::Idle:
        default:
        {
            ConfigureAsInput();
            m_state = State::Idle;
            break;
        }
    }
}

void DHT22::StoreBit(bool bit)
{
    if (m_bitCount >= 40U)
    {
        return;
    }

    const uint8_t byteIndex = m_bitCount / 8U;
    const uint8_t bitIndex = 7U - (m_bitCount % 8U);

    if (bit)
    {
        m_frame[byteIndex] |=
            static_cast<uint8_t>(1U << bitIndex);
    }
    else
    {
        m_frame[byteIndex] &=
            static_cast<uint8_t>(~(1U << bitIndex));
    }

    ++m_bitCount;
}

void DHT22::DecodeFrame()
{
    const uint8_t humidityHigh = m_frame[0];
    const uint8_t humidityLow = m_frame[1];
    const uint8_t temperatureHigh = m_frame[2];
    const uint8_t temperatureLow = m_frame[3];
    const uint8_t checksum = m_frame[4];

    const uint8_t calculatedChecksum =
        static_cast<uint8_t>(
            humidityHigh +
            humidityLow +
            temperatureHigh +
            temperatureLow);

    if (checksum != calculatedChecksum)
    {
        /*
         * Do not use corrupted data.
         */
        m_status = HAL_ERROR;
        return;
    }

    float humidity = 0.0f;
    float temperature = 0.0f;

    if (m_type == DHT2X)
    {
        const uint16_t rawHumidity =
            static_cast<uint16_t>(
                (static_cast<uint16_t>(humidityHigh) << 8U) |
                humidityLow);

        const uint16_t rawTemperature =
            static_cast<uint16_t>(
                (static_cast<uint16_t>(temperatureHigh & 0x7FU) << 8U) |
                temperatureLow);

        humidity = static_cast<float>(rawHumidity) / 10.0f;
        temperature = static_cast<float>(rawTemperature) / 10.0f;

        /*
         * DHT22 temperature sign bit.
         */
        if ((temperatureHigh & 0x80U) != 0U)
        {
            temperature = -temperature;
        }
    }
    else
    {
        humidity = static_cast<float>(humidityHigh);
        temperature = static_cast<float>(temperatureHigh);
    }

    if (humidity < 0.0f || humidity > 100.0f ||
        temperature < -50.0f || temperature > 100.0f)
    {
        m_status = HAL_ERROR;
        return;
    }

    m_data.Hum = humidity;
    m_data.Temp = temperature;

    m_status = HAL_OK;
    m_hasValidData = true;
    m_newData = true;
}

bool DHT22::IsBusy() const
{
    return m_state != State::Idle;
}

bool DHT22::IsIdle() const
{
    return m_state == State::Idle;
}

bool DHT22::HasNewData() const
{
    return m_newData;
}

void DHT22::ClearNewData()
{
    m_newData = false;
}

DHT22::State DHT22::GetState() const
{
    return m_state;
}

HAL_StatusTypeDef DHT22::GetStatus() const
{
    return m_status;
}

Temp_Hum DHT22::GetData() const
{
    return m_data;
}

float DHT22::GetTemperature(bool fahrenheit) const
{
    if (!fahrenheit)
    {
        return m_data.Temp;
    }

    return CelsiusToFahrenheit(m_data.Temp);
}

float DHT22::GetHumidity() const
{
    return m_data.Hum;
}

float DHT22::CelsiusToFahrenheit(float celsius)
{
    return (celsius * 1.8f) + 32.0f;
}

DHT22Type DHT22::GetDHTType() const
{
    return m_type;
}

void DHT22::SetDHTType(DHT22Type type)
{
    if (m_state == State::Idle)
    {
        m_type = type;
    }
}
