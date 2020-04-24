#include <Functions/FunctionFactory.h>
#include <Functions/FunctionBinaryArithmetic.h>

namespace DB
{
namespace ErrorCodes
{
    extern const int LOGICAL_ERROR;
}

template <typename Result, typename A, typename B> 
inline Result applySpecial(A /*a*/, B /*b*/) {
    throw Exception("Bit shift is not implemented for big integers", ErrorCodes::LOGICAL_ERROR);
    // if constexpr (std::is_same_v<A, UInt8>)
    //     return static_cast<Result>(static_cast<UInt16>(a)) >> static_cast<Result>(b);
    // else
    //     return static_cast<Result>(a) >> static_cast<Result>(static_cast<UInt16>(b));
}

template <typename A, typename B>
struct BitShiftRightImpl
{
    using ResultType = typename NumberTraits::ResultOfBit<A, B>::Type;
    static const constexpr bool allow_fixed_string = false;
    static constexpr bool is_special = is_big_int_v<ResultType>; 

    template <typename Result = ResultType>
    static inline NO_SANITIZE_UNDEFINED Result apply(A a, B b)
    {
        if constexpr (is_special)
            return applySpecial<Result>(a, b);
        else
            return static_cast<Result>(a) >> static_cast<Result>(b);
    }

#if USE_EMBEDDED_COMPILER
    static constexpr bool compilable = true;

    static inline llvm::Value * compile(llvm::IRBuilder<> & b, llvm::Value * left, llvm::Value * right, bool is_signed)
    {
        if (!left->getType()->isIntegerTy())
            throw Exception("BitShiftRightImpl expected an integral type", ErrorCodes::LOGICAL_ERROR);
        return is_signed ? b.CreateAShr(left, right) : b.CreateLShr(left, right);
    }
#endif
};

struct NameBitShiftRight { static constexpr auto name = "bitShiftRight"; };
using FunctionBitShiftRight = FunctionBinaryArithmetic<BitShiftRightImpl, NameBitShiftRight>;

void registerFunctionBitShiftRight(FunctionFactory & factory)
{
    factory.registerFunction<FunctionBitShiftRight>();
}

}
