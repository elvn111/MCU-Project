#include "dsp.h"
#include <stdlib.h>


char info[1024] = { 0 };

//GPS_Module_Info GPS_Info;

info_type_def Info_Check(char *ch) {
	info_type_def info_type;
	if (ch[1] == 'G' && ch[2] == 'P' && ch[3] == 'N' && ch[4] == 'A' && ch[5] == 'V') {
		info_type = GPNAV;
	}
	if (ch[1] == 'G' && ch[2] == 'P' && ch[3] == 'Y' && ch[4] == 'B' && ch[5] == 'M') {
		info_type = GPYBM;
	}
	if (*ch != '$') {
		return INFO_ERROE;
	}
	else {
		ch++;
		char nb = *ch;
		ch++;
		int i = 0;
		while (*ch != '*'&&i<250) {
			nb = nb ^ *ch;
			ch++;
			i++;
		}
		char* end;
		if (nb == strtol(ch+1, &end, 16))
			return info_type;
	}
	return INFO_ERROE;
}

int GPS_Info_RSL(char* ptr, GPS_Module_Info* pInfo) {
	double temp;
	info_type_def info_type = Info_Check(ptr);
	double info_d[40] = { 0 };
	if (INFO_ERROE == info_type) {
		return 0;
	}
	else {
		for (int i = 0;*ptr && *ptr != '*' && i < 36;) {
			temp = (double)strtod(ptr + 1, &ptr);
			if (*ptr == ',') {
				info_d[i] = temp;
				i++;
			}
		}
	}
	if (info_type == GPNAV) {
		pInfo->latitude			= info_d[6];
		pInfo->longitude		= info_d[7];
		pInfo->ElpHeight		= info_d[8];
		pInfo->Heading_Angle	= info_d[11];
		pInfo->Baseline			= info_d[20];
		pInfo->GPS_Satellite	= (uint8_t)info_d[23];
	}
	if (info_type == GPYBM) {
		pInfo->latitude			= info_d[4];
		pInfo->longitude		= info_d[5];
		pInfo->ElpHeight		= info_d[6]; 
		pInfo->Heading_Angle	= info_d[7];
		pInfo->Baseline			= info_d[19];
		pInfo->GPS_Satellite	= (uint8_t)info_d[20];
	}
	return 1;
}
