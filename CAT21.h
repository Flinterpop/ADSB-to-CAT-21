#ifndef CAT21H
#define CAT21H





bool BuildCAT21FromADSB(aircraft* ADSB_AC);



void initFRN();
void InitADSB();
void CleanUpADSB();


void SetupModernCAT21();
int CreateCAT21(unsigned char*m);
void CreateCAT21Modern(); //this is compliant to V2.X







#endif