#include <vector>
#include <mutex>

namespace NES
{
    class AudioRingBuffer
    {
        private:
            std::vector<float> buffer;
            size_t head = 0;
            size_t tail = 0;
            size_t capacity = 0;
            size_t size = 0;

        public:
            AudioRingBuffer(size_t cap);

            void push(float sample);
            size_t read(float *dest, size_t maxSamples);
            size_t readResampled(float *dest, size_t requestedSamples, double ratio, double& fractionalPosition);
            void clear();
            size_t available() const;
    };
}