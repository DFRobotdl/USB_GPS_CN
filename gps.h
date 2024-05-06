#ifndef _GPS_H_
#define _GPS_H_

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

void read_GPS_Data(char *Gps_Buffer);
void parse_GpsDATA();
void print_Save();
void RST_Buffer(char *Buffer);

int wgs2bd(double lat, double lon, double* pLat, double* pLon); // WGS84=>BD09 地球坐标系=>百度坐标系 
int gcj2bd(double lat, double lon, double* pLat, double* pLon);  // GCJ02=>BD09 火星坐标系=>百度坐标系 
int wgs2gcj(double lat, double lon, double* pLat, double* pLon);// WGS84=>GCJ02 地球坐标系=>火星坐标系
double transformLat(double x, double y);// 纬度偏移量
double transformLon(double x, double y);// 经度偏移量
int outOfChina(double lat, double lon);
double StringToIntToDu(char* LatorLon);



struct
{
  char GPS_DATA[80];
  char GetData_Flag; //获取到 GPS 数据标志位
  char ParseData_Flag; //解析完成标志位
  char UTCTime[12]; //UTC 时间
  double slatitude; //纬度
  char N_S[2]; //N/S
  double slongitude; //经度
  char E_W[2]; //E/W
  char Usefull_Flag; //定位信息是否有效标志位
} Save_Data;



void read_GPS_Data(char *Gps_Buffer)
{
  char* GPS_DATAHead;
  char* GPS_DATATail;
  if ((GPS_DATAHead = strstr(Gps_Buffer, "$GPRMC,")) != NULL || (GPS_DATAHead =strstr(Gps_Buffer, "$GPVTG")) != NULL ){
    if (((GPS_DATATail = strstr(GPS_DATAHead, "\n")) != NULL) && (GPS_DATATail >GPS_DATAHead)){
      memset(Save_Data.GPS_DATA, 0, strlen(Save_Data.GPS_DATA));
      memcpy(Save_Data.GPS_DATA, GPS_DATAHead, GPS_DATATail - GPS_DATAHead);
      Save_Data.GetData_Flag = 1;
    }
  }
}

void parse_GpsDATA()
{
  
  char *subString;
  char *subStringNext;
  char sBuffer[20];
  double lat = 0 ,lon = 0;
  if (Save_Data.GetData_Flag){
    Save_Data.GetData_Flag = 0;
    //printf("%s\n",Save_Data.GPS_DATA);
    for (int i = 0 ; i <= 6 ; i++){
      if (i == 0){
        if ((subString = strstr(Save_Data.GPS_DATA, ",")) == NULL){
          printf("ERROR\n");//解析错误
        }
      }else{
        subString++;
        if ((subStringNext = strstr(subString, ",")) != NULL){
          char buffer[12];
          char usefullBuffer[2];
          switch(i){
            case 1:RST_Buffer(Save_Data.UTCTime);
              memcpy(Save_Data.UTCTime, subString, subStringNext - subString);break; //获取 UTC 时间
            case 2:RST_Buffer(usefullBuffer);
              memcpy(usefullBuffer, subString, subStringNext - subString);break;//获取定位状态
            case 3:RST_Buffer(sBuffer);
              memcpy(sBuffer, subString, subStringNext - subString);
              if(Save_Data.Usefull_Flag)
                lat = StringToIntToDu(sBuffer);break; //获取纬度信息
            case 4:RST_Buffer(Save_Data.N_S);
              memcpy(Save_Data.N_S, subString, subStringNext - subString);break;//获取 N/S
            case 5:RST_Buffer(sBuffer);
              memcpy(sBuffer, subString, subStringNext - subString);
              if(Save_Data.Usefull_Flag)
                lon = StringToIntToDu(sBuffer);break; //获取纬度信息
            case 6:RST_Buffer(Save_Data.E_W);
              memcpy(Save_Data.E_W, subString, subStringNext - subString);break;//获取 E/W
            default:break;
          }
          subString = subStringNext;
          Save_Data.ParseData_Flag = 1;
          if(usefullBuffer[0] == 'A')
            Save_Data.Usefull_Flag = 1;
          else 
            if(usefullBuffer[0] == 'V')
              Save_Data.Usefull_Flag = 0;
        }else{
          printf("ERROR\n");//解析错误
        }
      }
    }
    if(Save_Data.Usefull_Flag){
      //printf("lat= %f, lon= %f", lat, lon);
      Save_Data.Usefull_Flag = wgs2bd(lat, lon, &Save_Data.slatitude, &Save_Data.slongitude);
    }
  }
}

void print_Save()
{
  printf("***************************************\n");
  printf("时间\t:[%s]\n",Save_Data.UTCTime);
  printf("纬度\t:[%.6f]\n",Save_Data.slatitude);
  printf("N/S\t:[%s]\n",Save_Data.N_S);
  printf("经度\t:[%.6f]\n",Save_Data.slongitude);
  printf("E/W\t:[%s]\n",Save_Data.E_W);
  printf("%.6f,%.6f\n",Save_Data.slongitude,Save_Data.slatitude);
  printf("***************************************\n");

}

void RST_Buffer(char *Buffer)
{
  memset(Buffer, 0, strlen(Buffer));
}

//********************************************************************
// WGS84=>BD09 地球坐标系=>百度坐标系 
int wgs2bd(double lat, double lon, double* pLat, double* pLon)
{
  double lat_ = 0.0, lon_ = 0.0;
  wgs2gcj(lat, lon, &lat_, &lon_);
  gcj2bd(lat_, lon_,  pLat, pLon);
  return 1;
}
 
// GCJ02=>BD09 火星坐标系=>百度坐标系  
int gcj2bd(double lat, double lon, double* pLat, double* pLon)
{
  double x_pi = 3.14159265358979324 * 3000.0 / 180.0;
  double x = lon, y = lat;
  double z = sqrt(x * x + y * y) + 0.00002 * sin(y * x_pi);
  double theta = atan2(y, x) + 0.000003 * cos(x * x_pi);
  *pLon = z * cos(theta) + 0.0065;
  *pLat = z * sin(theta) + 0.006;
  return 0;
}


// WGS84=>GCJ02 地球坐标系=>火星坐标系
int wgs2gcj(double lat, double lon, double* pLat, double* pLon)
{
  double pi = 3.14159265358979324;
  double ee = 0.00669342162296594323;
  double a = 6378245.0;
  if (outOfChina(lat,lon)){
    *pLat = lat;
    *pLon = lon;
    return 0;
  }
  double dLat = transformLat(lon - 105.0, lat - 35.0);
  double dLon = transformLon(lon - 105.0, lat - 35.0);
  double radLat = lat / 180.0 * pi;
  double magic = sin(radLat);
  magic = 1 - ee * magic * magic;
  double sqrtMagic = sqrt(magic);
  dLat = (dLat * 180.0) / ((a * (1 - ee)) / (magic * sqrtMagic) * pi);
  dLon = (dLon * 180.0) / (a / sqrtMagic * cos(radLat) * pi);
  *pLat = lat + dLat;
  *pLon = lon + dLon;
  return 1;
}
 
// 纬度偏移量
double transformLat(double x, double y)
{
  double pi = 3.14159265358979324;
  double ret = 0.0;
  ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * sqrt(fabs(x));
  ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
  ret += (20.0 * sin(y * pi) + 40.0 * sin(y / 3.0 * pi)) * 2.0 / 3.0;
  ret += (160.0 * sin(y / 12.0 * pi) + 320 * sin(y * pi  / 30.0)) * 2.0 / 3.0;
  return ret;
}
 
// 经度偏移量
double transformLon(double x, double y)
{
  double pi = 3.14159265358979324;
  double ret = 0.0;
  ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * sqrt(fabs(x));
  ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
  ret += (20.0 * sin(x * pi) + 40.0 * sin(x / 3.0 * pi)) * 2.0 / 3.0;
  ret += (150.0 * sin(x / 12.0 * pi) + 300.0 * sin(x / 30.0 * pi)) * 2.0 / 3.0;
  return ret;
}



/**
 * Description: 中国境外返回true,境内返回false
 * @param lon   经度
 * @param lat  纬度
 * @return
 */
int outOfChina(double lat, double lon) {
  if (lon < 72.004 || lon > 137.8347)
    return 1;
    if (lat < 0.8293 || lat > 55.8271)
      return 1;
  return 0;
}

double StringToIntToDu(char *LatorLon)
{
  double dat = atof(LatorLon);
  char integer = dat/100;
  double decimal = (dat-integer*100)/60;
  double ret = integer+decimal;
  return ret;
}


#endif