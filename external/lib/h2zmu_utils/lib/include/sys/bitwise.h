//
// Created by jriessner on 13.10.22.
//

#ifndef H2ZMU_2_BITWISE_H
#define H2ZMU_2_BITWISE_H


#define CHECK_BIT(var,pos) ((var >> pos) & 1)
#define SET_BIT(var,n) (var |= 1 << n)
#define UNSET_BIT(var,n) (var &= ~(1 << n))


#endif //H2ZMU_2_BITWISE_H
