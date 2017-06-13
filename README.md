# README #

This repository contains code to provide processing over a continuous stream-in data at a particular sample rate (frames) allocating a fixed sized buffer of a definite number of frames which is being processed by another thread using FIFO pipe IPC method.

The code is intended to provide real-time processing using callback functionality provided by PortAudio audio processing library.

The "portaudio.h" is the header file and "main.c" is the running program (In development stage).

The "libportaudio.a" is the library that is taken from the source code of PortAudio that supports build.
