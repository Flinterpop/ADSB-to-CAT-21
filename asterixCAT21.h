#ifndef asterixCAT21H
#define asterixCAT21H

#include "aircraft.h"
typedef unsigned char byte;



void initFRN();
void InitADSB();
void CleanUpADSB();

bool BuildCAT21FromADSB(aircraft_ADSB *ADSB_AC);

void SetupModernCAT21();
int CreateCAT21(byte *m);
void CreateCAT21Modern(); //this is compliant to V2.X

#define FRN1 0b10000000
#define FRN2 0b01000000
#define FRN3 0b00100000
#define FRN4 0b00010000
#define FRN5 0b00001000
#define FRN6 0b00000100
#define FRN7 0b00000010

#define FRNFX 0b00000001

#define FRN8 0b10000000
#define FRN9 0b01000000
#define FRN10 0b00100000
#define FRN11 0b00010000
#define FRN12 0b00001000
#define FRN13 0b00000100
#define FRN14 0b00000010

#define FRN15 0b10000000
#define FRN16 0b01000000
#define FRN17 0b00100000
#define FRN18 0b00010000
#define FRN19 0b00001000
#define FRN20 0b00000100
#define FRN21 0b00000010

#define FRN22 0b10000000
#define FRN23 0b01000000
#define FRN24 0b00100000
#define FRN25 0b00010000
#define FRN26 0b00001000
#define FRN27 0b00000100
#define FRN28 0b00000010

#define FRN29 0b10000000
#define FRN30 0b01000000
#define FRN31 0b00100000
#define FRN32 0b00010000
#define FRN33 0b00001000
#define FRN34 0b00000100
#define FRN35 0b00000010

#define FRN36 0b10000000
#define FRN37 0b01000000
#define FRN38 0b00100000
#define FRN39 0b00010000
#define FRN40 0b00001000
#define FRN41 0b00000100
#define FRN42 0b00000010






enum MSG_STATE { MANDATORY, OPT, NOTIMPLEMENTED};


#endif