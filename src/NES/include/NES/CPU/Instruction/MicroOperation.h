#pragma once

#include <cstdint>
#include <array>
#include <string_view>

namespace NES::CPU
{
    enum class MicroOperation : uint8_t
    {
        // Control general
        FetchOpcode,
        FetchOpcodeAndDiscard,
        FetchOpcodeDiscardAndExec,

        DummyRead,
        DummyWrite,

        // Lectura de operandos
        FetchOperand,
        ReadOperand,
        WriteOperand,
        ReadOperandAndExecute,
        ReadOperandVariable,

        // Direccionamiento
        FetchZeroPageAddress,
        FetchAddressLow,
        FetchAddressHigh,
        FetchAddressHighExecute,
        FetchAddressHighAddX,
        FetchAddressHighAddY,

        FetchIndirectLow,
        FetchIndirectHigh,
        FetchIndirectHighAddY,
        FetchIndirectHighExecute,

        AddXToZeroPageAddress,
        AddYToZeroPageAddress,

        // Branches
        FetchBranch,
        EvaluateBranch,
        DummyBranchPageCrossRead,

        // Pila
        PushStatus,
        PushPCHigh,
        PushPCLow,
        PushAccumulator,
        PushStatusBRK,
        PushStatusInterrupt,

        PullStatus,
        PullPCLow,
        PullPCHigh,
        PullAccumulator,
        PullStatusBRK,
        FetchAddressHighToPC,
        StackPop,

        // Vectores de interrupción
        FetchInterruptVectorLow,
        FetchInterruptVectorHigh,

        // Instrucciones especiales
        CompleteRTI,
        CompleteBRK
    };

    struct MicroOperationMapper 
    {
        // Arreglo indexado por el valor numérico del enum
        static constexpr std::array<std::string_view, 43> Strings = {
            // Control general
            "FetchOpcode",
            "FetchOpcodeAndDiscard",
            "FetchOpcodeDiscardAndExec",

            "DummyRead",
            "DummyWrite",

            // Lectura de operandos
            "FetchOperand",
            "ReadOperand",
            "WriteOperand",
            "ReadOperandAndExecute",
            "ReadOperandVariable",

            // Direccionamiento
            "FetchZeroPageAddress",
            "FetchAddressLow",
            "FetchAddressHigh",
            "FetchAddressHighExecute",
            "FetchAddressHighAddX",
            "FetchAddressHighAddY",

            "FetchIndirectLow",
            "FetchIndirectHigh",
            "FetchIndirectHighAddY",
            "FetchIndirectHighExecute",

            "AddXToZeroPageAddress",
            "AddYToZeroPageAddress",

            // Branches
            "FetchBranch",
            "EvaluateBranch",
            "DummyBranchPageCrossRead",

            // Pila
            "PushStatus",
            "PushPCHigh",
            "PushPCLow",
            "PushAccumulator",
            "PushStatusBRK",
            "PushStatusInterrupt",

            "PullStatus",
            "PullPCLow",
            "PullPCHigh",
            "PullAccumulator",
            "PullStatusBRK",
            "FetchAddressHighToPC",
            "StackPop",

            // Vectores de interrupción
            "FetchInterruptVectorLow",
            "FetchInterruptVectorHigh",

            // Instrucciones especiales
            "CompleteRTI",
            "CompleteBRK"
        };

        // Función estática para obtener la cadena original de forma segura
        static constexpr std::string_view ToString(MicroOperation mode) 
        {
            auto index = static_cast<size_t>(mode);
            if (index < Strings.size()) {
                return Strings[index];
            }
            return "Unknown";
        }
    };
}