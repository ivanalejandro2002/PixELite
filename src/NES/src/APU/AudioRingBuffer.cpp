#include <NES/APU/AudioRingBuffer.h>

namespace NES
{
    AudioRingBuffer::AudioRingBuffer(size_t cap): capacity(cap), buffer(cap, 0.0f) {}

    void AudioRingBuffer::push(float sample)
    {
        if(size < capacity)
        {
            buffer[tail] = sample;
            tail = (tail + 1) % capacity;
            size++;
        }
    }

    size_t AudioRingBuffer::read(float *dest, size_t maxSamples)
    {
        size_t samplesToRead = std::min(maxSamples, size);
        for(size_t i = 0; i < samplesToRead; ++i)
        {
            dest[i] = buffer[head];
            head = (head + 1) % capacity;
            size--;
        }
        return samplesToRead;
    }

    size_t AudioRingBuffer::available() const
    {
        return size;
    }
}