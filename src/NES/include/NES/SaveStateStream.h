#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <cstdint>
#include <cstring>
#include <type_traits>

namespace NES {

class SaveStateStream {
    public:
        // Primitivos (uint8_t, uint16_t, int, bool, double, enums...)
        template <typename T>
        static void writeVal(std::ostream& out, const T& val) {
            static_assert(std::is_trivially_copyable_v<T>, "Tipo debe ser trivialmente copiable");
            out.write(reinterpret_cast<const char*>(&val), sizeof(T));
        }

        template <typename T>
        static void readVal(std::istream& in, T& val) {
            static_assert(std::is_trivially_copyable_v<T>, "Tipo debe ser trivialmente copiable");
            in.read(reinterpret_cast<char*>(&val), sizeof(T));
        }

        // std::array de tamaño fijo (RAM, CIRAM, Paletas, OAM, registros de Mapper)
        template <typename T, size_t N>
        static void writeArray(std::ostream& out, const std::array<T, N>& arr) {
            out.write(reinterpret_cast<const char*>(arr.data()), N * sizeof(T));
        }

        template <typename T, size_t N>
        static void readArray(std::istream& in, std::array<T, N>& arr) {
            in.read(reinterpret_cast<char*>(arr.data()), N * sizeof(T));
        }

        // std::vector dinámico (PRG-RAM, CHR-RAM)
        template <typename T>
        static void writeVector(std::ostream& out, const std::vector<T>& vec) {
            uint64_t size = static_cast<uint64_t>(vec.size());
            writeVal(out, size);
            if (size > 0) {
                out.write(reinterpret_cast<const char*>(vec.data()), size * sizeof(T));
            }
        }

        template <typename T>
        static void readVector(std::istream& in, std::vector<T>& vec) {
            uint64_t size = 0;
            readVal(in, size);
            vec.resize(static_cast<size_t>(size));
            if (size > 0) {
                in.read(reinterpret_cast<char*>(vec.data()), size * sizeof(T));
            }
        }
    };

}