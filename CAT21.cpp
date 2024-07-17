
#include "CAT21.h"



/*
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

enum MSG_STATE { MANDATORY, OPT, NOTIMPLEMENTED };


struct _MsgDetCAT21
{
    const char* name;
    MSG_STATE state;
};

struct _MsgDetCAT21 MsgDetSitaware[] = {
    {"FRN01: I021/010 Data Source Identification", MSG_STATE::MANDATORY},
    {"FRN02: I021/040 Target Report Descriptor", MSG_STATE::MANDATORY},
    {"FRN03: I021/030 Time of Day",MSG_STATE::MANDATORY},
    {"FRN04: I021/130 Position in WGS84 Coords",MSG_STATE::MANDATORY},
    {"FRN05: I021/080 Target Address",MSG_STATE::MANDATORY},
    {"FRN06: I021/140 Geometric Altitude",MSG_STATE::OPT},
    {"FRN07: I021/090 Figure of Merit",MSG_STATE::MANDATORY},
    {"FRN08: I021/210 Link Technology",MSG_STATE::MANDATORY},
    {"FRN09: I021/230 Roll Angle",MSG_STATE::NOTIMPLEMENTED},
    {"FRN10: I021/145 Flight Level",MSG_STATE::OPT},
    {"FRN11: I021/150 Air Speed",MSG_STATE::NOTIMPLEMENTED},
    {"FRN12: I021/151 True Airspeed",MSG_STATE::NOTIMPLEMENTED},
    {"FRN13: I021/152 Magnetic Heading",MSG_STATE::NOTIMPLEMENTED},
    {"FRN14: I021/155 Baro Verical Rate",MSG_STATE::NOTIMPLEMENTED},
    {"FRN15: I021/157 Geom Vertical rate",MSG_STATE::NOTIMPLEMENTED},
    {"FRN16: I021/160 Ground Vector",MSG_STATE::OPT},
    {"FRN17: I021/165 Rate of Turn",MSG_STATE::NOTIMPLEMENTED},
    {"FRN18: I021/170 Target Identification",MSG_STATE::MANDATORY},
    {"FRN19: I021/095 Velocity Accruacy",MSG_STATE::NOTIMPLEMENTED},
    {"FRN20: I021/032 TOD Accuracy",MSG_STATE::NOTIMPLEMENTED},
    {"FRN21: I021/200 Target Status",MSG_STATE::MANDATORY},
    {"FRN22: I021/020 Emitter Catagory",MSG_STATE::MANDATORY},
    {"FRN23: I021/220 Met Report",MSG_STATE::NOTIMPLEMENTED},
    {"FRN24: I021/146 Intermediate State Selected Atitude",MSG_STATE::NOTIMPLEMENTED},
    {"FRN25: I021/148 Final State Selected Atitude",MSG_STATE::NOTIMPLEMENTED},
    {"FRN26: I021/110 Trajectory Intent",MSG_STATE::NOTIMPLEMENTED},
    {"FRN27: I021/070 Mode 3/A",MSG_STATE::OPT},
    {"FRN28: I021/131 Signal Amplitude",MSG_STATE::NOTIMPLEMENTED},
};


struct _MsgDetCAT21 MsgDetModern[] = {
    //NEED to REVIEW MANDATORY/OPTIONAL
    {"FRN01: I021/010 Data Source Identification", MSG_STATE::MANDATORY},
    {"FRN02: I021/040 Target Report Descriptor", MSG_STATE::MANDATORY},
    {"FRN03: I021/161 Track Number",MSG_STATE::MANDATORY},
    {"FRN04: I021/015 Service Identification",MSG_STATE::NOTIMPLEMENTED},
    {"FRN05: I021/071 Time of Applicability for Position",MSG_STATE::NOTIMPLEMENTED},
    {"FRN06: I021/130 Position in WGS84 Coords",MSG_STATE::MANDATORY},  //3 byte version
    {"FRN07: I021/131 Position in WGS84 Coords-High Res",MSG_STATE::OPT},  //3 byte version
    {"FRN08: I021/071 Time of Applicability for Velocity",MSG_STATE::NOTIMPLEMENTED},
    {"FRN09: I021/150 Air Speed",MSG_STATE::NOTIMPLEMENTED},
    {"FRN10: I021/151 True Airspeed",MSG_STATE::NOTIMPLEMENTED},
    {"FRN11: I021/080 Target Address",MSG_STATE::MANDATORY},
    {"FRN12: I021/073 Time of Msg Reception of Position",MSG_STATE::MANDATORY},
    {"FRN13: I021/074 Time of Msg Reception of Position - High Precision",MSG_STATE::NOTIMPLEMENTED},
    {"FRN14: I021/075 Time of Msg Reception of Velocity",MSG_STATE::MANDATORY},
    {"FRN15: I021/076 Time of Msg Reception of Velocity - High Precision",MSG_STATE::NOTIMPLEMENTED},
    {"FRN16: I021/140 Geometric Altitude",MSG_STATE::MANDATORY},
    {"FRN17: I021/090 Quality Indicators",MSG_STATE::NOTIMPLEMENTED},
    {"FRN18: I021/210 MOPS Version",MSG_STATE::NOTIMPLEMENTED},
    {"FRN19: I021/070 Mode 3/A",MSG_STATE::OPT},
    {"FRN20: I021/230 Roll Angle",MSG_STATE::NOTIMPLEMENTED},
    {"FRN21: I021/145 Flight Level",MSG_STATE::MANDATORY},
    {"FRN22: I021/152 Magnetic Heading",MSG_STATE::NOTIMPLEMENTED},
    {"FRN23: I021/200 Target Status",MSG_STATE::NOTIMPLEMENTED},
    {"FRN24: I021/155 Baro Verical Rate",MSG_STATE::MANDATORY},
    {"FRN25: I021/157 Geom Vertical rate",MSG_STATE::NOTIMPLEMENTED},
    {"FRN26: I021/160 Ground Vector",MSG_STATE::MANDATORY},
    {"FRN27: I021/165 Track Angle Rate",MSG_STATE::NOTIMPLEMENTED},
    {"FRN28: I021/077 Time of Report Transmission",MSG_STATE::NOTIMPLEMENTED},
    {"FRN29: I021/170 Target Identification",MSG_STATE::MANDATORY},
    {"FRN30: I021/020 Emitter Catagory",MSG_STATE::MANDATORY},
    {"FRN31: I021/220 Met Report",MSG_STATE::NOTIMPLEMENTED},
    {"FRN32: I021/146 Intermediate State Selected Atitude",MSG_STATE::NOTIMPLEMENTED},
    {"FRN33: I021/148 Final State Selected Atitude",MSG_STATE::NOTIMPLEMENTED},
    {"FRN34: I021/110 Trajectory Intent",MSG_STATE::NOTIMPLEMENTED},
    {"FRN35: I021/016 Service Management",MSG_STATE::NOTIMPLEMENTED},
    {"FRN36: I021/008 Aircraft Operational Status",MSG_STATE::NOTIMPLEMENTED},
    {"FRN37: I021/271 Surface Capabilities and Characteristics",MSG_STATE::NOTIMPLEMENTED},
    {"FRN38: I021/132 Message Amplitude",MSG_STATE::NOTIMPLEMENTED},
    {"FRN39: I021/250 Mode S MB Data",MSG_STATE::NOTIMPLEMENTED},
    {"FRN40: I021/260 ACAS Resolution Advisory Report",MSG_STATE::NOTIMPLEMENTED},
    {"FRN41: I021/400 Receiver ID",MSG_STATE::NOTIMPLEMENTED},
    {"FRN42: I021/295 Data Ages",MSG_STATE::NOTIMPLEMENTED}
};


*/