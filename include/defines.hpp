#pragma once

#define CAST static_cast

#define ENUM_VAL_T($ENUM_T) __underlying_type($ENUM_T)


#define GEN_BITOPS($ENUM_T) \
    constexpr $ENUM_T operator~ ($ENUM_T operand) {                                         \
        return CAST<$ENUM_T>(~CAST<ENUM_VAL_T($ENUM_T)>(operand));                          \
    }                                                                                       \
    constexpr $ENUM_T operator| ($ENUM_T first, $ENUM_T second) {                           \
        return CAST<$ENUM_T>(                                                               \
            CAST<ENUM_VAL_T($ENUM_T)>(first) |                                              \
            CAST<ENUM_VAL_T($ENUM_T)>(second)                                               \
        );                                                                                  \
    }                                                                                       \
    constexpr $ENUM_T operator& ($ENUM_T first, $ENUM_T second) {                           \
        return CAST<$ENUM_T>(                                                               \
            CAST<ENUM_VAL_T($ENUM_T)>(first) |                                              \
            CAST<ENUM_VAL_T($ENUM_T)>(second)                                               \
        );                                                                                  \
    }
