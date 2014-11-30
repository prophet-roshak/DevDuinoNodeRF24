/*
 * sensornet.hpp
 *
 *  Created on: 20 нояб. 2014 г.
 *      Author: Prophet
 */

#ifndef SENSORNET_HPP_
#define SENSORNET_HPP_

/***********************************************************************
************* Set the Node Address *************************************
***********************************************************************/

// These are the Octal addresses that will be assigned
const uint16_t node_address_set[10] = { 00, 02, 05, 012, 015, 022, 025, 032, 035, 045 };

// 0 = Master
// 1-2 (02,05)   = Children of Master(00)
// 3,5 (012,022) = Children of (02)
// 4,6 (015,025) = Children of (05)
// 7   (032)     = Child of (02)
// 8,9 (035,045) = Children of (05)

uint8_t NODE_ADDRESS = 1; // Use numbers 0 through 9 to select an address from the array




#endif /* SENSORNET_HPP_ */
