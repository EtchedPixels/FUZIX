# Fuzix for a Gemine with paged mode memory

Work in progress only

## Supported Hardware

Gemini GM811
Paged RAM cards (3 or 4 pages required) - eg GM806, GM862
Z80PIO wired to a CF adapter
TODO - CTC or RTC (only used for time sync right now)

## Memory Map
0000-DFFF		banked memory
E000-FFFF		Common space (not true common, but copies so be careful with vsriables)

## Hardware Support
- Serial
- PIO CF adapter
- GM833 RAMDisc

## TODO 
- GM822 RTC (or similar) as timer only
- GM816 I/O (CTC and RTC)
- Keyboard
- Video
- Gemini version of floppy driver
- Implement RTC NMI timer interrupt
- Normal interrupt handling for timer on PIO bit
- Full RTC
- AVC video
- Gemini SASI/SCSI

## Installation

Not yet
