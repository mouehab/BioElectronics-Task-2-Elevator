#include"compiler_defs.h"
#include"C8051F020_defs.h"


//Data pins connected to port 2 on microcontroller
#define display_port P2      

// LCD Commands
#define clear 0x01
#define second_line 0xc1
#define cursor_first_line 0x80
#define cursor_second_line 0xc0

//Door Status
#define closed "DOOR IS CLOSED"   	
#define closing "DOOR IS CLOSING"	
#define open "DOOR IS OPEN"    		
#define down "GOING DOWN.."    		
#define up "GOING UP.." 
#define wait "PLEASE WAIT..."	
#define warning "WARNING..."
#define overload "OVERLOAD!!"	

//Motor Direction
#define right 1
#define left 0

//Push Buttons (Floors)
sbit ground      = P0^0;
sbit floor1_up   = P0^1;
sbit floor1_down = P0^2;
sbit floor2_up   = P0^3;
sbit floor2_down = P0^4;
sbit floor3_up   = P0^5;
sbit floor3_down = P0^6;
sbit floor4 	 = P0^7;


sbit rs = P3^5;  //RS pin connected to pin 5 of port 3
sbit rw = P3^6;  // RW pin connected to pin 6 of port 3
sbit e =  P3^7;  //E pin connected to pin 7 of port 3



// counter for displaying
int cnt=0;


// counter for counting people inside the elevator
int person_cnt=0;

//Request Flags
int request_down_flag=0;
int request_up_flag=0;

//Functions Prototypes
void go_down(void);
void go_up(void);

// Function for creating delay in milliseconds
void msdelay(unsigned int time)
{
    unsigned i,j ;
    for(i=0;i<time;i++)    
    for(j=0;j<1275;j++);
}

//Function to send command instruction to LCD
void lcd_cmd(unsigned char command)  
{
    display_port = command;
    rs= 0;
    rw=0;
    e=1;
    msdelay(1);
    e=0;
}

//Function to send display data to LCD
void lcd_data(unsigned char disp_data)  
{
    display_port = disp_data;
    rs= 1;
    rw=0;
    e=1;
    msdelay(1);
    e=0;
}
//displaying the door status
void lcd_display(unsigned char status[])
{

  while(status[cnt] != '\0') // searching the null terminator in the sentence
    {
        lcd_data(status[cnt]);
        cnt++;
        msdelay(5);
    }
	cnt=0;

}

// displaying the number on the seven segment
void seven_seg_display_BCD(int num)
{
		/*  Bit Manipulation */
//Port &= ~(1 << x) ; /* clears bit x */
//Port |= (1 << x) ; /* sets bit x */
switch(num)
{
case 0:
		P1 &= ~(1 << 0) & ~(1 << 1) & ~(1 << 2) & ~(1 << 3);
		break;
case 1:
		P1 |= (1 << 0) ;
		P1 &= ~(1 << 1) & ~(1 << 3) & ~(1 << 2) ;
		break;
case 2:
		P1 &= ~(1 << 0) & ~(1 << 2) & ~(1 << 3) ;
		P1 |= (1 << 1) ;
		break;
case 3:
		P1 |= (1 << 0) | (1 << 1);
		P1 &= ~(1 << 2) & ~(1 << 3);
		break;
case 4:
		P1 &= ~(1 << 0) & ~(1 << 1) & ~(1 << 3) ;
		P1 |= (1 << 2) ;
		break;
case 5:
		P1 |= (1 << 0) | (1 << 2);
		P1 &= ~(1 << 1) & ~(1 << 3) ;
		break;
case 6:
		P1 &= ~(1 << 0) & ~(1 << 3) ;
		P1 |= (1 << 2) | (1 << 1) ;
		break;
case 7:
		P1 |= (1 << 0) | (1 << 2) | (1 << 1) ;
		P1 &= ~(1 << 3) ;
		break;
case 8:
		P1 &= ~(1 << 0) & ~(1 << 1) & ~(1 << 2) ;
		P1 |= (1 << 3) ;
		break;
case 9:
		P1 |= (1 << 0) | (1 << 3) ;
		P1 &= ~(1 << 1) & ~(1 << 2) ;
		break;
}


}
//Motor Function
void motor_spin(unsigned char direction)
{
		if(direction == right)
		{
		P1 |= (1 << 4) | (1 << 5) ;
		P1 &= ~(1 << 6) & ~(1 << 7) ;         //0011     
        msdelay(20);
        P1 |= (1 << 5) | (1 << 6) ;
		P1 &= ~(1 << 7) & ~(1 << 4) ;         //0110
        msdelay(20);
        P1 |= (1 << 7) | (1 << 6) ;
		P1 &= ~(1 << 4) & ~(1 << 5) ;         //1100
        msdelay(20);
        P1 |= (1 << 7) | (1 << 4) ;
		P1 &= ~(1 << 5) & ~(1 << 6) ;         //1001
        msdelay(20);
		}
		else if(direction == left)
		{
		P1 |= (1 << 7) | (1 << 4) ;
		P1 &= ~(1 << 5) & ~(1 << 6) ;         //1001
        msdelay(20);
		P1 |= (1 << 7) | (1 << 6) ;
		P1 &= ~(1 << 4) & ~(1 << 5) ;         //1100
        msdelay(20);
        P1 |= (1 << 5) | (1 << 6) ;
		P1 &= ~(1 << 7) & ~(1 << 4) ;         //0110
        msdelay(20);
        P1 |= (1 << 4) | (1 << 5) ;
		P1 &= ~(1 << 6) & ~(1 << 7) ;         //0011     
        msdelay(20);
        
		}
		
}
//Function to prepare the LCD  and get it ready
 void lcd_init(void)    
{
    lcd_cmd(0x38);  // for using 2 lines and 5X7 matrix of LCD
    msdelay(10);
    lcd_cmd(0x0F);  // turn display ON, cursor blinking
    msdelay(10);
    lcd_cmd(0x01);  //clear screen
    msdelay(10);
    lcd_cmd(0x81);  // bring cursor to position 1 of line 1
    msdelay(10);
}

//Function to go to the Top floor
void go_up(void)
{
/* Closing The Door */
	lcd_cmd(clear);
	lcd_display(closing);
	lcd_cmd(second_line);
	lcd_display(wait);
	msdelay(500);
/* Polling For Down Requests */
	if(ground==0 || floor1_down==0 || floor2_down==0 || floor3_down==0)
	{
	request_down_flag=1;	
	}
/* Going Up */
	lcd_cmd(clear);
	lcd_display(closed);
	lcd_cmd(clear);
	lcd_display(up);
/* First Floor */
	motor_spin(right);
	seven_seg_display_BCD(1);
	msdelay(100);
/* Polling For Down Requests */
	if(ground==0 || floor1_down==0 || floor2_down==0 || floor3_down==0)
	{
	request_down_flag=1;	
	}
/* Second Floor */
	motor_spin(right);
	seven_seg_display_BCD(2);
	msdelay(100);
	/* Polling For Down Requests */
	if(ground==0 || floor1_down==0 || floor2_down==0 || floor3_down==0)
	{
	request_down_flag=1;	
	}
/* Third Floor */
	motor_spin(right);
	seven_seg_display_BCD(3);
	msdelay(100);
/* Polling For Down Requests */
	if(ground==0 || floor1_down==0 || floor2_down==0 || floor3_down==0)
	{
	request_down_flag=1;	
	}
/* Last Floor */
	motor_spin(right);
	seven_seg_display_BCD(4);
	lcd_cmd(clear);
	lcd_display(open);
	msdelay(100);
/* Checking For Saved Requests*/
	if(request_down_flag==1)
	{
	go_down();
	request_down_flag=0;
	}

}


//Function to go to the Ground
void go_down(void)
{
/* Closing The Door */
 	lcd_cmd(clear);
	lcd_display(closing);
	lcd_cmd(second_line);
	lcd_display(wait);
	msdelay(500);
/* Polling For Up Requests */
	if(floor4==0 || floor1_up==0 || floor2_up==0 || floor3_up==0)
	{
	request_up_flag=1;	
	}

/* Going Down */
	lcd_cmd(clear);
	lcd_display(closed);
	lcd_cmd(clear);
	lcd_display(down);
	motor_spin(left);
	seven_seg_display_BCD(3);
	msdelay(100);
/* Polling For Up Requests */
	if(floor4==0 || floor1_up==0 || floor2_up==0 || floor3_up==0)
	{
	request_up_flag=1;	
	}
/* Second Floor */
	motor_spin(left);
	seven_seg_display_BCD(2);
	msdelay(100);
	/* Polling For Up Requests */
	if(floor4==0 || floor1_up==0 || floor2_up==0 || floor3_up==0)
	{
	request_up_flag=1;	
	}
/* First Floor */
	motor_spin(left);
	seven_seg_display_BCD(1);
	msdelay(100);
/* Polling For Up Requests */
	if(floor4==0 || floor1_up==0 || floor2_up==0 || floor3_up==0)
	{
	request_up_flag=1;	
	}
/* Ground Floor */
	motor_spin(left);
	seven_seg_display_BCD(0);
	lcd_cmd(clear);
	lcd_display(open);
	msdelay(100);
/* Checking For Saved Requests*/
	if(request_up_flag==1)
	{
	go_up();
	request_up_flag=0;
	}
}

																	/* Application */
/*============================================================================================================================================================*/
void main()
{
	// initializing external interrupt 1 & 2
	IE = 0x85;
    // initializing port 0 as input
	P0= 0xff;	
	//initializing elevator status
    lcd_init();
	lcd_display(open);
	msdelay(50);

	while(1)
	{
	if(floor4==0 || floor1_up==0 || floor2_up==0 || floor3_up==0)
	{
	go_up();
	}
	else if(ground==0 || floor1_down==0 || floor2_down==0 || floor3_down==0)
	{
	go_down();
	}
	else
	{
	lcd_cmd(clear);
	lcd_display(open);
	msdelay(200);
	}
	}
}
/*============================================================================================================================================================*/
//Handler for Opening and Blocking The Door							/* Interrupt Handling*/
void ISR_open(void) interrupt 0
{
    lcd_cmd(clear);
	lcd_display(open);
	msdelay(500);
	lcd_cmd(clear);
	lcd_display(closing);
	lcd_cmd(second_line);
	lcd_display(wait);
}
//Handler for the overload in the elevator
void ISR_Room(void) interrupt 2
{
if(person_cnt==0 || person_cnt==1 || person_cnt==2 || person_cnt==3 )
{
	person_cnt++;
	msdelay(50);

}
else
{
lcd_cmd(clear);
lcd_cmd(cursor_first_line);
lcd_display(warning);
lcd_cmd(second_line);
lcd_cmd(cursor_second_line);
lcd_display(overload);
msdelay(50);
}

}