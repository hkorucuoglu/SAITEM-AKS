       /*
       Program isimi: AKS
       Aciklama: AKS CALISMALARI
       Konfigurasyon:
                     Mincrocontroller: Pic32mx764f128h
                     Oscillator      : 8.000 mhz
                     Version         : Microc for pic32 8.0
       */


#include <stdio.h>
#include <stdlib.h>
#include "stdint.h"
#include <string.h>

 //* Silecek Tanımlamalari *//
  unsigned int current_duty, pwm_period1, oran;
/*********************************************/

//* STATE TANIMLAMA *//
  int i;
  int KORNA_durum = 0;
  int SILECEK_durum = 0;
  int FAR_durum = 0;
/*************************/

//* HIZ Degiskenleri TANIMLA *//
int HALL_durum = 0;
unsigned long EskiZaman; 
const float Yaricap = 0.15; //metre
const float PI = 3.14;
float Cevre;
float Sure;
float ToplamYol=0;

/*************************/

//*  *//

/* UART'DAN BASILACAK VERILER*/
  unsigned int SICAKLIK = 61, AKIM=11, GERILIM=86;
  float HIZ = 0;
  float HUCRE_GERILIMLERI[32];
           /*Nextion Text Box*/

  char SicaklikTxtBox [] = "Sicaklik.val=";
  char HizTxtBox[] = "Hiz.val=";
  char AkimTxtBox[] = "Akim.val=";
  char GerilimTxtBox[] = "Gerilim.val=";
  char Hucreler [31][4];
/*************************/


#define INPUT 1
#define OUTPUT 0
#define ON 1
#define OFF 0
#define LOW 0
#define HIGH 1
#define Read 0
#define Write 1

  /* XBEE CONFIG */

#define START_DELIMITER 0x7E
#define FRAME_TYPE 0x90
#define SOURCE_ADDRESS_64_H 0x0013A200
#define SOURCE_ADDRESS_64_L 0x410AC893
#define SOURCE_ADDRESS_16 0x9D4D
#define RECEIVER_OPTIONS 0x01

uint16_t CheckSum = 0xCE;



  /********************/
      /**PIN TANIMLAMALARI**/
//sbit BUZZER at LATF3_bit;
//sbit BUZZER_Direction at TRISF3_bit;

#define DEBUG_LED1 PORTB.B5
#define DEBUG_LED1_Direction TRISB.B5
//////////////
#define DEBUG_LED2 PORTB.B4
#define DEBUG_LED2_Direction TRISB.B4
//////////////
#define DEBUG_LED3 PORTB.B2
#define DEBUG_LED3_Direction TRISB.B2
//////////////
#define DEBUG_LED4 PORTB.B3
#define DEBUG_LED4_Direction TRISB.B3
//////////////
#define FAR_CIKIS PORTB.B6
#define FAR_CIKIS_Direction TRISB.B6
//////////////
#define KORNA_CIKIS PORTB.B13
#define KORNA_CIKIS_Direction TRISB.B13
//////////////
#define SILECEK_CIKIS PORTB.B14
#define SILECEK_CIKIS_Direction TRISB.B14
/////////////
#define BUZZER PORTF.F3
#define BUZZER_Direction TRISF.F3
/////////////   INPUT ////////////////////
sbit KORNA at RE2_bit;
sbit KORNA_Direction at TRISE2_bit;
///
sbit SILECEK at RE3_bit;
sbit SILECEK_Direction at TRISE3_bit;
///
sbit FAR at RE4_bit;
sbit FAR_Direction at TRISE4_bit;

sbit SILECEK_PWM at RD0_bit;
sbit SILECEK_PWM_Direction at TRISD0_bit;

sbit HALL_SENSOR at RE0_bit;
sbit HALL_SENSOR_Direction at TRISE0_bit;

void test_acilma(){

  BUZZER = ON;
  DEBUG_LED1 = ON;
  Delay_ms(500);
  DEBUG_LED1 = OFF;
  DEBUG_LED2 = ON;
  Delay_ms(500);
  DEBUG_LED2 = OFF;
  DEBUG_LED3 = ON;
  Delay_ms(500);
  DEBUG_LED3 = OFF;
  DEBUG_LED4 = ON;
  Delay_ms(500);
  DEBUG_LED4 = OFF;
  BUZZER = OFF;
}
void pin_tanimlama(){
  DEBUG_LED1_Direction = OUTPUT;
  DEBUG_LED2_Direction = OUTPUT;
  DEBUG_LED3_Direction = OUTPUT;
  DEBUG_LED4_Direction = OUTPUT;
  BUZZER_Direction = OUTPUT;
  ////////////////////
  FAR_CIKIS_Direction = OUTPUT;
  SILECEK_CIKIS_Direction = OUTPUT;
  KORNA_CIKIS_Direction = OUTPUT;
  SILECEK_PWM_Direction = OUTPUT;
  ////
  KORNA_Direction = INPUT;
  SILECEK_Direction = INPUT;
  FAR_Direction = INPUT;
  HALL_SENSOR_Direction = INPUT;
}
void silecek_atamalari(){
  current_duty  = 54;
  pwm_period1 = PWM_Init(55, 1, 64, 2);
  oran = pwm_period1/1000;
}
void Silecek_Calistir(){
 while(current_duty < 110){
  current_duty ++;
  PWM_Set_Duty(oran*current_duty,1);
  Delay_ms(15);
  }
  while(current_duty > 54){
  current_duty --;
  PWM_Set_Duty(oran*current_duty,1);
  Delay_ms(15);
  }

    Delay_ms(1);  
}
void Silecek_Durdur(){
  if(current_duty > 54){
    while(current_duty == 82){
      current_duty --;
      PWM_Set_Duty(oran*current_duty,1);
      Delay_ms(15);
    }
  }
  else if(current_duty < 110){
    while(current_duty == 82){
      current_duty ++;
      PWM_Set_Duty(oran*current_duty,1);
      Delay_ms(15);
    }
  }
}

void Uart4_Tanimlama(){
  UART4_Init(9600);
  Delay_ms(100);
}
void Uart3_Tanimlama(){
  UART3_Init(9600);
  Delay_ms(100);
}
void ondalikAyir(int veri, int *Basamaklar){
	Basamaklar[0] = veri/100;
	Basamaklar[1] = veri/10;
	Basamaklar[2] = veri - (100*Basamaklar[0] + 10*Basamaklar[1]);
}

void ChrDonustur(int veri, char *sayiDizi){
	int i;
	int basamaklar[3];
	ondalikAyir(veri, basamaklar);
	for (i = 0; i < 3; i++)	sayiDizi[i] = basamaklar[i] + '0';
}

void ChrBirlestir(char *dizi_1, char *dizi_2, int dizi_1_u, int dizi_2_u, char *Sonuc){
	int i, k;
	//printf(" dizi 1 uzunluk :%d\n", dizi_1_u);
	for (i = 0; i < dizi_1_u; i++) {
		Sonuc[i] = dizi_1[i];
	}
	//printf(" dizi 2 uzunluk :%d\n", dizi_2_u);
	for (k = 0; k <= dizi_2_u; k++, i++){
		Sonuc[i] = dizi_2[k];
	}
	Sonuc[i] = '\0';
}


void Sicakik_Yaz(){
  char SicaklikVeriDizi [3];
  char Sonuc[20];
  ChrDonustur(SICAKLIK, SicaklikVeriDizi);
  ChrBirlestir(SicaklikTxtBox,SicaklikVeriDizi,sizeof(SicaklikTxtBox)-1,sizeof(SicaklikVeriDizi)-1,Sonuc);
  UART4_Write_Text(Sonuc);
  UART4_Write(0xFF);
  UART4_Write(0xFF);
  UART4_Write(0xFF);
}
void Hiz_Yaz(){
  char HizVeriDizi [3];
  char Sonuc[20];
  ChrDonustur(HIZ,HizVeriDizi);
  ChrBirlestir(HizTxtBox,HizVeriDizi,sizeof(HizTxtBox)-1,sizeof(HizVeriDizi)-1,Sonuc);
  UART4_Write_Text(Sonuc);
  UART4_Write(0xFF);
  UART4_Write(0xFF);
  UART4_Write(0xFF);
}
void Akim_Yaz(){
  char AkimVeriDizi [3];
  char Sonuc[20];
  ChrDonustur(AKIM,AkimVeriDizi);
  ChrBirlestir(AkimTxtBox,AkimVeriDizi,sizeof(AkimTxtBox)-1,sizeof(AkimVeriDizi)-1,Sonuc);
  UART4_Write_Text(Sonuc);
  UART4_Write(0xFF);
  UART4_Write(0xFF);
  UART4_Write(0xFF);

}
void Gerilim_Yaz(){
  char GerilimVeriDizi [3];
  char Sonuc[20];
  ChrDonustur(GERILIM,GerilimVeriDizi);
  ChrBirlestir(GerilimTxtBox,GerilimVeriDizi,sizeof(GerilimTxtBox)-1,sizeof(GerilimVeriDizi)-1,Sonuc);
  UART4_Write_Text(Sonuc);
  UART4_Write(0xFF);
  UART4_Write(0xFF);
  UART4_Write(0xFF);

}
void Ekran_Veri_Yaz(){
     //Sicakik_Yaz();
     Hiz_Yaz();
     //Akim_Yaz();
     //Gerilim_Yaz();
}
void XbeeVeriGonder(){
 
UART3_Write_Text("merhaba");

}
void HALL_TEST(){
  if (HALL_SENSOR == HIGH)
  {
    BUZZER = ON;
    Delay_ms(1000);
    BUZZER = OFF;
  }
}
unsigned long millis_sayac=0;
void InitTimer1(){
  T1CON	 = 0x8000;
  T1IP0_bit	 = 1;
  T1IP1_bit	 = 1;
  T1IP2_bit	 = 1;
  T1IF_bit	 = 0;
  T1IE_bit	 = 1;
  PR1		 = 8000;
  TMR1		 = 0;
}
 
void Timer1Interrupt() iv IVT_TIMER_1 ilevel 7 ics ICS_SRS {
  T1IF_bit	 = 0;
  //Enter your code here 
  millis_sayac++;
}

unsigned long millis()
{
  return(millis_sayac); 
}
void Cevre_Hesapla(){
  Cevre=2*PI*Yaricap;
}
void Hiz_Hesapla(){
  
  if (HALL_SENSOR == 1 && HALL_durum == 0){
  EskiZaman = millis();
  HALL_durum = 1;
  }
  else if (HALL_SENSOR == 1 && HALL_durum == 1 && millis() - EskiZaman > 10){    
    Sure= (millis() - EskiZaman)/1000.f;
    HIZ = (Cevre/Sure) * 3.6;      
    HALL_durum = 0;
    DEBUG_LED1 = ON;
  }
  else if (HALL_SENSOR == 0 && millis() - EskiZaman > 2000)
  {
    HIZ = 0;
  }
}
void ilkKonfig(){

  UART3_Write(0x7E);
  UART3_Write(0x00);
  UART3_Write(0x04);
  
  UART3_Write(0x08);
  UART3_Write(0x01);
  UART3_Write(0x41);
  
  UART3_Write(0x50);
  UART3_Write(0x65);
}



void main(){

  pin_tanimlama();
  test_acilma();
  silecek_atamalari();
  Uart4_Tanimlama(); // EKRAN
  Uart3_Tanimlama(); // XBEE
  ilkKonfig();
  InitTimer1();
  EnableInterrupts();  
  Cevre_Hesapla();
  //UART3_Write_Text("merhaba");
  while(1){

     Hiz_Hesapla();
     Ekran_Veri_Yaz();
     XbeeVeriGonder();
     //HALL_TEST();

                  /*KORNA*/
    if(KORNA == ON){
      KORNA_durum = 1;
      KORNA_CIKIS = ON;
    }
    else if (KORNA == OFF && KORNA_durum){             
      KORNA_CIKIS = OFF;
      KORNA_durum = 0;
    }
                  /*SILECEK*/
    if(SILECEK == ON){
      PWM_Start(1);
      Silecek_Calistir();
      SILECEK_durum = 1;
      }
    else if(SILECEK == OFF && SILECEK_durum){

      Silecek_Durdur();
      Pwm_Stop(1);
      SILECEK_durum = 0;
    }
                  /*FAR*/         
    if(FAR == ON){
      FAR_durum = 1;
      FAR_CIKIS = ON;
      }
    else if(FAR == OFF && FAR_durum){
      ///
      FAR_CIKIS = OFF;
      ///
      FAR_durum = 0;
    }

  }

}

