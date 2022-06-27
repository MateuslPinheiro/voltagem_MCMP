#include <p18f4520.h>
#define RS LATCbits.LATC0
#define EN LATCbits.LATC1

unsigned int tensao1, tensao2;
unsigned int v1, v2, j;
unsigned long int aux, x;
unsigned char valor1[5]={'_','.','_','_','V'}, valor2[5]={'_','.','_','_','V'};

unsigned long char c, d, u; //centena, dezena, unidade
unsigned char dado;


void delay_ms (void)
{
	unsigned int i;
	for (i=0;i<300;i++) {}
}


void delay_s (void) 
{
	unsigned long int i;
	for (i=0;i<90000;i++) {}
}

//FUNÇÃO DRIVER
void envia_comando(unsigned char comando)
{
	RS=0;
	EN=0;
	LATD = comando;
	EN=1;
	EN=0;	//habilita LCD
	delay_ms();		// tempo pro LCD executar o comando
}

//FUNÇÃO DRIVER
void envia_dado(unsigned char dado)
{
	RS=1;	
	EN=0;
	LATD = dado;
	EN=1;
	EN=0;
	delay_ms();
}

void envia_string1(char valor[])
{
	for (j=0; j<5; j++)
	{
		dado = valor[j];

		RS=1;
		EN=0;
		LATD = dado;
		EN=1;
		EN=0;
		delay_ms();
		envia_comando(0x80 +(j+5));		//desloca para o próximo endereço do display
	}
}

void envia_string2(char valor[])
{
	for (j=0; j<5; j++)
	{
		dado = valor[j];

		RS=1;
		EN=0;
		LATD = dado;
		EN=1;
		EN=0;
		delay_ms();
		envia_comando(0xC0 +(j+5));		//desloca para o próximo endereço do display
	}
}



void inicializa_lcd(void)
{
	delay_s();
	envia_comando(0b00111000); // 2 linhas caractere 5x8
	envia_comando(0b00001100); // curson off(sétimo bit);
	envia_comando(0b00000001); 
}

void show_display (void)
{
	envia_comando(0x80);
	envia_dado('V');
	envia_dado('1');
	envia_dado('=');

	envia_comando(0x84);
	envia_string1(valor1); // Valor da tensão, varrendo e pondo no display


	envia_comando(0xC0);
	envia_dado('V');
	envia_dado('2');
	envia_dado('=');

	envia_comando(0xC4);
	envia_string2(valor2); // Valor da tensão, varrendo e pondo no display
}

int converte_tensao()
{
	//TENSÃO 01
	aux = (50*tensao1);
	x = (aux*10);
	v1 = (x/1024);    //v1=499 é o pior caso

	c = (v1/100);					// Separar pra pegar o valor da centena, dezena e unidade
	d = (v1%100);
	u = (d%10);
	d = (d/10);
	valor1[0] = c + 0x30;
	valor1[2] = d + 0x30;
	valor1[3] = u + 0x30;

	//TENSÃO 02
	aux = (50*tensao2);
	x = (aux*10);		// Não tá dando pra fazer a operação direta
	v2 = (x/1024);    //v1=250 é o pior caso

	c = (v2/100);     
	d = (v2%100);
	u = (d%10);
	d = (d/10);
	valor2[0] = c + 0x30;
	valor2[2] = d + 0x30;
	valor2[3] = u + 0x30;
}

//Habilita AN0
void conf_an0()
{
	ADCON0bits.CHS3 = 0;
	ADCON0bits.CHS2 = 0;
	ADCON0bits.CHS1 = 0;
	ADCON0bits.CHS0 = 0;
//	ADCON0bits.CHS=0b01; //select analog input, AN1 -- WILL CHANGE LATER TO AN3
	ADCON0bits.ADON = 1;
}

void conf_an1()
{
	ADCON0bits.CHS3 = 0;
	ADCON0bits.CHS2 = 0;
	ADCON0bits.CHS1 = 0;
	ADCON0bits.CHS0 = 1;
//	ADCON0bits.CHS=0b11; //select analog input, AN3 -- WILL CHANGE LATER TO AN1
	ADCON0bits.ADON = 1; //Habilita o conversor A/D
}

main ()
{
	//área de configuração do sistema
	TRISD=0b00000000;
	TRISC=0b00000000;
	TRISB=0b11111111;
	TRISCbits.RC0 = 0;
	TRISCbits.RC1 = 0;
	
//ADCON1
	ADCON1bits.PCFG3 = 1;
	ADCON1bits.PCFG2 = 1;
	ADCON1bits.PCFG1 = 0;
	ADCON1bits.PCFG0 = 1; //1101 -> AN0 e AN1 como analógicos

	ADCON1bits.VCFG1 = 0; //VREF- (GND)
	ADCON1bits.VCFG0 = 0; //VREF+ (SV)

//ADCON2
	ADCON2 = 0b10111100; //right justified, acquisition times are at 0 with 31KHz


	inicializa_lcd();

	// programa monitor
	while (1)
	{
		
		conf_an0();
		ADCON0bits.ADON = 1; //Habilita o conversor A/D
		ADCON0bits.GO = 1;
		while(ADCON0bits.GO == 1){}   //espera conversão
		tensao1 = (ADRESH*256) + (ADRESL *1); //The ADRESL and ADRESH registers are 2 registers used to store the end result of an analog-to-digital conversion (ADC).
		
		conf_an1();
		ADCON0bits.GO = 1;
		while(ADCON0bits.GO == 1){}
		tensao2 = (ADRESH*256) + (ADRESL *1); //The ADRESL and ADRESH registers are 2 registers used to store the end result of an analog-to-digital conversion (ADC).
		
		
		converte_tensao();
		show_display();
	}
}
