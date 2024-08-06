//---------------------------------------------------------------------------

#ifndef AircraftH
#define AircraftH
//---------------------------------------------------------------------------

#include <string>
#include <sysinfoapi.h>


struct aircraft_ADSB
{
    bool bTrackValid;
    bool bMagValid;
    bool bTrueValid;

    double now;  // The time this file was generated, in seconds since Jan 1 1970 00:00:00 GMT (the Unix epoch)
    std::string hex; //the 24-bit ICAO identifier of the aircraft, as 6 hex digits. The identifier may start with ‘~’, this means that the address is a non-ICAO address (e.g. from TIS-B).
    std::string type; //type of underlying messages / best source of current data for this position / aircraft: ( order of which data is preferentially used )
    std::string flight;  //callsign, the flight name or aircraft registration as 8 chars (2.2.8.2.6)
    int alt_baro; //the aircraft barometric altitude in feet as a number OR “ground” as a string
    int alt_geom; //geometric (GNSS / INS) altitude in feet referenced to the WGS84 ellipsoid
    double gs; //ground speed in knots
    double ias;
    double mach;
    double track; //true track over ground in degrees (0-359)
    double track_Rate;
    double mag_heading;
    double true_heading;
    double nav_heading;
    double roll;
    int baro_rate; //Rate of change of barometric altitude, feet/minute
    int geom_rate; //Rate of change of GPS altitude, feet/minute
	std::string squawk; //Mode A code (Squawk), encoded as 4 octal digits
    bool hasSquawk=false;
    std::string emergency; //ADS-B emergency/priority status, a superset of the 7×00 squawks (2.2.3.2.7.8.1.1) (none, general, lifeguard, minfuel, nordo, unlawful, downed, reserved)
    std::string category; // emitter category to identify particular aircraft or vehicle classes (values A0 – D7) (2.2.3.2.5.2)
    double lat; //the aircraft position in decimal degrees
    double lon;
    int nic; //avigation Integrity Category (2.2.3.2.7.2.6)
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

    int HEX;
    int NumUpdates = 0;

    SYSTEMTIME nowSystem;
    char nowAsString[25];
    int age = 0;

    std::string categoryString;
    bool markForDelete = false;

   
};





#endif