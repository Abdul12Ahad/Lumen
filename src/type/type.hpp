#pragma once

namespace lumen
{
    enum class TypeKind
    {
        Unknown,
        Int,
        Float,
        Double,
        Bool
    };

    inline const char* typeKindToString(TypeKind type)
    {
        switch (type)
        {
            case TypeKind::Unknown:
                return "unknown";

            case TypeKind::Int:
                return "int";

            case TypeKind::Float:
                return "float";

            case TypeKind::Double:
                return "double";

            case TypeKind::Bool:
                return "bool";
        }

        return "unknown";
    }

    inline bool isNumeric(TypeKind type)
    {
        return type==TypeKind::Int || type == TypeKind::Float || type==TypeKind::Double;
    }

    inline int typeRank(TypeKind type)
    {
        switch (type)
        {
            case TypeKind::Int:
                return 1;

            case TypeKind::Float:
                return 2;

            case TypeKind::Double:
                return 3;

            default:
                return 0;
        }
    }

    inline TypeKind commonNumericType(TypeKind a, TypeKind b)
    {
        if (!isNumeric(a) || !isNumeric(b))
            return TypeKind::Unknown;

        return typeRank(a) >= typeRank(b) ? a : b;
    }
}