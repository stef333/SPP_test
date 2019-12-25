/*
 * spp.c
 *
 * Created: 07/09/2019 14:30:52
 * Author : Stefan
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 1000000UL
#include <stdlib.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/eeprom.h>

#define regsel  6
#define enable  4

#define enable_driver 5
#define in1 4
#define in2 7
#define buzzer 6 // on potrC
#define cLED 3 // port D
#define zLED 3 // port C
#define door_btn  2 // pord D

#define BAUD 9600                                   // define baud
#define BAUDRATE ((F_CPU)/(BAUD*16UL)-1)

void cmd(unsigned char);
void print_ch(unsigned char);
void send_a_string(char *str);
unsigned char reverse(unsigned char n);
void USART_Init( void );
unsigned char USART_Receive( void );
void Lcd8_Init();

static unsigned char lookup[16] = {
	0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf, };

char ID[4];
/*char korisnik_1[4];
char korisnik_2[4];
char korisnik_3[4];
char korisnik_4[4];*/

int main(void)
{
	/*** declaration ***/
	ID[0] = (eeprom_read_byte((uint8_t*)20));	// 0 or 1
	ID[1] = (eeprom_read_byte((uint8_t*)21));
	ID[2] = (eeprom_read_byte((uint8_t*)22));
	ID[3] = (eeprom_read_byte((uint8_t*)23));

	char u_sifra [4];
	char last_op='r';
	char last_kor=0;
	char ADMIN[4]={0,0,0,0};


//	char taster_A=0;
	char taster_B=0;
	char taster_C=0;
	char taster_D=0;

	char key=0;
	int key_count=0;
	int keypressed=0;
	int taster = 0;
	char admin_tast=0;
	int admin_menu=0;
	char add_or_rem_menu=0;
	char _temp='r';
						 // greska na prvoj kartici (e2 posletnji bajt)
	char ADMIT [5][5]={{(0x82),(0x6),(0xc2),(0x06),(0xe)},
					   {(0x82),(0x06),(0xc2),(0x46),(0xda)},
					   {(0x82),(0x06),(0xc2),(0x46),(0xea)}, 
					   {(0x82),(0x06),(0xc2),(0x46),(0xe2)},
					   {(0x82),(0x06),(0xc2),(0x06),(0x0a)}};
	char COUNTA;
	char SHOWA [1];
	char MEM[5];

    /*** init ***/
    DDRA =0xF0;	// keypad
	DDRB = 0xFF;	// LCD
	DDRC = 0xFF;
	DDRD = 0b11111010;	// rfid

	PORTA=0x0F;
	PORTB = 0;
	PORTC = 0x00;

	USART_Init();
	Lcd8_Init();

	PORTC |= (1<<enable_driver);
	_delay_ms(10.0);

	/*** program ***/
	while (1)
	{
		cmd(0x01);
		send_a_string("card or password");
		cmd(0x80 + 0x40);

		while(!(UCSRA&(1<<RXC)))  // zabraniti karticu ako je pritisnut neki taster 1,2,3...
		{
			if ((PINA != 0b00001111) )
			{
				if(taster == 0) 	// da ne ocita vise pritisaka
				{
					taster=1;
					keypressed = PINA;
					DDRA ^=0b11111111;
					PORTA ^= 0b11111111;
					keypressed |= PINA;
					_delay_ms(1);

					if (keypressed==0b11101110)
					{
						send_a_string("D");//if row1 and column1 is high show ?1?
						taster_D=1;
						cmd(0x01);
						send_a_string("unesi sifru");
						_delay_ms(500.0);
						cmd(0x80 + 0x40);
					}else if (keypressed==0b11101101)
					{
						send_a_string("C");// if row1 and column2 is high show ?4?
						taster_C=1;
						//---------------promijenio zadnji put------------------------------
						cmd(0x01);
						send_a_string("Enter old pass");
						cmd(0x80 + 0x40);
						//------------------------------------------------------------------
					}else if (keypressed==0b11101011)
					{
						send_a_string("B");		// B
						taster_B=1;
					}else if (keypressed==0b11100111)
					{
						send_a_string("A");		//A
					}else if (keypressed==0b11011110)
					{
						send_a_string("REM");	// #
						admin_tast='x';
					}else if (keypressed==0b11011101)
					{
						u_sifra[key_count++]=9;
						send_a_string("9");		// 9
					}else if (keypressed==0b11011011)
					{
						u_sifra[key_count++]=6;
						send_a_string("6");		// 6
					}else if (keypressed==0b11010111)
					{
						u_sifra[key_count++]=3;
						send_a_string("3");		// 3
					}else if (keypressed==0b10111110)
					{
						u_sifra[key_count++]=0;
						//send_a_string("0");	
						key="0";	// 0
					}else if (keypressed==0b10111101)
					{
						u_sifra[key_count++]=8;
						send_a_string("8");		// 8
					}else if (keypressed==0b10111011)
					{
						u_sifra[key_count++]=5;
						send_a_string("5");		// 5
					}else if (keypressed==0b10110111)
					{
						u_sifra[key_count++]=2;
						send_a_string("2");		// 2
					}else if (keypressed==0b01111110)
					{
						send_a_string("ADD");	// *
						admin_tast='y';
					}else if (keypressed==0b01111101)
					{
						u_sifra[key_count++]=7;			// 7
						send_a_string("7");
					}else if (keypressed==0b01111011)
					{	
						u_sifra[key_count++]=4;			// 4
						send_a_string("4");
					}else if (keypressed == 0b01110111)
					{
						u_sifra[key_count++]=1;			// 1
						//send_a_string("1");
						key="1";
					}
					keypressed=0;
					DDRA ^=0b11111111;
					_delay_ms(1);
					PORTA ^= 0b11111111;

					//---------ADMIN------------
					if (taster_B)		
					{
						cmd(0x01);
						send_a_string("ADD or REM User"); // OMOGUCI KORISNIKA
						cmd(0x80 + 0x40);
						itoa(ID[0], SHOWA, 10);
						send_a_string(SHOWA);
						itoa(ID[1], SHOWA, 10);
						send_a_string(SHOWA);
						itoa(ID[2], SHOWA, 10);
						send_a_string(SHOWA);
						itoa(ID[3] ,SHOWA, 10);
						send_a_string(SHOWA);
						send_a_string(" | ");
				//		_delay_ms(500.0);
						key_count=0;
						add_or_rem_menu =1;
					}
					if (add_or_rem_menu)
					{
						taster_B=0;
						if (key_count==4)
						{
							add_or_rem_menu=0;
							eeprom_update_byte((uint8_t*)20, u_sifra[0]);
							eeprom_update_byte((uint8_t*)21, u_sifra[1]);
							eeprom_update_byte((uint8_t*)22, u_sifra[2]);
							eeprom_update_byte((uint8_t*)23, u_sifra[3]);
							_delay_ms(500.0);
							key_count=0;
							cmd(0x01);
							send_a_string("card or password");// ADD or REM User kraj
							cmd(0x80 + 0x40);
							ID[0] = (eeprom_read_byte((uint8_t*)20));
							ID[1] = (eeprom_read_byte((uint8_t*)21));
							ID[2] = (eeprom_read_byte((uint8_t*)22));
							ID[3] = (eeprom_read_byte((uint8_t*)23));

						}
					}

					if (taster_D)
					{
					//	cmd(0x01);
						if(key_count==4)
						{

							if((u_sifra[0] == ADMIN[0])&&(u_sifra[1] == ADMIN[1]) && (u_sifra[2] == ADMIN[2])&&(u_sifra[3] == ADMIN[3]))
						    {
								cmd(0x01);
								send_a_string("Welcome admin");
								_delay_ms(500.0);
								taster_D=0;
								cmd(0x01);
								itoa(ID[0], SHOWA, 10);
								send_a_string(SHOWA);
								itoa(ID[1], SHOWA, 10);
								send_a_string(SHOWA);
								itoa(ID[2], SHOWA, 10);
								send_a_string(SHOWA);
								itoa(ID[3] ,SHOWA, 10);
								send_a_string(SHOWA);
								cmd(0x80 + 0x40);
					//			send_a_string("ADD or CHANGE");//pasword
								key_count=0;
								
								cmd(0x01);
								// send_a_string("kor");//1000,0100,0010,0001 ; 1 je izabrani korisnik
								cmd(0x80 + 0x40);
								_delay_ms(250.0);
								cmd(0x01);
								admin_menu =1;
							}else
							{
								cmd(0x01);
								send_a_string("pogresna sifra");
								key_count=0;
								taster_D=0;
								_delay_ms(750.0);
								cmd(0x01);
								send_a_string("card or password");
								cmd(0x80 + 0x40);
							}
						}

					}
					if (admin_menu)
					{
					//	cmd(0x01);
						/*itoa(u_sifra[0], SHOWA, 10);
						send_a_string(SHOWA);
						itoa(u_sifra[1], SHOWA, 10);
						send_a_string(SHOWA);
						itoa(u_sifra[2], SHOWA, 10);
						send_a_string(SHOWA);
						itoa(u_sifra[3] ,SHOWA, 10);
						send_a_string(SHOWA);*/
					//	send_a_string("izaberi kori");//1000,0100,0010,0001 ; 1 je izabrani korisnik
						if (key_count==4)
						{
							admin_menu=0;
							cmd(0x01);
							send_a_string("izabrani kor");
							cmd(0x80 + 0x40);


							itoa(u_sifra[0], SHOWA, 10);
							send_a_string(SHOWA);
							itoa(u_sifra[1], SHOWA, 10);
							send_a_string(SHOWA);
							itoa(u_sifra[2], SHOWA, 10);
							send_a_string(SHOWA);
							itoa(u_sifra[3] ,SHOWA, 10);
							send_a_string(SHOWA);
							_delay_ms(500.0);

							if (u_sifra[0]==1)
							{
								cmd(0x01);
								send_a_string("korisnik 1");
								_delay_ms(500.0);
								_temp='0';
								admin_tast=1;
								key_count=0;
								cmd(0x01);
								send_a_string("unesi sifru");
							}else if (u_sifra[1]==1)
							{
								cmd(0x01);
								send_a_string("korisnik 2");
								_delay_ms(500.0);
								_temp='1';
								admin_tast=1;
								key_count=0;
							}else if (u_sifra[2]==1)
							{
								cmd(0x01);
								send_a_string("korisnik 3");
								_delay_ms(500.0);
								_temp='2';
								admin_tast=1;
								key_count=0;
							}else if ((u_sifra[3]==1))
							{
								cmd(0x01);
								send_a_string("korisnik 4");
								_delay_ms(500.0);
								_temp='3';
								admin_tast=1;
								key_count=0;
							}
							/*else
							{
								continue;
							}*/
						}
					}
					//cmd(0x01);
					//send_a_string("unesi sifru");
					if (admin_tast)
					{
						
						//admin unosi sifru za prethodno definisanog korisnika
						send_a_string(key);

						if(key_count==4)
						{
							admin_tast=0;
							switch(_temp)
							{
								case '0':
									cmd(0x01);
									send_a_string("Korisnik 1:");
									eeprom_update_byte((uint8_t*)0, u_sifra[0]);
									eeprom_update_byte((uint8_t*)1, u_sifra[1]);
									eeprom_update_byte((uint8_t*)2, u_sifra[2]);
									eeprom_update_byte((uint8_t*)3, u_sifra[3]);

									/*cmd(0x80 + 0x40);
									itoa((eeprom_read_byte((uint8_t*)0)), SHOWA, 10);
									send_a_string(SHOWA);
									itoa((eeprom_read_byte((uint8_t*)1)), SHOWA, 10);
									send_a_string(SHOWA);
									itoa((eeprom_read_byte((uint8_t*)2)), SHOWA, 10);
									send_a_string(SHOWA);
									itoa((eeprom_read_byte((uint8_t*)3)), SHOWA, 10);
									send_a_string(SHOWA);
									_delay_ms(1500.0);*/
									key_count=0;
									_temp='r';
									cmd(0x01);
									send_a_string("card or password");
									cmd(0x80 + 0x40);
									break;

								case '1':
									cmd(0x01);
									send_a_string("Korisnik 2:");
									eeprom_update_byte((uint8_t*)4, u_sifra[0]);
									eeprom_update_byte((uint8_t*)5, u_sifra[1]);
									eeprom_update_byte((uint8_t*)6, u_sifra[2]);
									eeprom_update_byte((uint8_t*)7, u_sifra[3]);
									cmd(0x80 + 0x40);

									itoa((u_sifra[0]), SHOWA, 10);
									send_a_string(SHOWA);
									itoa((u_sifra[1]), SHOWA, 10);
									send_a_string(SHOWA);
									itoa((u_sifra[2]), SHOWA, 10);
									send_a_string(SHOWA);
									itoa((u_sifra[3]), SHOWA, 10);
									send_a_string(SHOWA);
									_delay_ms(1500.0);
									key_count=0;
									_temp='r';
									cmd(0x01);
									send_a_string("card or password");
									cmd(0x80 + 0x40);
									break;

								case '2':
									cmd(0x01);
									send_a_string("Korisnik 3:");
									eeprom_update_byte((uint8_t*)8, u_sifra[0]);
									eeprom_update_byte((uint8_t*)9, u_sifra[1]);
									eeprom_update_byte((uint8_t*)10, u_sifra[2]);
									eeprom_update_byte((uint8_t*)11, u_sifra[3]);
									cmd(0x80 + 0x40);

									itoa((eeprom_read_byte((uint8_t*)8)), SHOWA, 10);
									send_a_string(SHOWA);
									itoa((eeprom_read_byte((uint8_t*)9)), SHOWA, 10);
									send_a_string(SHOWA);
									itoa((eeprom_read_byte((uint8_t*)10)), SHOWA, 10);
									send_a_string(SHOWA);
									itoa((eeprom_read_byte((uint8_t*)11)), SHOWA, 10);
									send_a_string(SHOWA);
									_delay_ms(1500.0);
									key_count=0;
									_temp='r';
									cmd(0x01);
									send_a_string("card or password");
									cmd(0x80 + 0x40);
									break;

								case '3':
									cmd(0x01);
									send_a_string("Korisnik 4:");
									eeprom_update_byte((uint8_t*)12, u_sifra[0]);
									eeprom_update_byte((uint8_t*)13, u_sifra[1]);
									eeprom_update_byte((uint8_t*)14, u_sifra[2]);
									eeprom_update_byte((uint8_t*)15, u_sifra[3]);
									cmd(0x80 + 0x40);

									itoa((eeprom_read_byte((uint8_t*)12)), SHOWA, 10);
									send_a_string(SHOWA);
									itoa((eeprom_read_byte((uint8_t*)13)), SHOWA, 10);
									send_a_string(SHOWA);
									itoa((eeprom_read_byte((uint8_t*)14)), SHOWA, 10);
									send_a_string(SHOWA);
									itoa((eeprom_read_byte((uint8_t*)15)), SHOWA, 10);
									send_a_string(SHOWA);
									_delay_ms(1500.0);
									key_count=0;
									_temp='r';
									cmd(0x01);
									send_a_string("card or password");
									cmd(0x80 + 0x40);
									break;
								default:
									break;
								}

							}
						}



					//--------------KRAJ ADMINA---------------------
					if (taster_C)
					{
						//cmd(0x01); --promijenio zadnji put-----------------------------------------
						//send_a_string("Enter old pass");
						//cmd(0x80 + 0x40);
						if (key_count==4)
						{
							if (((u_sifra[0] == (eeprom_read_byte((uint8_t*)0))) && (u_sifra[1] == (eeprom_read_byte((uint8_t*)1))) && (u_sifra[2] == (eeprom_read_byte((uint8_t*)2))) && (u_sifra[3] ==(eeprom_read_byte((uint8_t*)3))) && ((ID[0]==1))&&(key_count==4)))
							{
								last_kor=1;
								last_op='1';
								key_count=0;
								taster_C=0;
								// --promijenio zadnji put-----------------------------------------------------------
								_delay_ms(300.0);
								cmd(0x01);
								send_a_string("Enter new pass");
								cmd(0x80 + 0x40);
								//-------------------------------------------------------------------------------
							}else if (((u_sifra[0] == (eeprom_read_byte((uint8_t*)4))) && (u_sifra[1] == (eeprom_read_byte((uint8_t*)5))) && (u_sifra[2] == (eeprom_read_byte((uint8_t*)6))) && (u_sifra[3] == (eeprom_read_byte((uint8_t*)7)) && ((ID[1]==1))))&&(key_count==4))
							{
								last_kor=1;
								last_op='2';
								key_count=0;
								taster_C=0;
								// --promijenio zadnji put-----------------------------------------------------------
								_delay_ms(100.0);
								cmd(0x01);
								send_a_string("Enter new pass");
								cmd(0x80 + 0x40);
								//-------------------------------------------------------------------------------
							}else if ((u_sifra[0] == (eeprom_read_byte((uint8_t*)8))) && (u_sifra[1] == (eeprom_read_byte((uint8_t*)9))) && (u_sifra[2] == (eeprom_read_byte((uint8_t*)10))) && (u_sifra[3] ==(eeprom_read_byte((uint8_t*)11)) && ((ID[2]==1)))&&(key_count==4))
							{
								last_kor=1;
								last_op='3';
								key_count=0;
								taster_C=0;
								// --promijenio zadnji put-----------------------------------------------------------
								_delay_ms(100.0);
								cmd(0x01);
								send_a_string("Enter new pass");
								cmd(0x80 + 0x40);
								//-------------------------------------------------------------------------------
							}else if (((u_sifra[0] == (eeprom_read_byte((uint8_t*)12))) && (u_sifra[1] == (eeprom_read_byte((uint8_t*)13))) && (u_sifra[2] == (eeprom_read_byte((uint8_t*)14))) && (u_sifra[3] == (eeprom_read_byte((uint8_t*)15))) && ((ID[3]==1)))&&(key_count==4))
							{
								last_kor=1;
								last_op='4';
								key_count=0;
								taster_C=0;
								// --promijenio zadnji put-----------------------------------------------------------
								_delay_ms(100.0);
								cmd(0x01);
								send_a_string("Enter new pass");
								cmd(0x80 + 0x40);
								//-------------------------------------------------------------------------------
							}else if(key_count>=4)
							{
							  cmd(0x01);
								send_a_string("invalid pass");
								cmd(0x80 + 0x40);
								send_a_string("please try again");
								_delay_ms(650.0);
								key_count=0;
								cmd(0x01);
								send_a_string("card or password");
							}


							}
						}
					    
						if (last_kor)
						{
							//cmd(0x01);   --promijenio zadnji put-----------------------------------------
							//send_a_string("Enter new pass");
							//cmd(0x80 + 0x40);
							//--------------------------------------------------------------------------
							if (key_count==4)
							{
								last_kor=0;
								switch (last_op)
								{
									case '1' :
										cmd(0x01);
										send_a_string("nova");
										cmd(0x80 + 0x40);
										itoa(u_sifra[0], SHOWA, 10);
										send_a_string(SHOWA);
										itoa(u_sifra[1], SHOWA, 10);
										send_a_string(SHOWA);
										itoa(u_sifra[2], SHOWA, 10);
										send_a_string(SHOWA);
										itoa(u_sifra[3], SHOWA, 10);
										send_a_string(SHOWA);

										eeprom_update_byte((uint8_t*) 0, u_sifra[0]);
										eeprom_update_byte((uint8_t*) 1, u_sifra[1]);
										eeprom_update_byte((uint8_t*) 2, u_sifra[2]);
										eeprom_update_byte((uint8_t*) 3, u_sifra[3]);
										_delay_ms(1000.0);
										key_count=0;
										last_op='r';
										cmd(0x01);
										send_a_string("card or password");
										cmd(0x80 + 0x40);
										_delay_ms(1500.0);
										break;

									case '2' :
										cmd(0x01);
										send_a_string("nova");
										cmd(0x80 + 0x40);
										itoa(u_sifra[0], SHOWA, 10);
										send_a_string(SHOWA);
										itoa(u_sifra[1], SHOWA, 10);
										send_a_string(SHOWA);
										itoa(u_sifra[2], SHOWA, 10);
										send_a_string(SHOWA);
										itoa(u_sifra[3], SHOWA, 10);
										send_a_string(SHOWA);

										eeprom_update_byte((uint8_t*) 4, u_sifra[0]);
										eeprom_update_byte((uint8_t*) 5, u_sifra[1]);
										eeprom_update_byte((uint8_t*) 6, u_sifra[2]);
										eeprom_update_byte((uint8_t*) 7, u_sifra[3]);
										_delay_ms(1000.0);
										key_count=0;
										last_op='r';
										cmd(0x01);
										send_a_string("card or password");
										cmd(0x80 + 0x40);
										_delay_ms(1500.0);
										break;

									case '3' :
										cmd(0x01);
										send_a_string("nova");
										cmd(0x80 + 0x40);
										itoa(u_sifra[0], SHOWA, 10);
										send_a_string(SHOWA);
										itoa(u_sifra[1], SHOWA, 10);
										send_a_string(SHOWA);
										itoa(u_sifra[2], SHOWA, 10);
										send_a_string(SHOWA);
										itoa(u_sifra[3], SHOWA, 10);
										send_a_string(SHOWA);

										eeprom_update_byte((uint8_t*) 8, u_sifra[0]);
										eeprom_update_byte((uint8_t*) 9, u_sifra[1]);
										eeprom_update_byte((uint8_t*) 10, u_sifra[2]);
										eeprom_update_byte((uint8_t*) 11, u_sifra[3]);
										_delay_ms(1000.0);
										key_count=0;
										last_op='r';
										cmd(0x01);
										send_a_string("card or password");
										cmd(0x80 + 0x40);
										_delay_ms(1500.0);
										break;

									case '4' :
										cmd(0x01);
										send_a_string("nova");
										cmd(0x80 + 0x40);
										itoa(u_sifra[0], SHOWA, 10);
										send_a_string(SHOWA);
										itoa(u_sifra[1], SHOWA, 10);
										send_a_string(SHOWA);
										itoa(u_sifra[2], SHOWA, 10);
										send_a_string(SHOWA);
										itoa(u_sifra[3], SHOWA, 10);
										send_a_string(SHOWA);

										eeprom_update_byte((uint8_t*) 12, u_sifra[0]);
										eeprom_update_byte((uint8_t*) 13, u_sifra[1]);
										eeprom_update_byte((uint8_t*) 14, u_sifra[2]);
										eeprom_update_byte((uint8_t*) 15, u_sifra[3]);
										_delay_ms(1000.0);
										key_count=0;
										last_op='r';
										cmd(0x01);
										send_a_string("card or password");
										cmd(0x80 + 0x40);
										_delay_ms(1500.0);
										break;

									default:
										break;
								}
							}
						}



					// ---------------------------provjera sifre-----------------------------

					if (key_count==4)
					{

							if (!((((u_sifra[0] == (eeprom_read_byte((uint8_t*)0))) && (u_sifra[1] == (eeprom_read_byte((uint8_t*)1))) && (u_sifra[2] == (eeprom_read_byte((uint8_t*)2))) && (u_sifra[3] ==(eeprom_read_byte((uint8_t*)3)) && ((ID[0]==1))))) |
								   (((u_sifra[0] == (eeprom_read_byte((uint8_t*)4))) && (u_sifra[1] == (eeprom_read_byte((uint8_t*)5))) && (u_sifra[2] == (eeprom_read_byte((uint8_t*)6))) && (u_sifra[3] == (eeprom_read_byte((uint8_t*)7)) && ((ID[1]==1))))) |
								   (((u_sifra[0] == (eeprom_read_byte((uint8_t*)8))) && (u_sifra[1] == (eeprom_read_byte((uint8_t*)9))) && (u_sifra[2] == (eeprom_read_byte((uint8_t*)10))) && (u_sifra[3] ==(eeprom_read_byte((uint8_t*)11)) && ((ID[2]==1))))) |
								   (((u_sifra[0] == (eeprom_read_byte((uint8_t*)12))) && (u_sifra[1] == (eeprom_read_byte((uint8_t*)13))) && (u_sifra[2] == (eeprom_read_byte((uint8_t*)14))) && (u_sifra[3] == (eeprom_read_byte((uint8_t*)15))) && ((ID[3]==1))))) |
								   (((u_sifra[0] == ADMIN[0])&&(u_sifra[1] == ADMIN[1]) && (u_sifra[2] == ADMIN[2])&&(u_sifra[3] == ADMIN[3]))))
							{
								cmd(0x01);
								send_a_string("wrong password");
								cmd(0x80 + 0x40);
								send_a_string("please try again");
					//			PORTC |= (1<<buzzer);
					//			PORTD |= (1<<cLED);
					//			_delay_ms(400.0);
					//			PORTC &= ~(1<<buzzer);
					//			PORTD &= ~(1<<cLED);
					//			_delay_ms(4000.0);
								key_count=0;
								_delay_ms(2000.0);

								cmd(0x01);
								send_a_string("card or password");
								cmd(0x80 + 0x40);
							}else
							{
								cmd(0x01);
								send_a_string("open");
								//cmd(0x80 + 0x40);
								key_count=0;
								_delay_ms(200.0);
								// Otvori vrata
								// cekaj dok se ne zatvore
								cmd(0x01);
								send_a_string("car or password");
								cmd(0x80 + 0x40);
								
							}

						}

					}
		  	}else
			  {
				  taster = 0;
			  }

		  }
		// 1
		COUNTA=UDR;
		MEM[0]=COUNTA;
		itoa(COUNTA,SHOWA,16);
		send_a_string(SHOWA);
		itoa(COUNTA,SHOWA,16);
		send_a_string(SHOWA);
		_delay_ms(4.0);

		while(!(UCSRA&(1<<RXC)));	// 2
		COUNTA=UDR;
		MEM[1]=COUNTA;
		itoa(COUNTA,SHOWA,16);
		send_a_string(SHOWA);
		_delay_ms(4.0);

		while(!(UCSRA&(1<<RXC)));	// 3
		COUNTA=UDR;
		MEM[2]=COUNTA;
		itoa(COUNTA,SHOWA,16);
		send_a_string(SHOWA);
		_delay_ms(4.0);

		while(!(UCSRA&(1<<RXC)));	// 4
		COUNTA=UDR;
		MEM[3]=COUNTA;
		itoa(COUNTA,SHOWA,16);
		send_a_string(SHOWA);
		_delay_ms(4.0);

		while(!(UCSRA&(1<<RXC)));	// 5
		COUNTA=UDR;
		MEM[4]=COUNTA;
		itoa(COUNTA,SHOWA,16);
		send_a_string(SHOWA);
		_delay_ms(4.0);

		UCSRB &=~(1<<RXEN);
		//PORTC=0x00;
		for (int i=0;i<5;i++)
		{
			if ((MEM[0]==ADMIT[i][0])&(MEM[1]==ADMIT[i][1])&(MEM[2]==ADMIT[i][2])&(MEM[3]==ADMIT[i][3])&(MEM[4]==ADMIT[i][4])) // & id[i] == 1
			{
				i=5;
				cmd(0x01);
				send_a_string("ADMIN");

				key_count=0;

				PORTC |= (1<<buzzer);
				PORTC |= (1<<zLED);
				_delay_ms(100.0);
				PORTC &= ~(1<<buzzer);
				PORTC &= ~(1<<zLED);
				_delay_ms(100.0);
				PORTC |= (1<<buzzer);
				PORTC |= (1<<zLED);
				_delay_ms(100.0);
				PORTC &= ~(1<<buzzer);
				_delay_ms(100.0);

				// ON motor
				PORTC |= (1<<in1);
				_delay_ms(500.0);
				PORTC &= ~(1<<in1); // off
				//	PORTD |= (1<<in2);
				//	_delay_ms(500.0);
				//	PORTD &= ~(1<<in2); // off

				while ((PIND & (1<<door_btn)))		//if pressed btn -> close door
				{
					_delay_ms(50.0);
				}

				PORTD |= (1<<in2);
				_delay_ms(500.0);
				PORTD &= ~(1<<in2); // off
				PORTC &= ~(1<<zLED);

			}else if (i==4)
			{
				//send_a_string("Greska");

				key_count=0;

				PORTC |= (1<<buzzer);
				PORTD |= (1<<cLED);
				_delay_ms(400.0);
				PORTC &= ~(1<<buzzer);
				PORTD &= ~(1<<cLED);
				_delay_ms(4000.0);
			}
		}
		UCSRB |=(1<<RXEN);
		}
	}


void cmd(unsigned char command)
{
	command = reverse(command);
	PORTB = command;
	PORTD &= ~ (1<<regsel);
	PORTD |= (1<<enable);
	_delay_ms(1.0); //
	PORTD &= ~1<<enable;
	_delay_ms(1.0);
	PORTB = 0;
}

void print_ch(unsigned char ch)
{
	ch = reverse(ch);
	PORTB = ch;
	PORTD |= (1<<regsel);//telling LCD we are sending data not commands
	PORTD |= (1<<enable);//telling LCD to start receiving command/data
	_delay_ms(1.0);
	PORTD &= ~(1<<enable);//telling lcd we completed sending data/command
	_delay_ms(1.0);
	PORTB = 0;
}

void send_a_string(char *str)
{
	while(*str > 0)
	{
		print_ch(*str++);
	}
}

void USART_Init( void )
{
	/* Set baud rate */
	UBRRH = (unsigned char)(6>>8);
	UBRRL = (unsigned char)6;
	/* Enable receiver and transmitter */
	UCSRB = (1<<RXEN)|(1<<TXEN);
	/* Set frame format: 8data, 2stop bit */
	UCSRC = (1<<URSEL)|(3<<UCSZ0);
}
unsigned char USART_Receive( void )
{
	/* Wait for data to be received */
	while ( !(UCSRA & (1<<RXC)) );
	/* Get and return received data from buffer */
	return UDR;
}

unsigned char reverse(unsigned char n) {
	// Reverse the top and bottom nibble then swap them.
	return (lookup[n&0b1111] << 4) | lookup[n>>4];
}

void Lcd8_Init()
{
	cmd(0x30);
	cmd(0x38);    //function set
	cmd(0x0C);    //display on,cursor off,blink off
	cmd(0x01);    //clear display
	cmd(0x06);    //entry mode, set increment
}

/*	korisnik_1[0] = (eeprom_read_byte((uint8_t*)0));
	korisnik_1[1] = (eeprom_read_byte((uint8_t*)1));
	korisnik_1[2] = (eeprom_read_byte((uint8_t*)2));
	korisnik_1[3] = (eeprom_read_byte((uint8_t*)3));

	korisnik_2[0] = (eeprom_read_byte((uint8_t*)4));
	korisnik_2[1] = (eeprom_read_byte((uint8_t*)5));
	korisnik_2[2] = (eeprom_read_byte((uint8_t*)6));
	korisnik_2[3] = (eeprom_read_byte((uint8_t*)7));

	korisnik_3[0] = (eeprom_read_byte((uint8_t*)8));
	korisnik_3[1] = (eeprom_read_byte((uint8_t*)9));
	korisnik_3[2] = (eeprom_read_byte((uint8_t*)10));
	korisnik_3[3] = (eeprom_read_byte((uint8_t*)11));

	korisnik_4[0] = (eeprom_read_byte((uint8_t*)12));
	korisnik_4[1] = (eeprom_read_byte((uint8_t*)13));
	korisnik_4[2] = (eeprom_read_byte((uint8_t*)14));
	korisnik_4[3] = (eeprom_read_byte((uint8_t*)15));*/