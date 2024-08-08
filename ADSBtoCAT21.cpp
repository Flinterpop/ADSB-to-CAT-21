
//#define _WIN32

#define BMG_VERSION 1.2

#pragma comment (lib, "Ws2_32.lib")

#include <iostream>
#include <winsock2.h>
#include <vector>
#include <algorithm> //has sort


#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <string>
#include <sysinfoapi.h>
#include <conio.h>






struct aircraft_ADSB
{
    //ADSB fields from JSON

    double now;  // The time this file was generated, in seconds since Jan 1 1970 00:00:00 GMT (the Unix epoch)
    std::string hex; //the 24-bit ICAO identifier of the aircraft, as 6 hex digits. The identifier may start with ‘~’, this means that the address is a non-ICAO address (e.g. from TIS-B).
    std::string type; //type of underlying messages / best source of current data for this position / aircraft: ( order of which data is preferentially used )
    std::string flight;  //callsign, the flight name or aircraft registration as 8 chars (2.2.8.2.6)
    int alt_baro; //the aircraft barometric altitude in feet as a number OR “ground” as a string
    bool bHasAltBaro = false;
    int alt_geom; //geometric (GNSS / INS) altitude in feet referenced to the WGS84 ellipsoid
    bool bHasAltGeom = false;
    double gs; //ground speed in knots
    bool bHasGndSpd = false;
    int ias;
    bool bHasIAS = false;
    int tas;
    bool bHasTAS = false;
    double mach;
    bool bHasMach = false;
    double track; //true track over ground in degrees (0-359)
    bool bHasTrack = false;
    
    double track_Rate;
    double mag_heading;
    bool bHasMagHdg = false;
    
    double true_heading;
    bool bHasTrueHdg = false;
    
    double nav_heading;
    bool bHasNavHdg = false;
    double roll;
    int baro_rate; //Rate of change of barometric altitude, feet/minute
    int geom_rate; //Rate of change of GPS altitude, feet/minute
    std::string squawk; //Mode A code (Squawk), encoded as 4 octal digits
    bool bHasSquawk = false;
    std::string emergency; //ADS-B emergency/priority status, a superset of the 7×00 squawks (2.2.3.2.7.8.1.1) (none, general, lifeguard, minfuel, nordo, unlawful, downed, reserved)
    std::string category; // emitter category to identify particular aircraft or vehicle classes (values A0 – D7) (2.2.3.2.5.2)
    double lat; //the aircraft position in decimal degrees
    double lon;
    int nic; //n+avigation Integrity Category (2.2.3.2.7.2.6)
    int rc; //Radius of Containment, meters; a measure of position integrity derived from NIC & supplementary bits. (2.2.3.2.7.2.6, Table 2-69)
    double seen_pos; // how long ago(in seconds before “now”) the position was last updated
    double r_dst;
    double r_dir;
    int nic_baro; //Navigation Integrity Category for Barometric Altitude (2.2.5.1.35)
    int nac_p; //Navigation Accuracy for Position (2.2.5.1.35)
    int nac_v; //Navigation Accuracy for Velocity (2.2.5.1.19)
    int sil; //Source Integity Level (2.2.5.1.40)
    std::string sil_type; //interpretation of SIL: unknown, perhour, persample
    int gva; //Geometric Vertical Accuracy (2.2.3.2.7.2.8)
    int sda; //System Design Assurance (2.2.3.2.7.2.4.6)
    int alert; //Flight status alert bit (2.2.3.2.3.2)
    int spi; //Flight status special position identification bit (2.2.3.2.3.2)
    int messages;   //The total number of Mode S messages processed (arbitrary)
    double seen;  //how long ago (in seconds before “now”) a message was last received from this aircraft
    double rssi; //recent average RSSI (signal power), in dbFS; this will always be negative.

    //derived internal fields
    std::string categoryString;


    // track metadata
    int NumUpdates = 0;
    int age = 0;

    SYSTEMTIME nowSystem;
    char nowAsString[25];

    //CAT 21 fields
    int CAT21TrackNumber;
    char ACIdent[8];
    int ICAO;
    int Mode3;
    int EmitterCatagory;
};

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

enum SORT_BY {AGE, HEX, FLIGHT, SQUAWK ,  IAS, HDG, UPDATES, LAST_ITEM_SENTINEL };
constexpr int numSortTypes = static_cast<int>(LAST_ITEM_SENTINEL);
SORT_BY sortBy = FLIGHT;


enum MSG_STATE { MANDATORY, OPT, NOTIMPLEMENTED };

struct _MsgDetCAT21
{
    const char* name;
    MSG_STATE state;
};

//This is compliant to ASTERIX CAT21 V0.23 (draft version from 2003) or V0.26 (draft version from 2005).
// This is an obsolete version
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
    {"FRN19: I021/095 Velocity Accuracy",MSG_STATE::NOTIMPLEMENTED},
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

//This is compliant to ASX CAT 21 V2.6 21 Dec 2021. Checked 5 Aug 2024
/*
struct _MsgDetCAT21 MsgDetModern[] = {
    // MANDATORY means Mandatory in ASX 21 Standard, OPT means optional in the standard and has been implemented in this application, 
    // NOTIMPLEMENTED means optional in the ASX21 standard and not implemented in this application.
    {"FRN01: I021/010 Data Source Identification", MSG_STATE::MANDATORY},
    {"FRN02: I021/040 Target Report Descriptor", MSG_STATE::MANDATORY},
    {"FRN03: I021/161 Track Number",MSG_STATE::MANDATORY},
    {"FRN04: I021/015 Service Identification",MSG_STATE::NOTIMPLEMENTED}, 
    {"FRN05: I021/071 Time of Applicability for Position",MSG_STATE::NOTIMPLEMENTED},
    {"FRN06: I021/130 Position in WGS84 Coords",MSG_STATE::MANDATORY},  //3 byte version
    {"FRN07: I021/131 Position in WGS84 Coords-High Res",MSG_STATE::OPT},  //3 byte version
    {"FRN08: I021/072 Time of Applicability for Velocity",MSG_STATE::NOTIMPLEMENTED},
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
    {"FRN21: I021/145 Flight Level",MSG_STATE::OPT},
    {"FRN22: I021/152 Magnetic Heading",MSG_STATE::OPT},
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
    {"FRN42: I021/295 Data Ages",MSG_STATE::NOTIMPLEMENTED},
    {"FRN43: not used",MSG_STATE::NOTIMPLEMENTED },
    {"FRN44: not used",MSG_STATE::NOTIMPLEMENTED },
    {"FRN45: not used",MSG_STATE::NOTIMPLEMENTED },
    {"FRN46: not used",MSG_STATE::NOTIMPLEMENTED },
    {"FRN47: not used",MSG_STATE::NOTIMPLEMENTED },
    {"FRN48: RE - Reserved Expansion Field",MSG_STATE::NOTIMPLEMENTED },
    {"FRN49: SP - Special Purpose Field",MSG_STATE::NOTIMPLEMENTED }
};
*/

#define BUFLEN 1510  
byte _MSG[1500];
int _MSGLength;

static sockaddr_in m_server;
static int m_client_socket;
static SOCKET TCPClientSocket;

char CAT21_IP[20] = "239.255.1.1";
int CAT21_Port = 5055;
char ADSB_IP[20] = "192.168.1.133";
int ADSB_Port = 30154;

int RxPacketCount = 0;
int TxPacketCount = 0;
int Cat21MsgCount = 0;


//default ASTERIX details - overidden by CAT21.ini if present
int sac = 206;
int sic = 102;

int TrackNumberStart = 200;

std::vector<aircraft_ADSB> ACList;

HANDLE ptrTimerHandleSendAircraft;
int refreshPeriod = 2;
int refreshCounter = 4;
int dropAfterAge = 60;

bool bMODERN = true;
bool bPeriodicList = false;
bool debug = false;

int index = 0;
//int NumFRN;








//forward declarations
bool initialise_winsock();
void closeandclean_winsock();
SOCKET OpenTCPServerSocket(char* host, u_short port);
bool OpenSocket(char* ip, u_short port);
bool SendUDP(char* buffer, int len);

void UnixTimeToFileTime(time_t t, LPFILETIME pft);

int StartADSBThread(const char* host, unsigned int port);
int ExpandCategory(std::string cat, std::string& catStr);
aircraft_ADSB *parseJSON(const char* jsonString);
void UpdateAllAircraftPositions(aircraft_ADSB* ExistingAC, aircraft_ADSB ac);
aircraft_ADSB* FindAircraftByHex(std::string hex);

bool compareAC(aircraft_ADSB left, aircraft_ADSB  right);
void clear();

int BuildCAT21FromADSB(aircraft_ADSB* ADSB_AC);
int CreateCAT21Modern(aircraft_ADSB* ADSB_AC); //this is compliant to V2.X
int CreateCAT21SITAWARE(aircraft_ADSB* ADSB_AC);
void ShowADSBList();




void __stdcall TimerCallbackUpdateAircraft(PVOID, BOOLEAN)//(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
    for (int x = 0; x < ACList.size(); x++)
    {
        //ACList[x].age++;
        if (++ACList[x].age > dropAfterAge) ACList.erase(ACList.begin() + x);
    }

    if (bPeriodicList)
    {
        //refreshCounter++;
        if ((++refreshCounter % refreshPeriod) == 0)
        {
            clear();
            ShowADSBList();
        }
    }
}




//DISPLAY FUNCTIONS

void clear() {
    COORD topLeft = { 0, 0 };
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screen;
    DWORD written;

    GetConsoleScreenBufferInfo(console, &screen);
    FillConsoleOutputCharacterA(
        console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
    );
    FillConsoleOutputAttribute(
        console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
        screen.dwSize.X * screen.dwSize.Y, topLeft, &written
    );
    SetConsoleCursorPosition(console, topLeft);
}

void print_SortBy()
{
    switch (sortBy)
    {
    case AGE:
        puts("Sort by AGE"); break;
    case UPDATES:
        puts("Sort by Number of Updates"); break;
    case HEX:
        puts("Sort by ICAO"); break;
    case SQUAWK:
        puts("Sort by SQUAWK Code"); break;
    case FLIGHT:
        puts("Sort by Flight"); break;
    case IAS:
        puts("Sort by IAS"); break;
    case HDG:
        puts("Sort by True Heading"); break;
    default:
        puts("Sort by None"); break;
    }
}


void PrintMenu()
{
    //printf("ADS-B Dump1090 to CAT21 UDP. V%f Compiled %s\r\n", BMG_VERSION, __DATE__);
    //printf("Visual Studio version:%s\r\n", _MSC_VER);
    if (bMODERN) printf("Generating Modern CAT21\r\n");
    else printf("Generating SITAWARE CAT21\r\n");
    printf("Refresh Period: %d\r\n", refreshPeriod);
    printf("Age Out time: %d\r\n", dropAfterAge);
    printf("Receiving ADS-B DUMP 1090 via Telnet on %s:%d\r\n", ADSB_IP, ADSB_Port);
    printf("Transmitting CAT21 via UDP on %s:%d\r\n", CAT21_IP, CAT21_Port);
    print_SortBy();

    printf("Commands:\t\n----------\r\nx - eXit\r\ns - Sort By\r\nt - CAT21 Type: Modern or SITAWARE\r\nl - List\r\np - Periodic List\r\nd - debug\r\n");
    printf("This app is always running even if display is not refreshing\r\n");
}



void ShowADSBList()
{
    printf("Rx %d packets. Tx %d packets\r\n", RxPacketCount, TxPacketCount);
    print_SortBy();


    sort(ACList.begin(), ACList.end(), compareAC);
    char ias_s[50];
    char HdgT_s[50];
    char flight_s[50];
    char Cat_s[50];
    if (bMODERN) printf("Generating Modern CAT21\r\n");
    else printf("Generating SITAWARE CAT21\r\n");

    printf("Flight   ICAO     Cat      Squawk Lat [d.d] Lon [d.d]  Alt    IAS  GndSpd   HdgT #Updates    Age    TN   r_dst   r_dir\r\n");
    printf("-----------------------------------------------------------------------------------------------------------------------\r\n");
    for (aircraft_ADSB ac : ACList)
    {
        if ((ac.ias < 0) || (ac.ias > 1000))
            sprintf_s(ias_s, " ---");
        else sprintf_s(ias_s, "%4d", ac.ias);

        if ((ac.true_heading < 0) || (ac.true_heading > 360))
            sprintf_s(HdgT_s, "-----");
        else sprintf_s(HdgT_s, "%5.1f", ac.true_heading);

        if (ac.flight.size() > 0)
            sprintf_s(flight_s, "%s", ac.flight.c_str());
        else
            sprintf_s(flight_s, "------- ");

        if (ac.categoryString.size() > 0)
            sprintf_s(Cat_s, "%7s", ac.categoryString.c_str());
        else
            sprintf_s(Cat_s, "-------");

        printf("%s %s %s    %4s   %03.4f   %09.4f  %5d %s   %4.1f  %s      %3d    %3d %5d   ",
            flight_s, ac.hex.c_str(),  Cat_s, ac.squawk.c_str(), ac.lat, ac.lon, ac.alt_baro, ias_s, ac.gs, HdgT_s, ac.NumUpdates, ac.age,ac.CAT21TrackNumber);
        printf("%6.2f  %6.2f\r\n", ac.r_dst, ac.r_dir);
    }
    printf("-----------------------------------------------------------------------------------------------------------------------\r\n");

}



void mainConsoleLoop()
{
    bool done = false;
    while (!done)
    {
        if (_kbhit() != 0)
        {
            int ch = _getch();
            switch (ch)
            {
            case 'd':
            case 'D':
                debug = !debug;
                printf("Debug is %d\r\n", debug);
                break;
            case 'l':
            case 'L':
                ShowADSBList();
                break;
            case 'p':
            case 'P':
                clear();
                ShowADSBList();
                bPeriodicList = !bPeriodicList;
                if (bPeriodicList) printf("Periodic List On\r\n");
                else printf("Periodic List Off\r\n");
                break;
            case 's':
            case 'S':
                ++(int&)sortBy;
                if (sortBy >= numSortTypes) sortBy = AGE;
                clear();
                ShowADSBList();
                break;
            case 't':
            case 'T':
                bMODERN = !bMODERN;
                if (bMODERN) printf("Switched to Modern CAT21\r\n");
                else printf("Switched to SITAWARE CAT21\r\n");
                break;
            case 'x':
            case 'X':
                done = true;
                break;

            default:
                PrintMenu();
            }
            printf("Press h for help\r\n");
        }
    }
}


BOOL FileExists(LPCTSTR szPath)
{
    DWORD dwAttrib = GetFileAttributes(szPath);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}


void ReadIniFile()
{
    printf("Attempting to read .ini file\r\n");
    LPCTSTR inipath = L".\\ADSBtoCAT21.ini";
    if (!FileExists(inipath))
    {
        std::cout << "no ini file ";
        std::wcout << inipath;
        std::cout << " found" << std::endl;
    }
    else
    {
        std::cout << "Found ini file ";
        std::wcout << inipath;
        std::cout << " found" << std::endl;
    }

    wchar_t w_ADSB_IP[20];
    GetPrivateProfileString(L"Form", L"ADSB_IP", L"192.168.1.133", w_ADSB_IP, 20, inipath);
    size_t charsConverted = 0;
    wcstombs_s(&charsConverted, ADSB_IP, 20, w_ADSB_IP, 20);

    wchar_t w_CAT21_IP[20];
    GetPrivateProfileString(L"Form", L"CAT21_IP", L"239.255.1.1", w_CAT21_IP, 20, inipath);
    wcstombs_s(&charsConverted, CAT21_IP, 20, w_CAT21_IP, 20);

    ADSB_Port = GetPrivateProfileInt(L"Form", L"ADSB_PORT", 30154, inipath);
    CAT21_Port = GetPrivateProfileInt(L"Form", L"CAT21_PORT", 5055, inipath);
    printf("INI: ADSB: Port: %d  IP: %s \r\n", ADSB_Port, ADSB_IP);
    printf("INI: CAT21: Port: %d  IP: %s \r\n", CAT21_Port, CAT21_IP);

    sic = GetPrivateProfileInt(L"Form", L"CAT21_SIC", 16, inipath);
    sac = GetPrivateProfileInt(L"Form", L"CAT21_SAC", 204, inipath);
    printf("INI: sic: %d\r\n", sic);
    printf("INI: sac: %d\r\n", sac);

    printf("INI: CAT21: Port: %d  IP: %s \r\n", CAT21_Port, CAT21_IP);
    refreshPeriod = GetPrivateProfileInt(L"Form", L"RefreshPeriod", 2, inipath);
    printf("INI: Refresh Period : %d\r\n", refreshPeriod);

    dropAfterAge = GetPrivateProfileInt(L"Form", L"DropAfterAge", 360, inipath);
    printf("INI: Age Out time: %d\r\n", dropAfterAge);

    int t = GetPrivateProfileInt(L"Form", L"CAT21_is_SITAWARE", 0, inipath);
    if (t == 1) bMODERN = false;
    else bMODERN = true;

    if (bMODERN) printf("Generating Modern CAT21\r\n");
    else printf("Generating SITAWARE CAT21\r\n");

    int sb = GetPrivateProfileInt(L"Form", L"SORT_BY", 2, inipath);
    sortBy = static_cast<SORT_BY>(sb);

    printf("INI: Refresh Period : %d\r\n", refreshPeriod);

    printf("Done reading .ini file\r\n----------------------\r\n");


}
int main()
{
    ReadIniFile();




    const int UPDATE_INTERVAL = 1000; //this is ms //update Aircraft once per second
    CreateTimerQueueTimer(&ptrTimerHandleSendAircraft, NULL, TimerCallbackUpdateAircraft, NULL, 500, UPDATE_INTERVAL, WT_EXECUTEDEFAULT);

    initialise_winsock();
    
    StartADSBThread(ADSB_IP, ADSB_Port);
    OpenSocket(CAT21_IP, CAT21_Port);

    Sleep(1000);
    PrintMenu();
    ShowADSBList();

    mainConsoleLoop();
  
    closeandclean_winsock();
}


DWORD WINAPI TCPListenThread(LPVOID lpParam)
{
    char buffer[1510];
    SOCKET* TCPClientSocket = (SOCKET*)lpParam;

    while (true) {
        int n = ::recv(*TCPClientSocket, buffer, 1510, 0);

        if (SOCKET_ERROR == n) continue;

        if ((n > 0) && (n < 1510))
        {
            buffer[n] = 0;
            if (debug)
            {
                printf("%s\r\n", buffer); //doesn't flush without a newline
                fflush(stdout);// Force writing buffer to the stdout
            }

            aircraft_ADSB * a = parseJSON(buffer);
            RxPacketCount++;
            if (a!=NULL)
            {
                _MSGLength = BuildCAT21FromADSB(a);
                SendUDP((char *) & _MSG[0], _MSGLength);
                TxPacketCount++;
            }
        }
    }
    return 0;
}



bool compareAC(aircraft_ADSB left, aircraft_ADSB  right)
{
    switch (sortBy)
    {
    case AGE:
        return (left.age > right.age);
    case UPDATES:
        return (left.NumUpdates > right.NumUpdates );
    case HEX:
        return (left.hex < right.hex);
    case SQUAWK:
        return (left.squawk < right.squawk);
    case FLIGHT:
        return (left.flight < right.flight);
    case IAS:
        return (left.ias < right.ias);
    case HDG:
        return (left.true_heading < right.true_heading);

    default:
        return true;
    }
}

int StartADSBThread(const char* host, unsigned int port)
{
    //initialise_winsock();
    TCPClientSocket = OpenTCPServerSocket((char*)host, port);
    if (EXIT_FAILURE == TCPClientSocket) return EXIT_FAILURE;

    HANDLE hThread = CreateThread(NULL, 0, TCPListenThread, (LPVOID)&TCPClientSocket, 0, NULL);
    if (hThread == NULL)
    {
        printf("Thread not created\r\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


SOCKET OpenTCPServerSocket(char* host, u_short port)
{
    // Create client socket 
    SOCKET TCPClientSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (TCPClientSocket == INVALID_SOCKET) {
        printf("Error, I cannot create TCP socket\r\n");
        return EXIT_FAILURE;
    }

    // setup address structure
    SOCKADDR_IN  addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_port = htons(port);

    // Attemp to connect to server and exit on failure. 
    int connval = connect(TCPClientSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if (connval == SOCKET_ERROR) {
        printf("Error: cannot connect to server.\r\n");
        //Returns status code other than zero
        return EXIT_FAILURE;
    }
    return TCPClientSocket;
}

bool initialise_winsock()
{
    WSADATA ws;
    printf("Initialising Winsock...\r\n");
    if (WSAStartup(MAKEWORD(2, 2), &ws) != 0)
    {
        printf("Failed. Error Code: %d\r\n", WSAGetLastError());
        return true;
    }
    printf("         Winsock Initialized...\r\n");
    return false;
}


void closeandclean_winsock()
{
    WSACleanup();
}

//These are not used by Radar Sites
bool OpenSocket(char* ip, u_short port)
{
    // create socket
    if ((m_client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR) // <<< UDP socket
    {
        printf("socket() failed with error code: %d", WSAGetLastError());
        return true;
    }

    // setup address structure
    memset((char*)&m_server, 0, sizeof(m_server));
    m_server.sin_family = AF_INET;
    m_server.sin_port = htons(port);

    //InetPton(AF_INET, _T("192.168.1.1"), &m_server.sin_addr.s_addr);

    m_server.sin_addr.S_un.S_addr = inet_addr(ip);
    return false;
}

bool SendUDP(char* buffer, int len)
{
    if (sendto(m_client_socket, (const char*)buffer, len, 0, (sockaddr*)&m_server, sizeof(sockaddr_in)) == SOCKET_ERROR)
    {
        printf("sendto() failed with error code: %d\r\n", WSAGetLastError());
        return true;
    }
    //printf("Sent message of length %d\r\n", len);
    return false;
}


void UnixTimeToFileTime(time_t t, LPFILETIME pft)
{
    // Note that LONGLONG is a 64-bit value
    LONGLONG ll;

    ll = Int32x32To64(t, 10000000) + 116444736000000000;
    pft->dwLowDateTime = (DWORD)ll;
    pft->dwHighDateTime = ll >> 32;
}




aircraft_ADSB* parseJSON(const char* jsonString)
{
    aircraft_ADSB ac;
    ac.age = 0;
    ac.nowAsString[0] = 0;
    
    rapidjson::Document d;
    d.Parse(jsonString);
    if (!d.IsObject()) {
        //puts("Not Object");
        return 0;
    }
    if (!d.HasMember("now")) {
        //puts("No now field"); 
        return 0;
    }

    rapidjson::Value& s = d["now"];    ac.now = s.GetDouble();
    FILETIME ft;
    UnixTimeToFileTime((time_t)ac.now, &ft);
    FileTimeToSystemTime(&ft, &ac.nowSystem);
    sprintf_s(ac.nowAsString, "%d-%02d-%02d %02d:%02d:%02d", ac.nowSystem.wYear, ac.nowSystem.wMonth, ac.nowSystem.wDay, ac.nowSystem.wHour, ac.nowSystem.wMinute, ac.nowSystem.wSecond);

    if (d.HasMember("hex")) 
        { 
        s = d["hex"];    
        ac.hex = s.GetString(); 
        ac.ICAO = std::stoul(ac.hex, nullptr, 16);
        }
    if (d.HasMember("type")) { s = d["type"];    ac.type = s.GetString(); } //this is message type
    if (d.HasMember("flight")) 
        { 
        s = d["flight"];    
        ac.flight = s.GetString(); 
        for (int i = 0; i < ac.flight.size(); i++) {
            char c = ac.flight[i];
            ac.ACIdent[i] = c;
        }
        ac.ACIdent[7] = 0;
        }

    if (d.HasMember("alt_baro")) { s = d["alt_baro"];    ac.alt_baro = s.GetInt(); ac.bHasAltBaro = true; }
    if (d.HasMember("alt_geom")) { s = d["alt_geom"];    ac.alt_geom = s.GetInt(); ac.bHasAltGeom = true;  }
    if (d.HasMember("gs")) { s = d["gs"];    ac.gs = s.GetDouble(); ac.bHasGndSpd = true; }

    if (d.HasMember("ias")) { s = d["ias"];    ac.ias = s.GetInt(); ac.bHasIAS = true;    }
    if (d.HasMember("tas")) { s = d["tas"];    ac.tas = s.GetInt(); ac.bHasTAS = true;    }

    if (d.HasMember("mach")) { s = d["mach"];    ac.mach = s.GetDouble(); ac.bHasMach = true; }

    if (d.HasMember("track")) { s = d["track"];    ac.track = s.GetDouble(); ac.bHasTrack = true; }
    if (d.HasMember("mag_heading")) { s = d["mag_heading"];    ac.mag_heading = s.GetDouble(); ac.bHasMagHdg = true; }
    if (d.HasMember("true_heading")) { s = d["true_heading"];    ac.true_heading = s.GetDouble(); ac.bHasTrueHdg = true; }


    if (d.HasMember("baro_rate")) { s = d["baro_rate"];    ac.baro_rate = s.GetInt(); }
    if (d.HasMember("squawk")) { 
        s = d["squawk"];    
        ac.squawk = s.GetString(); 
        ac.bHasSquawk = true;
        //Mode3 is int
        //squawk: Mode A code (Squawk), encoded as 4 octal digits
            if (ac.squawk.size() < 4) {
                ac.Mode3 = 0;
            }
            else
            {
                try
                {
                    ac.Mode3 = std::stoul(ac.squawk, nullptr, 8);
                }
                catch (...)
                {
                    puts("Exception in SQUAWK");
                    ac.Mode3 = 0;
                }
            }
        }

    if (d.HasMember("emergency")) { s = d["emergency"];    ac.emergency = s.GetString(); }
    if (d.HasMember("category")) {
        s = d["category"];
        ac.category = s.GetString();
        ac.EmitterCatagory = ExpandCategory(ac.category, ac.categoryString);
    }
    if (d.HasMember("lat")) { s = d["lat"];    ac.lat = s.GetDouble(); }
    if (d.HasMember("lon")) { s = d["lon"];    ac.lon = s.GetDouble(); }
    if (d.HasMember("nic")) { s = d["nic"];    ac.nic = s.GetInt(); }
    if (d.HasMember("rc")) { s = d["rc"];    ac.rc = s.GetInt(); }

    if (d.HasMember("seen_pos")) { s = d["seen_pos"];    ac.seen_pos = s.GetDouble(); }

    if (d.HasMember("r_dst")) { s = d["r_dst"];    ac.r_dst = s.GetDouble(); }
    if (d.HasMember("r_dir")) { s = d["r_dir"];    ac.r_dir = s.GetDouble(); }
    if (d.HasMember("nic_baro")) { s = d["nic_baro"]; ac.nic_baro = s.GetInt(); }
    if (d.HasMember("nac_p")) { s = d["nac_p"];    ac.nac_p = s.GetInt(); }
    if (d.HasMember("nac_v")) { s = d["nac_v"];    ac.nac_v = s.GetInt(); }
    if (d.HasMember("sil")) { s = d["sil"];    ac.sil = s.GetInt(); }
    if (d.HasMember("sil_type")) { s = d["sil_type"];    ac.sil_type = s.GetString(); }
    if (d.HasMember("gva")) { s = d["gva"];    ac.gva = s.GetInt(); }
    if (d.HasMember("sda")) { s = d["sda"];    ac.sda = s.GetInt(); }
    if (d.HasMember("alert")) { s = d["alert"];    ac.alert = s.GetInt(); }
    if (d.HasMember("spi")) { s = d["spi"];    ac.spi = s.GetInt(); }
    if (d.HasMember("messages")) { s = d["messages"];    ac.messages = s.GetInt(); }
    if (d.HasMember("seen")) { s = d["seen"];    ac.seen = s.GetDouble(); }
    if (d.HasMember("rssi")) { s = d["rssi"];    ac.rssi = s.GetDouble(); }

    for (auto& c : ac.hex) c = toupper(c);

    
    aircraft_ADSB* ExistingAC = FindAircraftByHex(ac.hex);
    if (ExistingAC == NULL)
    {
        ac.CAT21TrackNumber = TrackNumberStart++;
        ACList.push_back(ac);
    }
    else
        UpdateAllAircraftPositions(ExistingAC, ac);

    ///////////////
    aircraft_ADSB* ASX_AC = FindAircraftByHex(ac.hex);
    //printf("Returning: %s\r\n", ASX_AC->hex.c_str());
    return ASX_AC;    
}



int ExpandCategory(std::string cat, std::string& catStr)
{
    int EC = 0;

    if (cat[0] == 'A')
    {
        if (cat[1] == '0') catStr = "None";
        else if (cat[1] == '1') { catStr = "Light"; EC = 1;}
        else if (cat[1] == '2') {catStr = "Small"; EC = 2;
        }
        else if (cat[1] == '3') {catStr = "Large"; EC = 1;
        }
        else if (cat[1] == '4') {catStr = "Hi Vtx"; EC = 4;
        }
        else if (cat[1] == '5') {catStr = "Heavy"; EC = 5;
        }
        else if (cat[1] == '6') {catStr = "Hi Perf"; EC = 6;
        }
        else if (cat[1] == '7') {catStr = "Rotor"; EC = 10;
        }
    }
    else if (cat[0] == 'B')
    {
        if (cat[1] == '0') catStr = "None";
        else if (cat[1] == '1') {
            catStr = "Glider"; EC = 11;
        }
        else if (cat[1] == '2') {
            catStr = "LTA"; EC = 12;
        }
        else if (cat[1] == '3') {catStr = "Para/Sky"; EC = 16;}
        else if (cat[1] == '4') {catStr = "Ultra-Light"; EC = 15;}
        else if (cat[1] == '5') {catStr = "RSV"; EC = 0;}//reserved
        else if (cat[1] == '6') {catStr = "UAV"; EC = 13;}
        else if (cat[1] == '7') {catStr = "Space/Trans"; EC = 14;}
    }
    else if (cat[0] == 'C')
    {
        if (cat[1] == '0') catStr = "None";
        else if (cat[1] == '1') {catStr = "Surface/Emerg"; EC = 20;
        }
        else if (cat[1] == '2') {catStr = "Surface/Service"; EC = 21;
        }
        else if (cat[1] == '3') {catStr = "Point"; EC = 22;
        }
        else if (cat[1] == '4') {catStr = "Cluster Obs"; EC = 23;
        }
        else if (cat[1] == '5') {catStr = "Line Obs"; EC = 24;
        }
        else if (cat[1] == '6') {catStr = "Rsv"; EC = 0;//reserved
        }
        else if (cat[1] == '7') {catStr = "Rsv"; EC = 0;//reserved
        }
    }
    else catStr = "ZZZ";
    return EC;
}


void UpdateAllAircraftPositions(aircraft_ADSB* ExistingAC, aircraft_ADSB ac)
{
    ExistingAC->now = ac.now;
    ExistingAC->age = ac.age;
    ExistingAC->nowSystem = ac.nowSystem;
    strcpy_s(ExistingAC->nowAsString, 24, ac.nowAsString);

    ExistingAC->alt_baro = ac.alt_baro;
    ExistingAC->alt_geom = ac.alt_geom;
    ExistingAC->lat = ac.lat;
    ExistingAC->lon = ac.lon;
    ExistingAC->ias = ac.ias;
    ExistingAC->mag_heading = ac.mag_heading;
    ExistingAC->true_heading = ac.true_heading;
    ExistingAC->squawk = ac.squawk;
    ExistingAC->baro_rate = ac.baro_rate;
    ExistingAC->mach = ac.mach;

    ExistingAC->NumUpdates++;
    //printf("Updating AC %X / %X with update %d\r\n", ExistingAC->hex, ac.hex, ExistingAC->NumUpdates);
}

aircraft_ADSB* FindAircraftByHex(std::string hex)
{
    for (auto& element : ACList)
        if (element.hex == hex) return &element;
    return NULL;
}






void InsertSAC_SIC_DI010()
{
    _MSG[index++] = (byte)sac;
    _MSG[index++] = (byte)sic;
}

//THIS is HARD CODED
void InsertEmitterCategoryDI020(int EC)
{
    /*
    0 = No ADS - B Emitter Category Information
    1 = light aircraft <= 15500 lbs
    2 = 15500 lbs < small aircraft < 75000 lbs
    3 = 75000 lbs < medium a / c < 300000 lbs
    4 = High Vortex Large
    5 = 300000 lbs <= heavy aircraft
    6 = highly manoeuvrable (5g acceleration capability) and high speed (>400 knots cruise)
    7 to 9 = reserved
    10 = rotocraft
    11 = glider / sailplane
    12 = lighter-than-air
    13 = unmanned aerial vehicle
    14 = space / transatmospheric vehicle
    15 = ultralight / handglider / paraglider
    16 = parachutist / skydiver
    17 to 19 = reserved
    20 = surface emergency vehicle
    21 = surface service vehicle
    22 = fixed ground or tethered obstruction
    23 = cluster obstacle
    24 = line obstacle
    */
    _MSG[index++] = EC;
}



//V0.23/26 DI
void InsertTimeOfDayDI030()
{
    SYSTEMTIME st;
    GetSystemTime(&st);
    int elapsed = (int)st.wHour * 3600 + (int)st.wMinute * 60 + (int)st.wSecond;

    int TOD_16 = (int)(elapsed * 128);
    _MSG[index++] = (byte)(TOD_16 / (256 * 256));
    _MSG[index++] = (byte)(TOD_16 / (256));
    _MSG[index++] = (byte)(TOD_16 - (_MSG[index - 2] * 256 * 256) - (_MSG[index - 1] * 256));
}

//THIS is HARD CODED and matches messages recorded from Medium Range Radar (CA Army)
void InsertTargetReportDescriptorDI040()
{
    _MSG[index++] = 0x01;
    _MSG[index++] = 0x00;
}

void InsertMode3ADI070(int Mode3)//optional field for modern and old CAT 21
{
    _MSG[index++] = (byte)(Mode3 / 256);
    _MSG[index++] = (byte)(Mode3 - _MSG[index - 1] * 256);
}

void InsertTimeOfMsgReceptionDI073()
{
    SYSTEMTIME st;
    GetSystemTime(&st);
    int elapsed = (int)st.wHour * 3600 + (int)st.wMinute * 60 + (int)st.wSecond;

    int TOD_16 = (int)(elapsed * 128);
    _MSG[index++] = (byte)(TOD_16 / (256 * 256));
    _MSG[index++] = (byte)(TOD_16 / (256));
    _MSG[index++] = (byte)(TOD_16 - (_MSG[index - 2] * 256 * 256) - (_MSG[index - 1] * 256));
}

void InsertTimeOfMsgReceptionDI075()
{
    SYSTEMTIME st;
    GetSystemTime(&st);
    int elapsed = (int)st.wHour * 3600 + (int)st.wMinute * 60 + (int)st.wSecond;

    int TOD_16 = (int)(elapsed * 128);
    _MSG[index++] = (byte)(TOD_16 / (256 * 256));
    _MSG[index++] = (byte)(TOD_16 / (256));
    _MSG[index++] = (byte)(TOD_16 - (_MSG[index - 2] * 256 * 256) - (_MSG[index - 1] * 256));
}

void InsertAC_ICAO_Address_DI80(int ICAO)
{
    //Tgt Address  24 bit ICAO
    _MSG[index++] = (byte)((ICAO & 0xFF0000) >> 16);
    _MSG[index++] = (byte)((ICAO & 0x00FF00) >> 8);
    _MSG[index++] = (byte)((ICAO & 0x0000FF));
}

//V0.23/26 DI
void InsertFOM_DI090()
//quality indicators
//Definition : ADS - B quality indicators transmitted by a / c according to MOPS version.
// Format : Variable Length Data Item, comprising a primary subfield of oneoctet, followed by one - octet extensions as necessary.

{
    _MSG[index++] = 0b10100000;
    _MSG[index++] = 0b00001000;

}

void InsertPositionDI130_3BytePos(double latitude, double longitude)
//Position in WGS-84 Co-ordinates
{
    int Lat = (int)(latitude / 0.0000214576721191406);  //this number is 180/2^23
    _MSG[index++] = (byte)((Lat & 0xFF0000) >> 16);
    _MSG[index++] = (byte)((Lat & 0x00FF00) >> 8);
    _MSG[index++] = (byte)((Lat & 0x0000FF));

    int Lon = (int)(longitude / 0.0000214576721191406);
    _MSG[index++] = (byte)((Lon & 0xFF0000) >> 16);
    _MSG[index++] = (byte)((Lon & 0x00FF00) >> 8);
    _MSG[index++] = (byte)((Lon & 0x0000FF));
}

void InsertPositionDI130_4BytePos(double latitude, double longitude)
//High-Resolution Position in WGS-84 Co-ordinates
{
    int Lat = (int)(latitude / 0.00000536441802978515625); //this number is 180/2^25
    _MSG[index++] = (byte)((Lat & 0xFF000000) >> 24);
    _MSG[index++] = (byte)((Lat & 0x00FF0000) >> 16);
    _MSG[index++] = (byte)((Lat & 0x0000FF00) >> 8);
    _MSG[index++] = (byte)((Lat & 0x000000FF));



    int Lon = (int)(longitude / 0.00000536441802978515625);
    //byte val = (byte)((Lon & 0xFF000000) >> 24);
    //if (longitude > 0) val |= 0x10000000;
    //_MSG[index++] = val;

    _MSG[index++] = (byte)((Lon & 0xFF000000) >> 24);
    _MSG[index++] = (byte)((Lon & 0x00FF0000) >> 16);
    _MSG[index++] = (byte)((Lon & 0x0000FF00) >> 8);
    _MSG[index++] = (byte)((Lon & 0x000000FF));
}

void InsertGeoHeightDI140(int alt)
//Minimum height from a plane tangent to the earth’s ellipsoid, defined by WGS - 84, in two’s complement form.
//lsb is 6.25 feet
{
    int gcount = (int)(alt / 6.25);
    byte msb = (byte)(gcount / 256);
    byte lsb = gcount % 256;
    _MSG[index++] = msb;
    _MSG[index++] = lsb;
}

void InsertFLDI145(double alt_baro)
//Flight Level from barometric measurements, not QNH corrected,in two’s complement form.
//lsb is 1/4 FL (25 feet)
{
    int gcount = alt_baro / 25;
    byte msb = (byte)(gcount / 256);
    byte lsb = gcount % 256;
    _MSG[index++] = msb;
    _MSG[index++] = lsb;
}


void InsertTASDI151(int tas)
//Definition : True Air Speed.
//Format : Two - Octet fixed length data item
//lsb is 1 knot
{
    byte msb = (byte)(tas / 256);
    byte lsb = tas % 256;
    _MSG[index++] = msb;
    _MSG[index++] = lsb;
}

void InsertMagHdgDI152(double mag_heading)
//Definition : Magnetic Heading(Element of Air Vector).
//Format : Two - Octet fixed length data item.
//lsb = 360° / 216 (approx. 0.0055°)
{//VERIFY THIS
    byte msb = (byte)(mag_heading / 256);
    byte lsb = (int)(mag_heading) % 256;
    _MSG[index++] = msb;
    _MSG[index++] = lsb;
}




//HARD CODED 
void InsertBarometricVerticalRateDI155()
{
    _MSG[index++] = 0x00;
    _MSG[index++] = 0x00;
}

void InsertAirborneGroundVectorDI160(int gndSpdInKnots, int heading)
{
    int AGVSpeed = (int)(gndSpdInKnots / 0.22);
    _MSG[index++] = (byte)((AGVSpeed & 0xFF00) >> 8);
    _MSG[index++] = (byte)((AGVSpeed & 0x00FF));

    int angle = (int)(heading / 0.0055);
    _MSG[index++] = (byte)((angle & 0xFF00) >> 8); // / 256);
    _MSG[index++] = (byte)((angle & 0x00FF)); //  % 256);
}

void InsertTrackNumberDI161(int CAT21TrackNumber)
{
    _MSG[index++] = (byte)((CAT21TrackNumber & 0xFF00) >> 8);  //Track Number MSB
    _MSG[index++] = (byte)(CAT21TrackNumber & 0x00FF);         //Track Number LSB
}

byte ICAOencode(char c)
{
    if ((c >= 'A') && (c <= 'Z'))
    {
        return (byte)((c - 'A') + 1);
    }
    else if (c == ' ')
    {
        return 0x20;
    }
    else //number 0 thru 9
    {
        return (byte)c;
    }
}

void Insert_ACIdent_DI170(char * ACIdent)
{
    byte c[8];

    for (int x = 0; x < 8; x++) c[x] = ICAOencode(ACIdent[x]);

    _MSG[index] = (byte)((c[0] & 0x3F) << 2);
    _MSG[index] = (byte)(_MSG[index] | (byte)((c[1] & 0x30) >> 4));
    index++;

    _MSG[index] = (byte)((c[1] & 0x0F) << 4);
    _MSG[index] = (byte)(_MSG[index] | (byte)((c[2] & 0x3C) >> 2));
    index++;

    _MSG[index] = (byte)((c[2] & 0x03) << 6);
    _MSG[index] = (byte)(_MSG[index] | c[3]);
    index++;

    _MSG[index] = (byte)((c[4] & 0x3F) << 2);
    _MSG[index] = (byte)(_MSG[index] | (c[5] & 0x30) >> 4);
    index++;

    _MSG[index] = (byte)((c[5] & 0x0F) << 4);
    _MSG[index] = (byte)(_MSG[index] | (c[6] & 0x3C) >> 2);
    index++;

    _MSG[index] = (byte)((c[6] & 0x03) << 6);
    _MSG[index] = (byte)(_MSG[index] | (c[7] & 0x3F));
    index++;

}

//HARD CODED
void InsertTargetStatusDI200()
{
    _MSG[index++] = 0b00000000;
}

//HARD CODED
void InsertLinkTechnologyDI210()
{
    _MSG[index++] = 0b00000000;
}

void UpdateMsgLength(int _Length)
{
    int msb = _Length / 256;
    int lsb = _Length - (msb * 256);
    _MSG[1] = (byte)msb;
    _MSG[2] = (byte)lsb;
}


////////////////


int BuildCAT21FromADSB(aircraft_ADSB* ADSB_AC)
{
    char buf[300];
    SYSTEMTIME st;
    int MsgLength = 0;

    if (bMODERN)
    {
        MsgLength = CreateCAT21Modern(ADSB_AC);
        if (debug) {
            GetSystemTime(&st);
            sprintf_s(buf, "Sending CAT21 V2+ #%02d at time %02d:%02d:%02d.%d\r\n", Cat21MsgCount, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
            puts(buf);
        }
    }
    else //SITAWARE
    {
        MsgLength = CreateCAT21SITAWARE(ADSB_AC);
        if (debug) {
            GetSystemTime(&st);
            sprintf_s(buf, "Sending CAT21 V0.26 #%02d at time %02d:%02d:%02d.%d\r\n", Cat21MsgCount, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
            puts(buf);
        }
    }

    Cat21MsgCount++;
    return MsgLength;
}


int CreateCAT21Modern(aircraft_ADSB *ADSB_AC) //this is compliant to V2.X
{
    std::vector<int> FRNsToSend;      //this is empty unlesss populated outside of this file by settings in the UI or config file
    FRNsToSend.clear();

    //this CAT21 is compliant to V0.26
    FRNsToSend.push_back(1);
    FRNsToSend.push_back(2);
    FRNsToSend.push_back(3);
    FRNsToSend.push_back(6);
    FRNsToSend.push_back(11);
    FRNsToSend.push_back(12);
    FRNsToSend.push_back(14);
    FRNsToSend.push_back(16);
    FRNsToSend.push_back(24);
    FRNsToSend.push_back(26);
    FRNsToSend.push_back(29);
    FRNsToSend.push_back(30);
      
    if (ADSB_AC->bHasSquawk) FRNsToSend.push_back(19); //this is FRN 19
    if (ADSB_AC->bHasAltBaro) FRNsToSend.push_back(21); //this is FRN 21 is FL
    if (ADSB_AC->bHasMagHdg) FRNsToSend.push_back(22); //this is FRN 22 is FL

    //build the FSPEC (next ~75 lines)
    int FSPEC[7] = { 0,0,0,0,0,0,0 };

    unsigned char m_FRN[50];
    //builds a bit map lookup table for each of the FRNs. ASX CAT 21 supports 49 FRNs: 1 thru 49
    for (int x = 1; x <= 49; x++) m_FRN[x] = 0b10000000 >> ((x - 1) % 7);

    for (int frn : FRNsToSend)
    {
        int _index = (frn-1) / 7;

        FSPEC[_index] |= m_FRN[frn];
        //printf("Adding FRN %d  index: %d, FRN Bits:%X\r\n", frn, _index, m_FRN[frn]);
    }


    //set the extension bits if required
    if (FSPEC[6] != 0)//there is at least 1 FRN in the 7th word (index is 6 because zero indexing)
    {
        FSPEC[5] |= FRNFX;
        FSPEC[4] |= FRNFX;
        FSPEC[3] |= FRNFX;
        FSPEC[2] |= FRNFX;
        FSPEC[1] |= FRNFX;
        FSPEC[0] |= FRNFX;
    }
    else if (FSPEC[5] != 0)
    {
        FSPEC[4] |= FRNFX;
        FSPEC[3] |= FRNFX;
        FSPEC[2] |= FRNFX;
        FSPEC[1] |= FRNFX;
        FSPEC[0] |= FRNFX;
    }
    else if (FSPEC[4] != 0)
    {
        FSPEC[3] |= FRNFX;
        FSPEC[2] |= FRNFX;
        FSPEC[1] |= FRNFX;
        FSPEC[0] |= FRNFX;
    }
    else if (FSPEC[3] != 0)
    {
        FSPEC[2] |= FRNFX;
        FSPEC[1] |= FRNFX;
        FSPEC[0] |= FRNFX;
    }
    else if (FSPEC[2] != 0)
    {
        FSPEC[1] |= FRNFX;
        FSPEC[0] |= FRNFX;
    }
    else if (FSPEC[1] != 0)
    {
        FSPEC[0] |= FRNFX;
    }

    //Start building the ASTERIX Message
    index = 0; //reset the index
    _MSG[index++] = 21;     //ASTERIX CAT 21
    _MSG[index++] = 0;      //LEN MSB    these are placeholders that will be filled in later
    _MSG[index++] = 0;      //LEN LSB
    
    //Insert the FSPEC into the Message
    _MSG[index++] = FSPEC[0];
    if (FSPEC[0] & FRNFX) _MSG[index++] = FSPEC[1];
    if (FSPEC[1] & FRNFX) _MSG[index++] = FSPEC[2];
    if (FSPEC[2] & FRNFX) _MSG[index++] = FSPEC[3];
    if (FSPEC[3] & FRNFX) _MSG[index++] = FSPEC[4];
    if (FSPEC[4] & FRNFX) _MSG[index++] = FSPEC[5];
    if (FSPEC[5] & FRNFX) _MSG[index++] = FSPEC[6];

    //FRN1
    InsertSAC_SIC_DI010();
    
    //FRN2
    InsertTargetReportDescriptorDI040();

    //FRN3
    InsertTrackNumberDI161(ADSB_AC->CAT21TrackNumber);
    
    //FRN6
    InsertPositionDI130_3BytePos(ADSB_AC->lat,ADSB_AC->lon);

    //FRN9: Air Speed (so is this IAS?)
    //FRN10: True Air Speed
    //FRN11
    InsertAC_ICAO_Address_DI80(ADSB_AC->ICAO);

    //FRN12
    InsertTimeOfMsgReceptionDI073();//073 Time of MSG Reception for Position
    
    //FRN13:
    
    //FRN14
    InsertTimeOfMsgReceptionDI075();//075 Time of MSG Reception for Velocity

    //FRN15:
    
    //FRN16
    InsertGeoHeightDI140(ADSB_AC->alt_geom);

    //FRN17:
    //FRN18:

    //FRN19
    if (std::find(FRNsToSend.begin(), FRNsToSend.end(), 19) != FRNsToSend.end()) 
        InsertMode3ADI070(ADSB_AC->Mode3);
    
    //FRN20: Roll Angle

    //FRN21
    if (std::find(FRNsToSend.begin(), FRNsToSend.end(), 21) != FRNsToSend.end())
        InsertFLDI145(ADSB_AC->alt_baro);

    //FRN 22
    if (std::find(FRNsToSend.begin(), FRNsToSend.end(), 22) != FRNsToSend.end())
        InsertMagHdgDI152(ADSB_AC->mag_heading);

    //FRN23:
    

    //FRN24
    InsertBarometricVerticalRateDI155();

    //FRN25:Geo Vertical Rate

    //FRN26
    InsertAirborneGroundVectorDI160(ADSB_AC->ias, ADSB_AC->true_heading); // ias and true heading may be wrong
    
    //FRN27
    //FRN28
     
    
    //FRN29
    Insert_ACIdent_DI170(ADSB_AC->ACIdent);

    //FRN30
    InsertEmitterCategoryDI020(ADSB_AC->EmitterCatagory);

    UpdateMsgLength(index);
    return index;
}


int CreateCAT21SITAWARE(aircraft_ADSB *ADSB_AC)
{
    std::vector<int> FRNsToSend;      //this is empty unlesss populated outside of this file by settings in the UI or config file
    FRNsToSend.clear();

    //this CAT21 is compliant to Sitaware
    FRNsToSend.push_back(1);
    FRNsToSend.push_back(2);
    FRNsToSend.push_back(3);
    FRNsToSend.push_back(4);
    FRNsToSend.push_back(5);
    FRNsToSend.push_back(7);
    FRNsToSend.push_back(8);
    FRNsToSend.push_back(18);
    FRNsToSend.push_back(21);
    FRNsToSend.push_back(22);


    //if (ADSB_AC->bHasSquawk) 
        FRNsToSend.push_back(6);
    //if (ADSB_AC->bHasSquawk) 
        FRNsToSend.push_back(10);
    //if (ADSB_AC->bHasAltBaro) 
        FRNsToSend.push_back(16);
    //if (ADSB_AC->bHasMagHdg) 
        FRNsToSend.push_back(22); 
    //if (ADSB_AC->bHasMagHdg) 
        FRNsToSend.push_back(27); 

    //build the FSPEC (next ~75 lines)
    int FSPEC[6] = { 0,0,0,0,0,0 };

    unsigned char m_FRN[50];
    //builds a bit map lookup table for each of the FRNs. ASX CAT 21 supports 49 FRNs: 1 thru 49
    for (int x = 1; x <= 49; x++) m_FRN[x] = 0b10000000 >> ((x - 1) % 7);

    for (int frn : FRNsToSend)
    {
        int _index = (frn - 1) / 7;
        FSPEC[_index] |= m_FRN[frn];
        //printf("Adding FRN %d  index: %d, FRN Bits:%X\r\n", frn, _index, m_FRN[frn]);
    }




    //add the extension bit if required
    if (FSPEC[5] != 0)
    {
        FSPEC[4] |= FRNFX;
        FSPEC[3] |= FRNFX;
        FSPEC[2] |= FRNFX;
        FSPEC[1] |= FRNFX;
        FSPEC[0] |= FRNFX;
    }
    else if (FSPEC[4] != 0)
    {
        FSPEC[3] |= FRNFX;
        FSPEC[2] |= FRNFX;
        FSPEC[1] |= FRNFX;
        FSPEC[0] |= FRNFX;
    }
    else if (FSPEC[3] != 0)
    {
        FSPEC[2] |= FRNFX;
        FSPEC[1] |= FRNFX;
        FSPEC[0] |= FRNFX;
    }
    else if (FSPEC[2] != 0)
    {
        FSPEC[1] |= FRNFX;
        FSPEC[0] |= FRNFX;
    }
    else if (FSPEC[1] != 0)
    {
        FSPEC[0] |= FRNFX;
    }


    //Start building the ASTERIX Message
    index = 0; //reset the index
    _MSG[index++] = 21;     //ASTERIX CAT 21
    _MSG[index++] = 0;      //LEN MSB    these are placeholders that will be filled in later
    _MSG[index++] = 0;      //LEN LSB


    _MSG[index++] = FSPEC[0];
    if (FSPEC[0] & FRNFX) _MSG[index++] = FSPEC[1];
    if (FSPEC[1] & FRNFX) _MSG[index++] = FSPEC[2];
    if (FSPEC[2] & FRNFX) _MSG[index++] = FSPEC[3];
    if (FSPEC[3] & FRNFX) _MSG[index++] = FSPEC[4]; //this will never happen with this implementation. FSPEC[4] only contains RE and SP

    InsertSAC_SIC_DI010(); //FRN 1 Mandatory
    InsertTargetReportDescriptorDI040(); //FRN 2 M
    InsertTimeOfDayDI030(); //FRN 3 M
    InsertPositionDI130_4BytePos(ADSB_AC->lat,ADSB_AC->lon);   //FRN 4 M
    InsertAC_ICAO_Address_DI80(ADSB_AC->ICAO); // FRN 5 M

    if (std::find(FRNsToSend.begin(), FRNsToSend.end(), 6) != FRNsToSend.end())
        InsertGeoHeightDI140(ADSB_AC->alt_geom); // FRN 6 Optional

    InsertFOM_DI090();   //FRN 7 M
    InsertLinkTechnologyDI210();    // FRN 8 M


    if (std::find(FRNsToSend.begin(), FRNsToSend.end(), 10) != FRNsToSend.end()) 
        InsertFLDI145(ADSB_AC->alt_baro);// FRN 10 Optional
    if (std::find(FRNsToSend.begin(), FRNsToSend.end(), 16) != FRNsToSend.end()) 
        InsertAirborneGroundVectorDI160(ADSB_AC->ias, ADSB_AC->true_heading);// FRN 16 Optional  ias and true heading may be wrong

    Insert_ACIdent_DI170(ADSB_AC->ACIdent);//FRN 18 M
    InsertTargetStatusDI200(); // FRN 21 M
    InsertEmitterCategoryDI020(ADSB_AC->EmitterCatagory); //FRN 22

    if (std::find(FRNsToSend.begin(), FRNsToSend.end(), 27) != FRNsToSend.end())
        InsertMode3ADI070(ADSB_AC->Mode3);

    UpdateMsgLength(index);
    return index;
}













