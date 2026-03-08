# LCD Customer Display

## Project Overview

This was our first group project for the embedded systems course. The focus was not on writing perfect code, but rather on learning Git and GitHub, collaborating in a team, and understanding version control workflows.

The project simulates a dynamic customer display on a 16x2 LCD using an AVR microcontroller. It features various text animations, handles special characters (Å, Ä, Ö, å, ä, ö), and demonstrates random message selection.

## Features

- Random Customer Messages: Randomly selects a customer and one of their messages for display
- Text Animations:
  - Scroll (for long messages)
  - Blink
  - Fade-in
  - Typing effect
  - Split-text presentation
  - Fun “DiscoMan” animation
- Custom Characters: Supports Å, Ä, Ö, å, ä, ö using CGRAM
- UART Debugging: Serial output for monitoring program behavior

## Tools & Simulation

- Platform: Wokwi AVR Simulator
- Microcontroller: ATmega328P (Arduino Uno)
- Compiler: AVR-GCC (or Wokwi built-in)
- Version Control: Git and GitHub (primary learning goal)

## Learning Outcomes

- Collaborating in a team environment using Git and GitHub
- Structuring a multi-file C project for embedded systems
- Implementing interactive text animations on a 16x2 LCD
- Handling special characters on a limited display
- Simulating hardware without physical components using Wokwi

## How to Run

1. Clone the repository
```
git clone https://github.com/szandraugustsson/LED-Customer-Display.git
cd LED-Customer-Display
```
2. Build the project

Using Makefile
```
make
```
This will generate the hex file (avrdemo.hex) in the project folder.

3. Run in Wokwi Simulator

Open the project in Wokwi: https://wokwi.com/projects/416241646559459329
Upload the generated hex file.
