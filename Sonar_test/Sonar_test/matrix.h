/*
 * VirtualMatrixToDotMatrix.c
 *
 * Created: 12/28/2017 6:56:07 PM
 *  Author: FARBIN
 */ 


#include <avr/io.h> //header to enable data flow control over pins
#define F_CPU 1000000UL //telling controller crystal frequency attached
#include <util/delay.h> //header to enable delay function in program

#include <string.h>
#include <stdlib.h>


#define BOARD_SIZE 8
#define BOARD_INFINITY 8
#define VERY_FAR 20

#define DRAW_MODE 1
#define ERASE_MODE 0

int mode = DRAW_MODE;


/*int BOARD[8][8] = {
	{0,1,1,1,0,0,0,0},
	{0,1,1,1,1,1,1,1},
	{0,0,0,1,0,0,0,0},
	{0,0,0,1,0,0,0,0},
	{0,0,0,1,0,0,0,0},
	{1,0,0,1,0,1,1,1},
	{0,0,0,1,0,0,0,0},
	{0,0,0,1,0,0,0,0}						
};
*/

int BOARD[BOARD_SIZE][BOARD_SIZE];
int rotated_board[BOARD_SIZE][BOARD_SIZE];

static int column_selector[8];
char row_selector[8] = {1,2,4,8,16,32,64,128}; //pin values of row


int cursorX=0, cursorY=0;
int offsetX=4, offsetY=4;

int prevPosX=0, prevPosY=0;


void rotate_board_90_anticlockwise (){
	for(int i=BOARD_SIZE-1; i>=0; i--){
		for(int j=0; j<BOARD_SIZE; j++){
			rotated_board[BOARD_SIZE-1-i][j] = BOARD[j][i];
		}
	}
}


void mirror_board(){
	for (int i=0; i<BOARD_SIZE; i++){
		for (int j=0; j<BOARD_SIZE; j++){
			int temp = rotated_board[i][j];
			rotated_board[i][j] = rotated_board[i][BOARD_SIZE-1-j];
			rotated_board[i][BOARD_SIZE-1-j] = temp;
		}
	}
}

void set_column_values_from_board (){
	for (int i=0; i<BOARD_SIZE; i++){
		column_selector[i] = 0;
		for (int j=0; j<BOARD_SIZE; j++){
			column_selector[i] |= (BOARD[i][j] << (BOARD_SIZE-1-j));			
		}
	}
}


void draw_board(){
	mirror_board();
	set_column_values_from_board();
	int k;
	for(k=0;k<500;k++){		
		for (int i=0; i<BOARD_SIZE; i++)
		{
			PORTA = row_selector[i];
			PORTC = ~column_selector[i];
			_delay_us(100);
		}
		PORTA = 0;
	}
}


int get_index_from_distance_y (int distance){
	
	int index = -1;
	distance=round(distance);
	
	if (distance >= 3 && distance < 4) index = 0;
	else if (distance >= 4 && distance < 5) index = 1;
	else if (distance >= 5 && distance < 6) index = 2;
	else if (distance >= 6 && distance < 7) index = 3;
	else if (distance >= 7 && distance < 8) index = 4;
	else if (distance >= 8 && distance < 9) index = 5;
	else if (distance >= 9 && distance < 10) index = 6;
	else if (distance >= 10 && distance < 11) index = 7;
	
	return index;
}
int get_index_from_distance_x (int distance){
	
	int index = -1;
	distance=round(distance);
	if(distance==3) distance=4;
	if (distance >= 4 && distance < 5) index = 0;
	else if (distance >= 5 && distance < 6) index = 1;
	else if (distance >= 6 && distance < 7) index = 2;
	else if (distance >= 7 && distance < 8) index = 3;
	else if (distance >= 8 && distance < 9) index = 4;
	else if (distance >= 9 && distance < 10) index = 5;
	else if (distance >= 10 && distance < 11) index = 6;
	else if (distance >= 11 && distance < 12) index = 7;
	
	return index;
}


void set_cursor_from_sensor_data (int posX, int posY){	
	
/*	if (posX > VERY_FAR || posY > VERY_FAR) return;
	
	
	if (posX < offsetX) posX = offsetX;
	else if (posX > offsetX+BOARD_SIZE-1) posX = offsetX+BOARD_SIZE-1;		
	
	cursorX = BOARD_SIZE - 1 - (int) (posX - offsetX);
	
	
	if (posY < offsetY) posY = offsetY;
	else if (posY > offsetY+BOARD_SIZE-1) posY = offsetY+BOARD_SIZE-1;
	
	cursorY = BOARD_SIZE - 1 - (int) (posY - offsetY);	
*/


	cursorX = get_index_from_distance_x(posX);
	cursorY = get_index_from_distance_y(posY);
	
	if (cursorY < 0 || cursorX < 0) return;	

/*	
//	posX = round(posX);
//	posY = round(posY);
	
	if (prevPosX == 0) {	//1st time
		prevPosX = posX;
		prevPosY = posY;
		BOARD[cursorY][cursorX] = 1;
		return;
	}
	
	if (posX > prevPosX && cursorX < BOARD_SIZE-1) {
		cursorX++;
		prevPosX = posX;
	}
	else if (posX < prevPosX && cursorX > 0) {
		cursorX--;
		prevPosX = posX;
	}

	
	if (posY > prevPosY && cursorY < BOARD_SIZE-1) {
		cursorY++;
		prevPosY = posY;	
	}
	else if (posY < prevPosY && cursorY > 0) {
		cursorY--;
		prevPosY = posY;
	}

	*/
	if (mode == DRAW_MODE){
		BOARD[cursorY][cursorX] = 1;		
	} else if (mode == ERASE_MODE){
		BOARD[cursorY][cursorX] = 0;
	}
	
}



void set_mode_of_painting (int _mode){
	mode = _mode;
}


void initialize_for_matrix_output(){
	
	DDRA = 0xFF;	//row
	DDRC = 0xFF;	//column
}



void reset_board(){
	for (int i=0; i<BOARD_SIZE; i++)
	{
		for (int j=0; j<BOARD_SIZE; j++)
		{
			BOARD[i][j] = 0;
		}
	}
	draw_board();
	cursorX=0; cursorY=0;
}