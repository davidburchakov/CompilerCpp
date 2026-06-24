//
// Created by David Burchakov on 6/24/26.
//
module;

#include "CppBaseVisitor.h"
export module SemanticVisitor;


export namespace CppZero {
    class SemanticVisitor: public CppBaseVisitor {
    public:
        SemanticVisitor() = default;
    };
};
