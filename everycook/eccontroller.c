#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include <wiringPi.h>
#include <softPwm.h>
//Pin defination for the SPI
#define MOSI	10
#define MISO	9
#define SCLK	11
#define CS	8
//Pin defination for the I2C
#define SDA	      2
#define SCL	      3
//AD774 Register defination
#define READ_STATUS_REG		0x40
#define READ_MODE_REG		0x48
#define WRITE_MODE_REG		0x08
#define READ_STRUCT_REG		0x50
#define WRITE_STRUCT_REG	0x10
#define READ_DATA_REG		0x58
#define READ_SERIAL_REG		0x60
#define READ_IO_REG 		0x68
#define WRITE_IO_REG		0x28
#define READ_SHIFT_REG		0x70
//I2C_DELAY_TIME
#define DELAY_TIME   10
//virtualSPI function delaration
void VirtualSPIInit(void);
void SPIReset(void);
void SPIWrite(uint8_t data);	
uint8_t SPIRead(void);
void SPIWriteByte(uint8_t reg, uint8_t data);
void SPIWrite2Bytes(uint8_t reg, uint32_t data);
uint8_t SPIReadByte(uint8_t register);
uint32_t SPIRead2Bytes(uint8_t register);
uint32_t SPIRead3Bytes(uint8_t register);
//virtualI2C function delaration
void VirtualI2CInit(void);
void I2CStart(void);
void I2CStop(void);
int CheckAck(void);
uint8_t I2CReadByte(void);
void I2CWriteByte(uint8_t data);
void I2CWriteBytes(uint8_t *data, uint8_t len);
//GPIO PCA9685 initialization
void GPIOInit(void);
void AD7794Init(void);
void PCA9685Init(void);
//Ohter functions
void WriteFile(void);
void ReadFIle(void);
void ReadConfigurationFile(void);
void NumberConvertToString(uint32_t num, char *str);
void StringClean(char *str, uint32_t len);
void StringUnion(char *fristString, char *secondString);
uint32_t StringConvertToNumber(char *str);
int POWNTimes(uint32_t num, uint8_t n);

uint8_t Data[10];
uint32_t adc0;
uint8_t structregpointer = 0;
char TotalUpdate[512];
uint32_t ConfigurationReg[] = {0x710, 0x711, 0x712, 0x713, 0x714, 0x115};//For AD7794 conf-reg

int main(void)
{	
	VirtualSPIInit();
	VirtualI2CInit();
	GPIOInit();
	PCA9685Init();
	AD7794Init();
	delay(30);
	StringClean(TotalUpdate, 512);
	ReadConfigurationFile();

	while (1)
	{
		WriteFile();
		ReadFIle();
		delay(1000);
	}
}
/*******************PI File read/write Code**********************/
/* Get the adc datas and signals and write to the file "/var/www/readfile.txt"
 */
void WriteFile(void)
{
	uint32_t temp;
	uint8_t i;
	char tempString[10];
	char writeString[32];
	FILE *fp;
//	uint32_t reg = 0x0110;	
	uint8_t PinNum[] = {24, 25, 27, 22, 7, 17};
	
	for (i = 0; i < 6; i++)
	{
		SPIWrite2Bytes(WRITE_STRUCT_REG, ConfigurationReg[i]);
		delay(50);
		temp = SPIRead3Bytes(READ_DATA_REG);	
		
		StringClean(writeString, 32);	
		StringClean(tempString ,10);
		StringUnion(writeString, "ADC");			//ADC(ADCX<TAB>NUM)
		NumberConvertToString(i+1, tempString);	
		StringUnion(writeString, tempString);			//X(ADCX<TAB>NUM)
		StringUnion(writeString, "	");			//TAB(ADCX<TAB>NUM)
		StringClean(tempString, 10);
		NumberConvertToString(temp, tempString);
		StringUnion(writeString, tempString);			//NUM(ADCX<TAB>NUM)
		StringUnion(writeString, "\n");				//\n(ADCX<TAB>NUM)
		StringUnion(TotalUpdate, writeString);
	}
	for (i = 0; i < 6; i ++)
	{
		temp = digitalRead(PinNum[i]);

		StringClean(writeString, 32);	
		StringClean(tempString ,10);
		StringUnion(writeString, "SIG");			//SIG(SIGX<TAB>NUM)
		NumberConvertToString(i+1, tempString);	
		StringUnion(writeString, tempString);			//X(SIGX<TAB>NUM)
		StringUnion(writeString, "	");			//TAB(SIGX<TAB>NUM)
		StringClean(tempString, 10);
		NumberConvertToString(temp, tempString);
		StringUnion(writeString, tempString);			//NUM(SIGX<TAB>NUM)
		StringUnion(writeString, "\n");				//\n(SIGX<TAB>NUM)
		StringUnion(TotalUpdate, writeString);
		
	}
	
	fp = fopen("/var/www/readfile.txt", "w");
	fputs(TotalUpdate, fp);
	StringClean(TotalUpdate, 512);
	fclose(fp);
}
/* Get the control datas from the file "/var/www/writefile.txt", and write to the i2c(pwm)
 * and buzzer and button.
 */
void ReadFIle(void)
{
	FILE *fp;
	char tempString[10];
	uint32_t control[22];
	uint8_t i = 0;
	uint8_t ptr = 0;
	char c;
	uint8_t PinNum[] = {14, 15, 18, 23};

	StringClean(tempString, 10);
	fp = fopen("/var/www/writefile.txt", "r");
	if (fp != NULL)
	{
		while ((c = fgetc(fp)) != 255)
		{
			if (c == 9) 
			{
				c = fgetc(fp);
				while (c != ';')
				{
					tempString[i] = c;
					c = fgetc(fp);
					i++;
				}
				i = 0;
				if (c == ';')
				{					
					control[ptr] = StringConvertToNumber(tempString);
					StringClean(tempString, 10);
					ptr++;
				}
			}
		}
	}
	fclose(fp);
	//The button should up after down (the KEYS's value should be changed back after used)
	system("sed -i '3s/^.*$/KEY1	1;/' /var/www/writefile.txt");
	system("sed -i '4s/^.*$/KEY2	1;/' /var/www/writefile.txt");
	system("sed -i '5s/^.*$/KEY3	1;/' /var/www/writefile.txt");
	system("sed -i '6s/^.*$/KEY4	1;/' /var/www/writefile.txt");
	//Buzzer 
	if (control[0]) softPwmWrite(4, 0);
	else {softPwmCreate(4, 0, control[1]);softPwmWrite(4, control[1]/10);}
	//Control buttons
	for (i = 0; i < 4; i++)
	{
		if (control[i+2])
			digitalWrite(PinNum[i], HIGH);
		else
		{
			digitalWrite(PinNum[i], LOW);
			delay(500);
			digitalWrite(PinNum[i], HIGH);	
		}
	}
	//I2C pwm setting
	for (i = 6; i < 22; i++)
	{
		Data[1] = 0x06+(i-6)*4;
		Data[2] = 0x00;
		Data[3] = 0x00;//control[i]&0xff;
		Data[4] = control[i]&0xff;
		Data[5] = (control[i]>>8)&0xff;
		I2CWriteBytes(Data, 6);		
		delay(3);
	}
}
/* Read the configuration of the amp of the reference voltage
 *
 */
void ReadConfigurationFile(void)
{
	FILE *fp;
	char tempString[10];
	uint8_t i = 0;
	uint8_t ptr = 0;
	char c;

	StringClean(tempString, 10);
	fp = fopen("config.txt", "r");

	if (fp != NULL)
	{
		while ((c = fgetc(fp)) != 255)
		{
			if (c == '=') 
			{
				c = fgetc(fp);
				while (c != ';')
				{
					tempString[i] = c;
					c = fgetc(fp);
					i++;
				}
				i = 0;
				if (c == ';')
				{					
					ConfigurationReg[ptr] = StringConvertToNumber(tempString);
					ConfigurationReg[ptr] = POWNTimes(ConfigurationReg[ptr], 2)<<9   				 |
								1<<8 				 |
								1<<4 				 |
								ptr;
//printf("%d\n", ConfigurationReg[ptr]);
					StringClean(tempString, 10);
					ptr++;
				}
			}
		}
	}
	fclose(fp);
}
/*
*/
int POWNTimes(uint32_t num, uint8_t n)
{
	int i = 0;

	while (num > 1)
	{
		num = num / n;
		i++;
	}
	return i;
}
/* Convert a number to a string
 *
 */
void NumberConvertToString(uint32_t num, char *str)
{
	uint8_t i = 0;
	uint32_t temp, mutiplecand = 1;

	temp = num;
	do
	{
		mutiplecand = mutiplecand*10;
		i++;
	}while (temp >= mutiplecand || i > 9);
	
	while (mutiplecand != 1)
	{
		mutiplecand = mutiplecand/10;
		*str++ = num/mutiplecand+48;
		num = num%mutiplecand;
	}
}
/* Clean the string
 *
 */
void StringClean(char *str, uint32_t len)
{
	uint32_t i = 0;

	for (; i< len; i++)
		str[i] = 0x00;
}
/* Combine two strings to one string
 *
 */
void StringUnion(char *fristString, char *secondString)
{
	uint8_t i = 0, fristEndPtr = 0;

	while (fristString[fristEndPtr]) fristEndPtr++;
	while (secondString[i])
	{
		fristString[fristEndPtr+i] = secondString[i];
		i++;
 	}
}
/* convert a string to a number
 *
 */
uint32_t StringConvertToNumber(char *str)
{
	uint32_t temp = 0 ,len = 0, mutiple = 1;

	while (str[len]) 
	{
		len++;
		mutiple *= 10;
	}
	len = 0;	
	while (str[len])
	{
		mutiple = mutiple/10;
		temp = temp + (str[len]-48)*mutiple;
		len++;
	}
	return temp;
}

/*******************PI Dirver Code**********************/
/* VirtualSPIInit
 */
void VirtualSPIInit(void)
{
	wiringPiSetupGpio();
	
	pinMode(MOSI, OUTPUT);
	pinMode(MISO, INPUT);
	pinMode(SCLK, OUTPUT);
	pinMode(CS, OUTPUT);	
	delay(30);
}
/* SPIReset: Reset the AD7794 chip, write 4 0xff.
 *
 */
void SPIReset(void)
{
	digitalWrite(CS, LOW);
	SPIWrite(0xff);
	SPIWrite(0xff);
	SPIWrite(0xff);
	SPIWrite(0xff);
	digitalWrite(CS, HIGH);	
}
/* SPIWrite: Write one byte data to the register in AD7794
 * data:datas write to register
 */
void SPIWrite(uint8_t data)
{
	int i = 7;
	
	for (i = 7; i >= 0; i--)
	{
		digitalWrite(SCLK, LOW);	
		if (data & (1<<i))
			digitalWrite(MOSI, HIGH);
		else
			digitalWrite(MOSI, LOW);
		delayMicroseconds(100);
		digitalWrite(SCLK, HIGH);
		delayMicroseconds(100);		
	}	
}
/* SPIRead: Read one byte data from the AD7794
 * return: data from register
 */
uint8_t SPIRead(void)
{
	int i = 7;
	uint8_t temp = 0;
	
	for (i = 7; i >= 0; i--)
	{
		digitalWrite(SCLK, LOW);
		delayMicroseconds(100);
		digitalWrite(SCLK, HIGH);
		delayMicroseconds(100);
		if (digitalRead(MISO))
			temp = temp | (1<<i);
	}

	return temp;
}
/* SPIWriteByte: Wirte one byte to denstination register. 
 * 
 */
void SPIWriteByte(uint8_t reg, uint8_t data)
{
	digitalWrite(CS, LOW);
	SPIWrite(reg);
	SPIWrite(data);
	digitalWrite(CS, HIGH);
}
/* SPIWrite2Bytes: Wirte one byte to denstination register. 
 * 
 */
void SPIWrite2Bytes(uint8_t reg, uint32_t data)
{
	digitalWrite(CS, LOW);
	SPIWrite(reg);
	SPIWrite((data>>8)&0xff);
	SPIWrite(data&0xff);
	digitalWrite(CS, HIGH);
}
/* SPIReadByte: Get one byte from denstination register. 
 * return: 1 byte data
 */
uint8_t SPIReadByte(uint8_t reg)
{
	uint8_t data;

	digitalWrite(CS, LOW);
	SPIWrite(reg);
	data = SPIRead();
	digitalWrite(CS, HIGH);

	return data;
}
/* SPIReadByte: Get two bytes from denstination register. 
 * return: 2 bytes data
 */
uint32_t SPIRead2Bytes(uint8_t reg)
{
	uint32_t data;

	digitalWrite(CS, LOW);
	SPIWrite(reg);
	data = SPIRead();
	data = (data << 8)| SPIRead();
	digitalWrite(CS, HIGH);
	
	return data;
}
/* SPIReadByte: Get three bytes from denstination register. 
 * return: 3 bytes data
 */
uint32_t SPIRead3Bytes(uint8_t reg)
{
	uint32_t data;

	digitalWrite(CS, LOW);
	SPIWrite(reg);
	data = SPIRead();
	data = (data << 8)| SPIRead();
	data = (data << 8)| SPIRead();
	digitalWrite(CS, HIGH);

	return data;
}
/* VirtualI2CInit: Initializate the virtual I2C-BUS protocal
 *
 */
void VirtualI2CInit(void)
{
	wiringPiSetupGpio();
	pinMode(SDA, OUTPUT);
	pinMode(SCL, OUTPUT);
}
/* I2CStart: Simulate the I2C start
 *
 */ 
void I2CStart(void)
{
	digitalWrite(SDA, HIGH);
	delayMicroseconds(DELAY_TIME);
	digitalWrite(SCL, HIGH);
	delayMicroseconds(DELAY_TIME);
	digitalWrite(SDA, LOW);
	delayMicroseconds(DELAY_TIME);
	digitalWrite(SCL, LOW);
	delayMicroseconds(DELAY_TIME);
}
/* I2CStop: Simulate the I2C stop
 *
 */
void I2CStop(void)
{
	digitalWrite(SDA, LOW);
	delayMicroseconds(DELAY_TIME);
	digitalWrite(SCL, HIGH);
	delayMicroseconds(DELAY_TIME);
	digitalWrite(SDA, HIGH);
	delayMicroseconds(DELAY_TIME);
	digitalWrite(SCL, LOW);
	delayMicroseconds(DELAY_TIME);
}
/* CheckAck: Checking Acknowleage in the SDA
 * return:1- Got the ACK, 0-Did got the ACK
 */
int CheckAck(void)
{
	int temp;
	
	digitalWrite(SDA, HIGH);
	delayMicroseconds(DELAY_TIME);
	digitalWrite(SCL, HIGH);
	delayMicroseconds(DELAY_TIME);
	pinMode(SDA, INPUT);
	delayMicroseconds(1);
	temp = digitalRead(SDA);
	pinMode(SDA, OUTPUT);
	digitalWrite(SCL, LOW);
	delayMicroseconds(DELAY_TIME);
	if (temp == 1)
		return 0;
	return 1;
}
/* I2CWriteByte: Write one Byte to the PCA9685
 * data:Data to the PCA9685
 */
void I2CWriteByte(uint8_t data)
{
	uint8_t i = 0;

	for (; i < 8; i++)
	{
		if ((data&0x80)==0x80)		
			digitalWrite(SDA, HIGH);
		else
			digitalWrite(SDA, LOW);
		data = data << 1;
		delayMicroseconds(DELAY_TIME);		
		digitalWrite(SCL, HIGH);
		delayMicroseconds(DELAY_TIME);
		digitalWrite(SCL, LOW);
		delayMicroseconds(DELAY_TIME);
	}
	if (!CheckAck())
		I2CStop();
}
/* I2CReadByte: Read one Byte from the PCA9685
 * return:Data from the PCA9685
 */
uint8_t I2CReadByte(void)
{
	uint8_t i = 0, temp = 0, data = 0;

	for (; i < 8; i++)
	{
		digitalWrite(SDA, HIGH);
		delayMicroseconds(DELAY_TIME);
		digitalWrite(SCL, HIGH);
		pinMode(SDA, INPUT);
		delayMicroseconds(1);
		temp = digitalRead(SDA);
		pinMode(SDA, OUTPUT);
		delayMicroseconds(DELAY_TIME);
		digitalWrite(SCL, LOW);
		if (temp == 1)
		{
			data = data << 1;
			data = data | 0x01;
		}
		else
			data = data << 1;
	}
	digitalWrite(SCL, LOW);
	return data;
}
/* I2CWriteBytes: Write one Byte to the PCA9685
 * data:The frist byte must be the address of the 'CHIP', then the data to the
 *      chip.
 */
void I2CWriteBytes(uint8_t *data, uint8_t len)
{
	uint8_t i = 0;

	I2CStart();
	for (; i < len; i++)
		I2CWriteByte(*data++);
	I2CStop();
	delay(3);
}
void GPIOInit(void)
{
	pinMode(24 ,INPUT);	//SIG1
	pinMode(25 ,INPUT);	//SIG2
	pinMode(27 ,INPUT);	//SIG3
	pinMode(22 ,INPUT);	//SIG4
	pinMode(7 ,INPUT);	//SIG5
	pinMode(17 ,INPUT);	//SIG6

	pinMode(14, OUTPUT);	//KEY1
	pinMode(15, OUTPUT);	//KEY2
	pinMode(18, OUTPUT);	//KEY3
	pinMode(23, OUTPUT);	//KEY4
	
}
void PCA9685Init(void)
{
	Data[0] = 0x80;
	Data[1] = 0x00;
	Data[2] = 0x31;
	I2CWriteBytes(Data, 3);
	Data[1] = 0xfe;
	Data[2] = 0x0d;
	I2CWriteBytes(Data, 3);
	Data[1] = 0x00;
	Data[2] = 0xa1;
	I2CWriteBytes(Data, 3);
	Data[1] = 0x01;
	Data[2] = 0x04;
	I2CWriteBytes(Data, 3);
}
void AD7794Init(void)
{
	SPIReset();	
	delay(30);
	SPIWrite2Bytes(WRITE_MODE_REG, 0x0002);
}
