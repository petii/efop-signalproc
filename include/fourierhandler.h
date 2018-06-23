#pragma once

#include <vector>

class FourierHandler {

public:
    template<typename T>
    void addInput(const std::vector<T>& input) = delete;
    
    void runTransform() = delete;
    
    template<typename T>
    std::vector<T> getResult() const = delete;
};