# dsPIC Autonomous Robot

This project is an autonomous mobile robot based on the **dsPIC30F4013** microcontroller.

The robot uses distance sensors to follow the environment, detect obstacles and adjust its movement automatically. Two DC motors are controlled independently using PWM, which allows the robot to move forward, turn and make small corrections while following a wall.

The hardware part of the project also included assembling the PCB and connecting the microcontroller, sensors and motor control electronics.

## How it works

The movement logic is implemented as a simple state machine.

Depending on the sensor readings, the robot can:

- remain stopped and wait for a start command
- move forward
- turn left
- turn right
- correct its direction while moving
- stop or change direction when an obstacle is detected

While moving forward, the robot continuously checks the distance in front of it and the distance from the wall on its side.

If an obstacle is detected in front of the robot, it changes state and turns right.

If the side sensors no longer detect the wall, the robot can turn left and continue following the path.

Small differences between the side sensor measurements are also used to correct the robot's direction while it is moving.

## Sensors

The project uses several distance measurements.

Two ultrasonic sensors are connected through trigger and echo signals. The echo pulse duration is measured using the dsPIC timers and then converted into a distance value.

An additional analog **Sharp distance sensor** is read through the ADC and is mainly used for detecting the distance in front of the robot.

The sensor readings are combined in the movement logic to decide what the robot should do next.

## Motor control

The robot has two independently controlled DC motors.

PWM signals are generated using the dsPIC **Output Compare** modules and Timer 2.

Separate functions are used for controlling the left and right motor:

```text
PWMlevi()
PWMdesni()
```

The direction of each motor is controlled through digital output pins.

This allows the robot to:

```text
move forward
turn left
turn right
correct its direction
stop
```

Different PWM values can also be used for the left and right motor when the robot needs to make a smaller course correction.

## UART control

UART communication is used for sending information and receiving control commands.

The project supports commands such as:

```text
START
STOP
```

UART was also used during development for monitoring the current state of the robot and sensor values.

The START/STOP communication was used together with Bluetooth control during the project.

## State machine

The main movement states used in the program are:

```text
0 - stopped
1 - moving forward
2 - turning left
3 - turning right
5 - course correction
```

Transitions between the states depend mainly on the distance sensor measurements.

For example:

```text
Sensors
   |
   v
Read distances
   |
   v
Check obstacle / wall position
   |
   v
Select movement state
   |
   v
Set motor direction and PWM
```

This process is continuously repeated while the robot is running.

## Timers and interrupts

Several dsPIC hardware peripherals are used in the project.

Timers are used for:

- delay generation
- ultrasonic echo measurement
- PWM generation
- timing different robot operations

The project also uses interrupt service routines for:

- Timer interrupts
- UART receive interrupts
- ADC conversion

## Project structure

The main application is implemented in:

```text
main.c
```

Other source files contain drivers and initialization code for the peripherals:

```text
adc.c
adc.h

uart_driver.c
uart_driver.h

Tajmeri.c
Tajmeri.h

tajmer2.c
tajmer2.h

outcompare.h
```

The MPLAB X project configuration is stored in:

```text
nbproject/
```

## Technologies used

- dsPIC30F4013
- Embedded C
- MPLAB X
- XC16 compiler
- PWM / Output Compare
- ADC
- UART
- Hardware timers
- Interrupts
- Ultrasonic distance sensors
- Sharp analog distance sensor
- DC motor control
- PCB assembly and hardware integration

## What I worked with

Through this project I worked with:

- programming the dsPIC30F4013 at register level
- configuring hardware timers and interrupts
- generating PWM signals for DC motor control
- reading analog sensor data using the ADC
- measuring ultrasonic sensor signals
- UART communication
- sensor-based autonomous movement
- state-machine based robot control
- course correction and obstacle avoidance
- PCB assembly and integration of sensors and motors

## Note

The repository contains the MPLAB X source project.

Build output and user-specific MPLAB files are not required for the source repository and can be generated again when the project is built.
