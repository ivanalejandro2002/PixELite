#pragma once
namespace NES
{
    class OpenBus
    {
        public:
            OpenBus();

            uint8_t getValue();
            void update(uint8_t value);
        private:
            uint8_t lastValue;
    };
}