#include <NES/APU/AudioRingBuffer.h>

namespace NES
{
    AudioRingBuffer::AudioRingBuffer(size_t cap): capacity(cap), buffer(cap, 0.0f) {}

    void AudioRingBuffer::push(float sample)
    {
        buffer[tail] = sample;
        tail = (tail + 1) % capacity;
        
        if (size < capacity)
        {
            size++;
        }
        else
        {
            // Si el búfer está lleno, avanzamos el head para sobrescribir 
            // la muestra más antigua y mantener el flujo en tiempo real sin perder lo nuevo.
            head = (head + 1) % capacity;
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

    size_t AudioRingBuffer::readResampled(float *dest, size_t requestedSamples, double ratio, double& srcPos)
    {
        size_t samplesRead = 0;
        
        for (size_t i = 0; i < requestedSamples; ++i)
        {
            // Necesitamos al menos 2 muestras disponibles para la interpolación lineal (index1 e index2)
            if (srcPos + 1.0 >= static_cast<double>(size)) 
            {
                break; // No hay suficientes datos todavía, salimos de forma limpia
            }

            size_t index1 = (head + static_cast<size_t>(srcPos)) % capacity;
            size_t index2 = (head + static_cast<size_t>(srcPos) + 1) % capacity;
            
            double frac = srcPos - static_cast<size_t>(srcPos);

            // Interpolación lineal suave
            float sample1 = buffer[index1];
            float sample2 = buffer[index2];
            dest[i] = sample1 + static_cast<float>(frac * (sample2 - sample1));

            srcPos += ratio;
            samplesRead++;
        }

        // Limpiamos del búfer físico únicamente las muestras enteras que ya consumimos con éxito
        size_t wholeSamplesConsumed = static_cast<size_t>(srcPos);
        if (wholeSamplesConsumed > 0)
        {
            head = (head + wholeSamplesConsumed) % capacity;
            size -= wholeSamplesConsumed;
            srcPos -= static_cast<double>(wholeSamplesConsumed);
        }

        return samplesRead; // ¡Devuelve estrictamente la cantidad real leída!
    }

    void AudioRingBuffer::clear()
    {
        head = 0;
        tail = 0;
        size = 0;
        std::fill(buffer.begin(), buffer.end(), 0.0f);
    }

    size_t AudioRingBuffer::available() const
    {
        return size;
    }
}