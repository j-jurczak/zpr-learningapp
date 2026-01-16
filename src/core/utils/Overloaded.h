/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Helper template to handle variant types easily.
 */
#pragma once

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overloaded( Ts... ) -> overloaded<Ts...>;