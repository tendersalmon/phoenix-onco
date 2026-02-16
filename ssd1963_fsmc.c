#include "ssd1963_fsmc.h"

//ф-ция для отправки команд
static __inline void 	TFT_Send_Cmd(uint8_t index)
{
	*(uint8_t *) (LCD_REG) = index;	
}

//ф-ция для отправки данных
static __inline void TFT_Write_Data(uint16_t data)
{   
	*(uint16_t *) (LCD_DATA) = data;
}

//ф-ция инициализации дисплея
void Init_SSD1963(void)
{
	//Reset дисплея
	HAL_GPIO_WritePin(DP_RESET_GPIO_Port, DP_RESET_Pin, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(DP_RESET_GPIO_Port, DP_RESET_Pin, GPIO_PIN_SET);

	
	TFT_Send_Cmd(0x00E2);	//PLL multiplier, set PLL clock to 120M
	TFT_Write_Data(0x0023);	    //N=0x36 for 6.5M, 0x23 for 10M crystal
	TFT_Write_Data(0x0002);
	TFT_Write_Data(0x0054);
	TFT_Send_Cmd(0x00E0);  // PLL enable
	TFT_Write_Data(0x0001);
	HAL_Delay(10);
	TFT_Send_Cmd(0x00E0);
	TFT_Write_Data(0x0003);		// now, use PLL output as system clock
	HAL_Delay(10);
	TFT_Send_Cmd(0x0001);  // software reset
	HAL_Delay(20);
	TFT_Send_Cmd(0x00E6);	//PLL setting for PCLK, depends on resolution
	TFT_Write_Data(0x0003);
	TFT_Write_Data(0x0033);
	TFT_Write_Data(0x0033);

	TFT_Send_Cmd(0x00B0);	//LCD SPECIFICATION
	TFT_Write_Data(0x0020); //24 bit TFT panel
	TFT_Write_Data(0x0000); //Hsync+Vsync +DE mode  TFT mode
	TFT_Write_Data((HDP>>8));  //Set HDP
	TFT_Write_Data(HDP);
    TFT_Write_Data(VDP>>8);  //Set VDP
	TFT_Write_Data(VDP);
    TFT_Write_Data(0x0000);

	TFT_Send_Cmd(0x00B4);	//HSYNC
	TFT_Write_Data(0x04);  //Set HT
	TFT_Write_Data(0x1f);
	TFT_Write_Data(0x00);  //Set HPS
	TFT_Write_Data(0xd2);
	TFT_Write_Data(0x00);			   //Set HPW
	TFT_Write_Data(0x00);  //Set HPS
	TFT_Write_Data(0x00);
	TFT_Write_Data(0x00);

	TFT_Send_Cmd(0x00B6);	//VSYNC
	TFT_Write_Data(0x02);   //Set VT
	TFT_Write_Data(0x0c);
	TFT_Write_Data(0x00);  //Set VPS
	TFT_Write_Data(0x22);
	TFT_Write_Data(0x00);		//Set VPW
	TFT_Write_Data(0x00);  //Set FPS
	TFT_Write_Data(0x00);


	TFT_Send_Cmd(0x00B8);
	TFT_Write_Data(0x000f);    //GPIO is controlled by host GPIO[3:0]=output   GPIO[0]=1  LCD ON  GPIO[0]=1  LCD OFF 
	TFT_Write_Data(0x0001);    //GPIO0 normal

	TFT_Send_Cmd(0x00BA);
	TFT_Write_Data(0x0001);    //GPIO[0] out 1 --- LCD display on/off control PIN

	TFT_Send_Cmd(0x0036); //rotation
	TFT_Write_Data(0x0008);

	TFT_Send_Cmd(0x003A); //Set the current pixel format for RGB image data
	TFT_Write_Data(0x0050);//16-bit/pixel

	TFT_Send_Cmd(0x00F0); //Pixel Data Interface Format
	TFT_Write_Data(0x0003);//16-bit(565 format) data 

	TFT_Send_Cmd(0x00BC); 
	TFT_Write_Data(0x0040);//contrast value
	TFT_Write_Data(0x0080);//brightness value
	TFT_Write_Data(0x0040);//saturation value
	TFT_Write_Data(0x0001);//Post Processor Enable

	HAL_Delay(5);

	TFT_Send_Cmd(0x0029); //display on


	TFT_Send_Cmd(0x00BE); //set PWM for B/L
	TFT_Write_Data(0x0006);
	TFT_Write_Data(0x0080);
	TFT_Write_Data(0x0001);
	TFT_Write_Data(0x00f0);
	TFT_Write_Data(0x0000);
	TFT_Write_Data(0x0000);

	TFT_Send_Cmd(0x00d0); 
	TFT_Write_Data(0x000d);

	HAL_GPIO_WritePin(DIP_EN_GPIO_Port, DIP_EN_Pin, GPIO_PIN_SET);
}

//ф-ция устанавливает рабочую область по X
void TFT_Set_X(uint16_t start_x,uint16_t end_x)
{
	TFT_Send_Cmd(0x002A);
	TFT_Write_Data(start_x>>8);
	TFT_Write_Data(start_x&0x00ff);
	
	TFT_Write_Data(end_x>>8);
	TFT_Write_Data(end_x&0x00ff);
}

//ф-ция устанавливает рабочую область по Y
void TFT_Set_Y(uint16_t start_y,uint16_t end_y)
{
	TFT_Send_Cmd(0x002B);
	TFT_Write_Data(start_y>>8);
	TFT_Write_Data(start_y&0x00ff);
	
	TFT_Write_Data(end_y>>8);
	TFT_Write_Data(end_y&0x00ff);
}

//ф-ция задает координаты точки на дисплее
void TFT_Set_XY(uint16_t x, uint16_t y)
{
	TFT_Set_X(x, x);
	TFT_Set_Y(y, y);
}


//ф-ция  задает координаты рабочей области
void TFT_Set_Work_Area(uint16_t x, uint16_t y, uint16_t length, uint16_t width)
{
	TFT_Set_X(x, x+length-1);
	TFT_Set_Y(y, y+width-1);
	TFT_Send_Cmd(0x2C);
}

//ф-ция закрашивает дисплей выбранным цветом
void TFT_Clear_Screen(uint16_t color)
{
	uint32_t i=0;
	TFT_Set_Work_Area(0,0,480,272);
	for(i=0; i < 130560; i++)
	{
		TFT_Write_Data(color);	//передаём кодировку цвета
	}
}


//ф-ция рисует символ нужного размера, цвета, на выбранном фоне, из указанной таблицы(это на случай если хочется использовать разные шрифты но размером 8х8)
void TFT_Draw_Char(uint16_t x, uint16_t y, uint16_t color, uint16_t phone,const uint8_t *table, uint8_t ascii, uint8_t size)
{
	uint8_t i,f = 0;
	
	
	for (i = 0; i < 11; i++)
	{
		for(f = 0; f < 8; f++)
		{
			if((*(table + 12*(ascii-0x20)+i)>>(7-f))&0x01)
			{
				 TFT_Draw_Fill_Rectangle(x+f*size, y+i*size, size, size, color);
			}
			else
			{	
				 TFT_Draw_Fill_Rectangle(x+f*size, y+i*size, size, size, phone);
			}
		}
	}
}

//ф-ция рисует строку, символами из указанной таблицы
void TFT_Draw_String(uint16_t x, uint16_t y, uint16_t color, uint16_t phone,
                     const uint8_t *table, char *string, uint8_t size)
{
    // Преобразуем char* в uint8_t* для работы с кириллицей
    uint8_t *str = (uint8_t *)string;

    while(*str)
    {
        // Проверяем не вылезем ли мы за пределы экрана
        if((x + 8) > MAX_X)
        {
            x = 1;
            y = y + 8 * size;
        }

        // Преобразуем char в uint8_t для корректной работы с кириллицей
        uint8_t char_code = (uint8_t)(*str);

        // Отладочный вывод (если есть UART)
        // printf("Char: %c (0x%02X)\n", *str, char_code);

        TFT_Draw_Char(x, y, color, phone, table, char_code, size);

        x += 8 * size;
        str++;  // Переходим к следующему символу
    }
}



void TFT_Draw_Cyrillic_String(uint16_t x, uint16_t y, uint16_t color, uint16_t bg_color,
                              const uint8_t *font_table, const char *text, uint8_t size)
{
    uint8_t *ptr = (uint8_t *)text;

    while(*ptr)
    {
        if((x + 8 * size) > MAX_X)
        {
            x = 1;
            y += 11 * size;
        }

        // Принудительно приводим к uint8_t
        uint8_t char_code = (uint8_t)(*ptr);

        // Отображаем символ
        TFT_Draw_Char(x, y, color, bg_color, font_table, char_code, size);

        x += 8 * size;
        ptr++;
    }
}



void TFT_fill_screen(uint16_t x, uint16_t y,uint16_t color){
	for(int i=0;i<x;i++){
		for(int j=0;j<y;j++){
			TFT_Draw_Char(i, j, color, WHITE, (uint8_t*)font_courier, 0x7F, 3);
		}
	}
}


//ф-ция рисует линию заданного цвета и размера
void TFT_Draw_Line (uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2, uint8_t size,uint16_t color)
{
	int deltaX = abs(x2 - x1);
	int deltaY = abs(y2 - y1);
	int signX = x1 < x2 ? 1 : -1;
	int signY = y1 < y2 ? 1 : -1;
	int error = deltaX - deltaY;
	int error2 = 0;
	
	for (;;)
	{
		TFT_Draw_Fill_Rectangle(x1,y1,size,size,color);
		
		if(x1 == x2 && y1 == y2)
		break;
		
		error2 = error * 2;
		
		if(error2 > -deltaY)
		{
			error -= deltaY;
			x1 += signX;
		}
		
		if(error2 < deltaX)
		{
			error += deltaX;
			y1 += signY;
		}
	}
}

//ф-ция рисует горизонтальную линию, указанной длины, толщины и цвета
void TFT_Draw_HLine(uint16_t x, uint16_t y, uint16_t length, uint16_t size, uint16_t color)
{
	uint16_t i=0;
	
	TFT_Set_Work_Area(x,y,length,size);
	for(i=0; i<(length*size); i++)
	TFT_Write_Data(color);
}

//ф-ция рисует вертикальную линию, указанной длины, толщины и цвета
void TFT_Draw_VLine(uint16_t x, uint16_t y, uint16_t length, uint16_t size, uint16_t color)
{
	uint16_t i=0;
	
	TFT_Set_Work_Area(x,y,size,length);
	for(i=0; i<(length*size); i++)
	TFT_Write_Data(color);
}

//ф-ция рисует прямоугольник, указанной длины, ширины, толщины линий и цвета
void TFT_Draw_Rectangle(uint16_t x, uint16_t y,
                        uint16_t length, uint16_t width,
                        uint8_t size, uint16_t color)
{
    TFT_Draw_HLine(x, y,                  length, size, color);         // верх
    TFT_Draw_HLine(x, y + width - size,   length, size, color);         // низ
    TFT_Draw_VLine(x, y,                  width,  size, color);         // лево
    TFT_Draw_VLine(x + length - size, y,  width,  size, color);         // право
}
//ф-ция рисует закрашенный прямоугольник, указанной длины, ширины, цвета
void TFT_Draw_Fill_Rectangle(uint16_t x, uint16_t y, uint16_t length, uint16_t width, uint16_t color)
{
	uint32_t i=0;
	
	TFT_Set_Work_Area(x,y,length, width);
	for(i=0; i < length*width; i++)
	{
		TFT_Write_Data(color);	//передаём кодировку цвета
	}
}

//ф-ция рисует треугольник по точкам с указанной толщиной линий и выбранным цветом
void TFT_Draw_Triangle( uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint8_t size, uint16_t color)
{
	
	TFT_Draw_Line( x1, y1, x2, y2, size, color);
	TFT_Draw_Line( x2, y2, x3, y3, size, color);
	TFT_Draw_Line( x3, y3, x1, y1, size, color);
}

//ф-ция рисует окружность нужного радиуса, линией задданой толщины и выбранным цветом, также возможно 
//залить окружность нужным цветом для этого установить аргумент fill равным единице, иначе ноль
void TFT_Draw_Circle(uint16_t x, uint16_t y, uint8_t radius, uint8_t fill, uint8_t size, uint16_t color)
{
	int a_,b_,P;
	a_ = 0;
	b_ = radius;
	P = 1 - radius;
	while (a_ <= b_)
	{
		if(fill == 1)
		{
			TFT_Draw_Fill_Rectangle(x-a_,y-b_,2*a_+1,2*b_+1,color);
			TFT_Draw_Fill_Rectangle(x-b_,y-a_,2*b_+1,2*a_+1,color);
		}
		else
		{
			TFT_Draw_Fill_Rectangle(a_+x, b_+y, size, size, color);
			TFT_Draw_Fill_Rectangle(b_+x, a_+y, size, size, color);
			TFT_Draw_Fill_Rectangle(x-a_, b_+y, size, size, color);
			TFT_Draw_Fill_Rectangle(x-b_, a_+y, size, size, color);
			TFT_Draw_Fill_Rectangle(b_+x, y-a_, size, size, color);
			TFT_Draw_Fill_Rectangle(a_+x, y-b_, size, size, color);
			TFT_Draw_Fill_Rectangle(x-a_, y-b_, size, size, color);
			TFT_Draw_Fill_Rectangle(x-b_, y-a_, size, size, color);
		}
		if (P < 0 )
		{
			P = (P + 3) + (2* a_);
			a_ ++;
		}
		else
		{
			P = (P + 5) + (2* (a_ - b_));
			a_ ++;
			b_ --;
		}
	}
}

//вспомогательная ф-ция для закругления краёв прямоугольника
void TFT_Draw_Circle_Helper( int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint8_t size, uint16_t color)
{
  int16_t f     = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x     = 0;
  int16_t y     = r;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;
    if (cornername & 0x4) {
      TFT_Draw_Fill_Rectangle(x0 + x, y0 + y, size, size, color);
      TFT_Draw_Fill_Rectangle(x0 + y, y0 + x, size, size, color);
    }
    if (cornername & 0x2) {
      TFT_Draw_Fill_Rectangle(x0 + x, y0 - y, size, size, color);
      TFT_Draw_Fill_Rectangle(x0 + y, y0 - x, size, size, color);
    }
    if (cornername & 0x8) {
      TFT_Draw_Fill_Rectangle(x0 - y, y0 + x, size, size, color);
      TFT_Draw_Fill_Rectangle(x0 - x, y0 + y, size, size, color);
    }
    if (cornername & 0x1) {
      TFT_Draw_Fill_Rectangle(x0 - y, y0 - x, size, size, color);
      TFT_Draw_Fill_Rectangle(x0 - x, y0 - y, size, size, color);
    }
  }
}

//ф-ция рисует прямоугольник заданной длины, ширины, радиусом закругления краёв, толщины линий и выбранным цветом
void TFT_Draw_Round_Rect(uint16_t x, uint16_t y, uint16_t length, uint16_t width, uint16_t r, uint8_t size, uint16_t color)
{

  TFT_Draw_HLine(x+r  , y    , length-2*r, size, color); // Top
  TFT_Draw_HLine(x+r  , y+width-1, length-2*r, size, color); // Bottom
  TFT_Draw_VLine(x    , y+r  , width-2*r, size, color); // Left
  TFT_Draw_VLine(x+length-1, y+r  , width-2*r, size, color); // Right

  TFT_Draw_Circle_Helper(x+r    , y+r    , r, 		 			1, size, color);
  TFT_Draw_Circle_Helper(x+length-r-1, y+r    , r, 			2, size, color);
  TFT_Draw_Circle_Helper(x+length-r-1, y+width-r-1, r,  4, size, color);
  TFT_Draw_Circle_Helper(x+r    , y+width-r-1, r, 			8, size, color);
}

//вспомогательная ф-ция для закругления краёв закрашенного прямоугольника
void TFT_Draw_Fill_Circle_Helper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color) 
{

  int16_t f     = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x     = 0;
  int16_t y     = r;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;

    if (cornername & 0x1) {
      TFT_Draw_VLine(x0+x, y0-y, 2*y+1+delta, 1, color);
      TFT_Draw_VLine(x0+y, y0-x, 2*x+1+delta, 1, color);
    }
    if (cornername & 0x2) {
     TFT_Draw_VLine(x0-x, y0-y, 2*y+1+delta, 1, color);
     TFT_Draw_VLine(x0-y, y0-x, 2*x+1+delta, 1, color);
    }
  }
}

//ф-ция рисует закрашенный прямоугольник заданной длины, ширины, радиусом закругления краев и выбранным цветом
void TFT_Draw_Fill_Round_Rect(uint16_t x, uint16_t y, uint16_t length, uint16_t width, uint16_t r, uint16_t color)
{
  TFT_Draw_Fill_Rectangle(x+r, y, length-2*r, width, color);

  TFT_Draw_Fill_Circle_Helper(x+length-r-1, y+r, r, 1, width-2*r-1, color);
  TFT_Draw_Fill_Circle_Helper(x+r    , y+r, r, 2, width-2*r-1, color);
}

// Функция преобразования UTF-8 в CP1251 для русских букв
uint8_t utf8_to_cp1251(const char **utf8_str) {
    uint8_t *ptr = (uint8_t*)*utf8_str;
    uint8_t first = *ptr++;

    if (first < 0x80) {
        // ASCII символ
        *utf8_str = (char*)ptr;
        return first;
    }

    // UTF-8 кириллица: 2 байта
    if ((first & 0xE0) == 0xC0) {
        uint8_t second = *ptr++;

        // Преобразование UTF-8 → CP1251 для русских букв
        if (first == 0xD0) {
            if (second >= 0x90 && second <= 0xBF) {
                *utf8_str = (char*)ptr;
                return 0xC0 + (second - 0x90); // А-п
            }
        } else if (first == 0xD1) {
            if (second >= 0x80 && second <= 0x8F) {
                *utf8_str = (char*)ptr;
                return 0xD0 + (second - 0x80); // р-я
            } else if (second == 0x91) { // ё
                *utf8_str = (char*)ptr;
                return 0xB8; // Ё в CP1251
            }
        }
    }

    // Если не распознали, возвращаем пробел
    *utf8_str = (char*)ptr;
    return 0x20;
}

// Новая функция для строк в UTF-8
void TFT_Draw_String_UTF8(uint16_t x, uint16_t y,
                          uint16_t color, uint16_t bg_color,
                          const uint8_t *font_table,
                          const char *utf8_text,
                          uint8_t size)
{
    const char *ptr = utf8_text;

    while (*ptr)
    {
        if ((x + 8 * size) > MAX_X) {
            x = 1;
            y += 11 * size;
        }

        uint8_t char_code = utf8_to_cp1251(&ptr);

        // ПРОЗРАЧНЫЙ ПРОБЕЛ:
        if (char_code == 0x20) {
            x += 8 * size;
            continue;
        }

        TFT_Draw_Char(x, y, color, bg_color, font_table, char_code, size);
        x += 8 * size;
    }
}
