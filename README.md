# agrotech
class project

Yair's project: *irrigation control system*,
We will build a few kinds of irrigation control systems:
1. A fixed amount of water is given in fixed time intervals.
2. irrigation is actuated by a soil moisture sensor. Whenever soil moisture goes below a pre-determined threshold, irrigation faucet is opened.
3. irrigation is actuated by the weight of the pot where the plant grows in. When the weight goes down, it means that water evaporated, and irrigation water is needed to replenish what was lost. (How to deal with the mass of the plant that keeps growing?)

involve: 
* building a digital scale based on a load cell.
* setting up an ESP32 microcontrollers to receive data from the sensors make the necessary real-time calculations, and control irrigation.\n",
* uploading real-time data to the cloud (e.g. ThingSpeak)
