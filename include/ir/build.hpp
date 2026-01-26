#pragma once
#ifndef BUILD_HPP
#define BUILD_HPP

#include "koopa.h"
#include "parser.hpp"


class Builder{
 public:
    std::unique_ptr<Parser> parser;

    Builder();
    Builder(std::string input);

    koopa_raw_program_t *build();
};

#endif // BUILD_HPP 
