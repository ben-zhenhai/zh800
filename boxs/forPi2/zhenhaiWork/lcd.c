#include "lcd.h"

/* 20160829, Joey Ni, test for LCD bug ------------------ { */
#include <pthread.h>


/* WF35M module { */
#define SB1 0x01
#define SB2 0x02
#define SB3 0x04

#define MODE_STRING             0X31
#define MODE_GRAPHIC		    0X32
#define MODE_PIXEL			    0X33
#define MODE_LINE			    0X34
#define MODE_SQUARE			    0X35
#define MODE_CLEAR			    0X36
#define MODE_PWM			    0X37
#define MODE_SET_BL			    0X39
#define MODE_ERASE_ICON_LAYER	0X41
#define MODE_CAL_RTP		    0X7E

#define CMD_LEN_GRAPHIC	13
#define CMD_LEN_PIXEL	14
#define CMD_LEN_CLEAR	14
#define CMD_LEN_SQUARE	19
/* WF35M module } */

#define TEXT_LEN_MAX 30


typedef enum screen_background {
    BG_POWER_ON = 0,
    BG_WORK_INFO,
    BG_PRODUCT_COUNT,
    BG_POWER_OFF,
    BG_MAIN_MENU,
    BG_MACHINE_INFO,
} screen_bg_t;


pthread_mutex_t screen_update_mutex = PTHREAD_MUTEX_INITIALIZER;
static screen_bg_t current_screen_bg = BG_POWER_OFF;  // screen background
static int flag_wf35m_error = 0;
static int flag_timer = 0;
static int WF35M_reset_count = 0;


void alarm_handler(int a);
void WF35M_screen_update(int index);
int WF35M_bg_update(screen_bg_t background);
int WF35M_check_busy(void);
int WF35M_spi_text_cmd(char font_type, char display_level, char transparent, char rotation,
                      int X_point, int Y_point, unsigned long text_color, unsigned long bg_color,
                      char *text, int text_len);
int WF35M_spi_picture_cmd(char display_level, char rotation, 
                           int X_point, int Y_point, int picture_index);
int WF35M_spi_square_cmd(char display_level, char rotation, int X_start, int Y_start,
                          int X_end, int Y_end, int border_pixel, unsigned long color);
int WF35M_spi_clear_cmd(int X_start, int Y_start, int X_end, int Y_end);


void alarm_handler(int a)
{
    flag_timer = 1;
}


void WF35M_screen_update(int index)
{
    int cmd_result = 0;
    char text_buffer[TEXT_LEN_MAX];
    int fd2;
    int fd;
    struct ifreq ethreq;

    pthread_mutex_lock(&screen_update_mutex);

    if (flag_wf35m_error) {
        pthread_mutex_unlock(&screen_update_mutex);
        return;
    }

    switch (index) {
    case 1:  // product count
        if (WF35M_bg_update(BG_PRODUCT_COUNT) == -1) goto WF35M_ERROR;

        cmd_result = WF35M_spi_text_cmd(1, 3, 0, 0, 106, 88, 0x00FFFF, 0x000000, CountNo, strlen(CountNo));
        if (cmd_result != 0) goto WF35M_ERROR;

        sprintf(text_buffer, "%ld", GoodCount);
        cmd_result = WF35M_spi_text_cmd(1, 3, 0, 0, 106, 134, 0x00FFFF, 0x000000, text_buffer, strlen(text_buffer));
        if (cmd_result != 0) goto WF35M_ERROR;

        sprintf(text_buffer, "%ld", TotalBadCount);
        cmd_result = WF35M_spi_text_cmd(1, 3, 0, 0, 106, 184, 0x00FFFF, 0x000000, text_buffer, strlen(text_buffer));
        if (cmd_result != 0) goto WF35M_ERROR;

        break;
        /* ����:  �����s�����L�{���B�z��global�ܼ�, ���k���O�ܫ���, �i�H�Ҽ{�ק� */

    case 2:  // Power off
        if (WF35M_bg_update(BG_POWER_OFF) == -1) goto WF35M_ERROR;
        break;

    case 3:  // select item 1
        if (WF35M_bg_update(BG_MAIN_MENU) == -1) goto WF35M_ERROR;

        cmd_result = WF35M_spi_square_cmd(3, 0, 100, 59, 225, 99, 3, 0x0000FF);
        if (cmd_result != 0) goto WF35M_ERROR;

        break;

    case 4:  // select item 2
        if (WF35M_bg_update(BG_MAIN_MENU) == -1) goto WF35M_ERROR;

        cmd_result = WF35M_spi_square_cmd(3, 0, 99, 111, 224, 151, 3, 0x0000FF);
        if (cmd_result != 0) goto WF35M_ERROR;

        break;

    case 5:  // select item 3
        if (WF35M_bg_update(BG_MAIN_MENU) == -1) goto WF35M_ERROR;

        cmd_result = WF35M_spi_square_cmd(3, 0, 98, 161, 223, 201, 3, 0x0000FF);
        if (cmd_result != 0) goto WF35M_ERROR;

        break;

    case 6:  // machine info
        fd2 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (WF35M_bg_update(BG_MACHINE_INFO) == -1) goto WF35M_ERROR;

        strncpy(ethreq.ifr_name, ZHNETWORKTYPE, IFNAMSIZ);
        ioctl(fd2, SIOCGIFFLAGS, &ethreq);

        if(ethreq.ifr_flags & IFF_RUNNING) {
            fd = socket(AF_INET, SOCK_DGRAM, 0);
            struct ifreq ifr;

            ifr.ifr_addr.sa_family = AF_INET;
            strncpy(ifr.ifr_name, ZHNETWORKTYPE, IFNAMSIZ-1);
            ioctl(fd, SIOCGIFADDR, &ifr);
            close(fd);

            /* IP address */
            strncpy(text_buffer, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 17);
            cmd_result = WF35M_spi_text_cmd(1, 3, 0, 0, 95, 200, 0x00FFFF, 0x000000, text_buffer, strlen(text_buffer));
            if (cmd_result != 0) goto WF35M_ERROR;
        }

        close(fd2);

        /* machine number */
        cmd_result = WF35M_spi_text_cmd(1, 3, 0, 0, 130, 140, 0x00FFFF, 0x000000, MachineNo, strlen(MachineNo));
        if (cmd_result != 0) goto WF35M_ERROR;

        break;

    case 7:  // repair
        if (WF35M_bg_update(BG_POWER_OFF) == -1) goto WF35M_ERROR;

        strcpy(text_buffer, "Repairing...");
        cmd_result = WF35M_spi_text_cmd(0, 3, 0, 0, 10, 50, 0xFF0000, 0x000000, text_buffer, strlen(text_buffer));
        if (cmd_result != 0) goto WF35M_ERROR;

        sprintf(text_buffer, "ID: %s", RepairNo);
        cmd_result = WF35M_spi_text_cmd(0, 3, 0, 0, 10, 134, 0xFF0000, 0x000000, text_buffer, strlen(text_buffer));
        if (cmd_result != 0) goto WF35M_ERROR;

        break;

    case 99:  // Barcode Error
        switch (BarcodeIndex) {
        case 0:
            strcpy(text_buffer, "Error 1");
            break;
        case 1:
            strcpy(text_buffer, "Error 2");
            break;
        case 2:
            strcpy(text_buffer, "Error 3");
            break;
        case 3:
            strcpy(text_buffer, "Error 4");
            break;
        default:
            strcpy(text_buffer, "Error 5");
            break;
        }
        cmd_result = WF35M_spi_text_cmd(0, 3, 1, 0, 106, 184, 0xFFFF00, 0xFF0000, text_buffer, strlen(text_buffer));
        if (cmd_result != 0) goto WF35M_ERROR;

        break;

    case 111:  // show debug message on screen
        if (current_screen_bg == BG_MAIN_MENU || current_screen_bg == BG_WORK_INFO) {
            cmd_result = WF35M_spi_clear_cmd(290, 0, 320, 40);
            if (cmd_result != 0) goto WF35M_ERROR;
            usleep(500000);  // 500ms
            
            if (WF35M_reset_count == 0) {
                cmd_result = WF35M_spi_square_cmd(3, 0, 300, 8, 314, 22, 7, 0x008040);
                if (cmd_result != 0) goto WF35M_ERROR;
            }
            else if (WF35M_reset_count < 10) {
                sprintf(text_buffer, "%d", WF35M_reset_count);
                cmd_result = WF35M_spi_text_cmd(1, 3, 0, 0, 290, 000, 0xFF0000, 0x000000, text_buffer, strlen(text_buffer));
                if (cmd_result != 0) goto WF35M_ERROR;
            }
            else {
                strcpy(text_buffer, "X");
                cmd_result = WF35M_spi_text_cmd(1, 3, 0, 0, 290, 000, 0xFF0000, 0x000000, text_buffer, strlen(text_buffer));
                if (cmd_result != 0) goto WF35M_ERROR;
            }
        }

        break;

    default:  // work info
        if (WF35M_bg_update(BG_WORK_INFO) == -1) goto WF35M_ERROR;

        /* working number */
        if (strlen(ISNo) > 0) {
            cmd_result = WF35M_spi_text_cmd(1, 3, 0, 0, 97, 57, 0x00FFFF, 0x000000, ISNo, strlen(ISNo));
            if (cmd_result != 0) goto WF35M_ERROR;
        }

        /* item number */
        if (strlen(ManagerCard) > 0) {
            cmd_result = WF35M_spi_text_cmd(1, 3, 0, 0, 75, 93, 0x00FFFF, 0x000000, ManagerCard, 10);
            if (cmd_result != 0) goto WF35M_ERROR;
            cmd_result = WF35M_spi_text_cmd(1, 3, 0, 0, 75, 123, 0x00FFFF, 0x000000, ManagerCard+10, 14);
            if (cmd_result != 0) goto WF35M_ERROR;
        }

        /* goal amount */
        cmd_result = WF35M_spi_text_cmd(1, 3, 0, 0, 130, 163, 0x00FFFF, 0x000000, CountNo, strlen(CountNo));
        if (cmd_result != 0) goto WF35M_ERROR;

        /* employee ID */
        if (isInPairMode == 1) {
            if(CanChangeRepairModeFlag == 3)
                strcpy(text_buffer, "Repair Done");
            else
                strcpy(text_buffer, "Repairing");
            cmd_result = WF35M_spi_text_cmd(0, 3, 0, 0, 130, 205, 0xFFFF00, 0x000000, text_buffer, strlen(text_buffer));
            if (cmd_result != 0) goto WF35M_ERROR;
        }
        else {
            if (strlen(UserNo) <= 25)
                cmd_result = WF35M_spi_text_cmd(0, 3, 0, 0, 130, 205, 0xFFFF00, 0x000000, UserNo, strlen(UserNo));
            else
                cmd_result = WF35M_spi_text_cmd(0, 3, 0, 0, 130, 205, 0xFFFF00, 0x000000, UserNo+24, strlen(UserNo)-24);
            if (cmd_result != 0) goto WF35M_ERROR;
        }

        break;
    }

    pthread_mutex_unlock(&screen_update_mutex);

    return;

WF35M_ERROR:  // error handle
    flag_wf35m_error = 1;
    pthread_mutex_unlock(&screen_update_mutex);

    /* reset LCD module */
    digitalWrite(ZHPIN33, LOW);
    sleep(2);
    digitalWrite(ZHPIN33, HIGH);
    sleep(2);

    current_screen_bg = BG_POWER_ON;

    WF35M_reset_count++;
    printf("WF35M LCD module is reset, reset count = %d\n", WF35M_reset_count);

    WF35M_bg_update(BG_MAIN_MENU);
    // Reset up down key if we reset the screen.
    DisableUpDown = 0;
    WF35M_spi_square_cmd(3, 0, 100, 59, 225, 99, 3, 0x0000FF);

    flag_wf35m_error = 0;

    return;
}

	
int WF35M_bg_update(screen_bg_t background)
{
    int cmd_result = 0;

    if (background != current_screen_bg) {
        cmd_result = WF35M_spi_picture_cmd(1, 0, 0, 0, background);
        if (cmd_result != 0) return -1;
        current_screen_bg = background;
    }

    cmd_result = WF35M_spi_clear_cmd(0, 0, 320, 240);
    if (cmd_result != 0) return -1;

    return 0;
}


int WF35M_check_busy(void)
{
    unsigned char cmd_buffer[2] = {0x01, 0xFF};
    int count = 1;

    usleep(5000);  // before checking, wait 5ms

    while (1) {
        wiringPiSPIDataRW(SPI_CHANNEL, cmd_buffer, 2);
        delayMicroseconds(500);  // delay 0.5ms and check busy flag
        if (cmd_buffer[1] == 0x01) break;
        count++;
        if (count > 100) break;
        usleep(5000);  // if still busy, wait 5ms
    }

    if (count > 20)
        printf("Warning! count of check_busy = %d\n", count);

    if (count > 100) {
        printf("Error! LCD always busy! \n");
        return -1;
    }

    #if 0  // for test
    static int test_count = 0;
    test_count++;
    if (test_count%50 == 0) {
        printf("test_count=%d\n", test_count);
        return -1;
    }
    #endif

    return 0;
}


int WF35M_spi_text_cmd(char font_type, char display_level, char transparent, char rotation,
                      int X_point, int Y_point, unsigned long text_color, unsigned long bg_color,
                      char *text, int text_len)
{
    unsigned char cmd_buffer[18+TEXT_LEN_MAX];

    if (text_len > TEXT_LEN_MAX) {
        printf("text is too long, can't be sent!\n");
        return -2;
    }

    cmd_buffer[0] = 0x31;
    cmd_buffer[1] = SB3;
    cmd_buffer[2] = MODE_STRING;
    cmd_buffer[3] = ((font_type << 4) & 0xF0) + display_level;
    cmd_buffer[4] = ((transparent << 4) & 0xF0) + rotation;
    cmd_buffer[5] = (X_point >> 8);
    cmd_buffer[6] = X_point;
    cmd_buffer[7] = (Y_point >> 8);
    cmd_buffer[8] = Y_point;
    cmd_buffer[9] = (text_color >> 16);
    cmd_buffer[10] = (text_color >> 8);
    cmd_buffer[11] =  text_color;
    cmd_buffer[12] = (bg_color >> 16);
    cmd_buffer[13] = (bg_color >> 8);
    cmd_buffer[14] =  bg_color;

    memcpy((void *)(cmd_buffer+15), text, text_len);

    cmd_buffer[text_len + 15] = 0x0A;
    cmd_buffer[text_len + 16] = 0x00;
    cmd_buffer[text_len + 17] = 0x0D;
    
    wiringPiSPIDataRW(SPI_CHANNEL, cmd_buffer, text_len + 18);

    return WF35M_check_busy();
}


int WF35M_spi_picture_cmd(char display_level, char rotation, 
                           int X_point, int Y_point, int picture_index)
{
	unsigned char cmd_buffer[CMD_LEN_GRAPHIC];

	cmd_buffer[0] = 0x31;
	cmd_buffer[1] = SB3;
	cmd_buffer[2] = MODE_GRAPHIC;
	cmd_buffer[3] = ((display_level << 4) & 0xF0) + rotation;
	cmd_buffer[4] = (X_point >> 8);
	cmd_buffer[5] = X_point;
	cmd_buffer[6] = (Y_point >> 8);
	cmd_buffer[7] = Y_point;
	cmd_buffer[8]= (picture_index >> 8);
	cmd_buffer[9]= picture_index;
	cmd_buffer[10]= 0x0A;
	cmd_buffer[11]= 0x00;
	cmd_buffer[12]= 0x0D;

    wiringPiSPIDataRW(SPI_CHANNEL, cmd_buffer, CMD_LEN_GRAPHIC);

    usleep(500000);  // painting full size picture needs about 500 ms
    
    return WF35M_check_busy();
}


int WF35M_spi_square_cmd(char display_level, char rotation, int X_start, int Y_start,
                          int X_end, int Y_end, int border_pixel, unsigned long color)
{
	unsigned char cmd_buffer[CMD_LEN_SQUARE];

	cmd_buffer[0] = 0x31;
	cmd_buffer[1] = SB3;
	cmd_buffer[2] = MODE_SQUARE;
	cmd_buffer[3] = ((display_level << 4) & 0xF0) + rotation;
	cmd_buffer[4] = (X_start >> 8);
	cmd_buffer[5] = X_start;
	cmd_buffer[6] = (Y_start >> 8);
	cmd_buffer[7] = Y_start;
	cmd_buffer[8] = (X_end >> 8);
	cmd_buffer[9] = X_end;
	cmd_buffer[10] = (Y_end >> 8);
	cmd_buffer[11] = Y_end;
	cmd_buffer[12] = border_pixel;
	cmd_buffer[13]= (color >> 16);
	cmd_buffer[14]= (color >> 8);
	cmd_buffer[15]= color;
	cmd_buffer[16]= 0x0A;
	cmd_buffer[17]= 0x00;
	cmd_buffer[18]= 0x0D;

    wiringPiSPIDataRW(SPI_CHANNEL, cmd_buffer, CMD_LEN_SQUARE);
	
    return WF35M_check_busy();
}


int WF35M_spi_clear_cmd(int X_start, int Y_start, int X_end, int Y_end)
{
	unsigned char cmd_buffer[CMD_LEN_CLEAR];

	cmd_buffer[0] = 0x31;
	cmd_buffer[1] = SB3;
	cmd_buffer[2] = MODE_CLEAR;
	cmd_buffer[3] = (X_start >> 8);
	cmd_buffer[4] = X_start;
	cmd_buffer[5] = (Y_start >> 8);
	cmd_buffer[6] = Y_start;
	cmd_buffer[7] = (X_end >> 8);
	cmd_buffer[8] = X_end;
	cmd_buffer[9] = (Y_end >> 8);
	cmd_buffer[10] = Y_end;
	cmd_buffer[11]= 0x0A;
	cmd_buffer[12]= 0x00;
	cmd_buffer[13]= 0x0D;

    wiringPiSPIDataRW(SPI_CHANNEL, cmd_buffer, CMD_LEN_CLEAR);

    return WF35M_check_busy();
}

/* 20160829, Joey Ni, test for LCD bug ------------------ } */


int UpdateScreenFunction(int screenIndex)
{
    /* 20160829, Joey Ni, test for screen bug { */
    WF35M_screen_update(screenIndex);
    return 0;
    /* 20160829, Joey Ni, test for screen bug } */

    unsigned char infoScreen[13] = {0x31, 0x04, 0x32, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0a, 0x00, 0x0d};
    unsigned char countScreen[13] = {0x31, 0x04, 0x32, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0a, 0x00, 0x0d};
    unsigned char powerOffScreen[13] = {0x31, 0x04, 0x32, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0a, 0x00, 0x0d};
    unsigned char menuScreen[13] = {0x31, 0x04, 0x32, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x0a, 0x00, 0x0d};
    unsigned char configScreen[13] = {0x31, 0x04, 0x32, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x0a, 0x00, 0x0d};
    unsigned char popUpScreen[18] = {0x31, 0x04, 0x31, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0a, 0x00, 0x0d};
    unsigned char endString[3] = {0x0a, 0x00, 0x0d};
    unsigned char startString[5] = {0x31, 0x04, 0x31, 0x10, 0x00};
    unsigned char startString2[5] = {0x31, 0x04, 0x31, 0x00, 0x00};
   
    if(screenIndex == 1)
    {   
        //count p2    
        unsigned char countNoPositionColorString[10] = {0x00, 0x69, 0x00, 0x5a, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        unsigned char goodNoPositionColorString[10] = {0x00, 0x69, 0x00, 0x89, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};   
        unsigned char totalBadNoPositionColorString[10] = {0x00, 0x69, 0x00, 0xb9, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};   

        unsigned char * commandArrayPtr;
        unsigned char * commandPtr, * countPtr;
        int arraySize = 0;
 
        SendCommandMessageFunction(countScreen, 13);
        nanosleep((const struct timespec[]){{0, 450000000L}}, NULL);
        //sleep(1);

        //count no.
        arraySize = strlen(CountNo);
#ifdef DEBUG
        printf("array size:%d\n", arraySize);
#endif
        commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(18 + arraySize)); 
        memset(commandArrayPtr, 0, sizeof(unsigned char)*(18 + arraySize));
        commandPtr = commandArrayPtr;
        memcpy(commandPtr, startString, 5);
        commandPtr = commandPtr + 5;
        memcpy(commandPtr, countNoPositionColorString, 10);
        commandPtr = commandPtr + 10;
        memcpy(commandPtr, CountNo, arraySize);
        commandPtr = commandPtr + arraySize;
        memcpy(commandPtr, endString, 3);
        SendCommandMessageFunction(commandArrayPtr, 18 + arraySize);
        if(commandArrayPtr != NULL) 
        {
            free(commandArrayPtr);
            commandPtr = NULL;
        }
 
        //good count
        if(arraySize > 0)
        {
            commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(18 + 7));
            memset(commandArrayPtr, 0, sizeof(unsigned char)*(18 + 7));
            countPtr = (unsigned char *)malloc(sizeof(unsigned char)*7);
            memset(countPtr, 0, sizeof(unsigned char)*7);

            sprintf(countPtr, "%ld", (GoodCount%1000000));

            commandPtr = commandArrayPtr;
            memcpy(commandPtr, startString, 5);
            commandPtr = commandPtr + 5;
            memcpy(commandPtr, goodNoPositionColorString, 10);
            commandPtr = commandPtr + 10;
            memcpy(commandPtr, countPtr, 7);
            commandPtr = commandPtr + 7;
            memcpy(commandPtr, endString, 3);
            SendCommandMessageFunction(commandArrayPtr, 18 + 7);
            if(commandArrayPtr != NULL) 
            {
                free(commandArrayPtr);
                commandPtr = NULL;
            }
            if(countPtr != NULL)
            {
                free(countPtr);
                countPtr = NULL;
            }

            commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(18 + 7));
            memset(commandArrayPtr, 0, sizeof(unsigned char)*(18 + 7));
            countPtr = (unsigned char *)malloc(sizeof(unsigned char)*7);
            memset(countPtr, 0, sizeof(unsigned char)*7);
         
            sprintf(countPtr, "%ld", (TotalBadCount%1000000));

            commandPtr = commandArrayPtr;
            memcpy(commandPtr, startString, 5);
            commandPtr = commandPtr + 5;
            memcpy(commandPtr, totalBadNoPositionColorString, 10);
            commandPtr = commandPtr + 10;
            memcpy(commandPtr, countPtr, 7);
            commandPtr = commandPtr + 7;
            memcpy(commandPtr, endString, 3);
            SendCommandMessageFunction(commandArrayPtr, 18 + 7);
            if(commandArrayPtr != NULL) 
            {
                free(commandArrayPtr);
                commandPtr = NULL;
            }
            if(countPtr != NULL)
            {
                free(countPtr);
                countPtr = NULL;
            }

        }
        SendCommandMessageFunction(popUpScreen, 18);
    }else if(screenIndex == 99)
    {
        unsigned char BarcodeError1[24] = {0x31, 0x04, 0x31, 0x13, 0x10, 
                   0x00, 0x5a, 0x00, 0x78, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 'E', 'r', 'r', 'o', 'r', '1', 0x0a, 0x00, 0x0d};

        unsigned char BarcodeError2[24] = {0x31, 0x04, 0x31, 0x13, 0x10, 
                   0x00, 0x5a, 0x00, 0x78, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 'E', 'r', 'r', 'o', 'r', '2', 0x0a, 0x00, 0x0d};

        unsigned char BarcodeError3[24] = {0x31, 0x04, 0x31, 0x13, 0x10, 
                   0x00, 0x5a, 0x00, 0x78, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 'E', 'r', 'r', 'o', 'r', '3', 0x0a, 0x00, 0x0d};

        unsigned char BarcodeError4[24] = {0x31, 0x04, 0x31, 0x13, 0x10, 
                   0x00, 0x5a, 0x00, 0x78, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 'E', 'r', 'r', 'o', 'r', '4', 0x0a, 0x00, 0x0d};

        unsigned char BarcodeError5[24] = {0x31, 0x04, 0x31, 0x13, 0x10, 
                   0x00, 0x5a, 0x00, 0x78, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 'E', 'r', 'r', 'o', 'r', '5', 0x0a, 0x00, 0x0d};

        if(BarcodeIndex == 0)
            SendCommandMessageFunction(BarcodeError1, 28);
        else if(BarcodeIndex == 1)
            SendCommandMessageFunction(BarcodeError2, 28);
        else if(BarcodeIndex == 2)
            SendCommandMessageFunction(BarcodeError4, 28);
        else if(BarcodeIndex == 3)
            SendCommandMessageFunction(BarcodeError3, 28);
        else
            SendCommandMessageFunction(BarcodeError5, 28);

    }else if(screenIndex == 2)
    {
        //power p3
        SendCommandMessageFunction(powerOffScreen, 13);
        nanosleep((const struct timespec[]){{0, 450000000L}}, NULL);
        SendCommandMessageFunction(popUpScreen, 18);
    }else if(screenIndex == 3)
    {
        unsigned char bar1[34] = {0x31,0x04,0x31,0x03,0x00,0x00,0x48,0x00,0x37,0xFF,0xFF,0xFF,0x00,0x00,0x00,
                                           '>',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','<', 0x0a, 0x00, 0x0d};
        
        //menu1 p4
        SendCommandMessageFunction(menuScreen, 13);
        nanosleep((const struct timespec[]){{0, 450000000L}}, NULL);

        SendCommandMessageFunction(bar1, 34);
        SendCommandMessageFunction(popUpScreen, 18);
    }else if (screenIndex == 4)
    {
        unsigned char bar2[34] = {0x31,0x04,0x31,0x03,0x00,0x00,0x48,0x00,0x69,0xFF,0xFF,0xFF,0x00,0x00,0x00,
                                           '>',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','<', 0x0a, 0x00, 0x0d};
        //menu 2 p4
        SendCommandMessageFunction(menuScreen, 13);
        nanosleep((const struct timespec[]){{0, 450000000L}}, NULL);


        SendCommandMessageFunction(bar2, 34);
        SendCommandMessageFunction(popUpScreen, 18);
    }else if(screenIndex == 5)
    {
        unsigned char bar3[34] = {0x31,0x04,0x31,0x03,0x00,0x00,0x48,0x00,0x9D,0xFF,0xFF,0xFF,0x00,0x00,0x00,
                                           '>',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','<', 0x0a, 0x00, 0x0d};
        //menu 3 p4
        SendCommandMessageFunction(menuScreen, 13);
        nanosleep((const struct timespec[]){{0, 450000000L}}, NULL);

        SendCommandMessageFunction(bar3, 34);
        SendCommandMessageFunction(popUpScreen, 18);
    }else if(screenIndex == 6)
    {
        unsigned char machineNoPositionColorString[10] = {0x00, 0x8c, 0x00, 0x8c, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        unsigned char ipAddressPositionColorString[10] = {0x00, 0x64, 0x00, 0xc8, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};   

        unsigned char * commandArrayPtr;
        unsigned char * commandPtr, * countPtr;
        int arraySize = 0;
 
        SendCommandMessageFunction(configScreen, 13);
        nanosleep((const struct timespec[]){{0, 450000000L}}, NULL);
        //sleep(1);

        int fd2 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        struct ifreq ethreq;
        memset(&ethreq, 0, sizeof(ethreq));
        strncpy(ethreq.ifr_name, ZHNETWORKTYPE, IFNAMSIZ);
        ioctl(fd2, SIOCGIFFLAGS, &ethreq);

        if(ethreq.ifr_flags & IFF_RUNNING)
        {
            char ipAddrArray[17] ;
            int fd = socket(AF_INET, SOCK_DGRAM, 0);
            struct ifreq ifr;
            ifr.ifr_addr.sa_family = AF_INET;
            strncpy(ifr.ifr_name, ZHNETWORKTYPE, IFNAMSIZ-1);
            ioctl(fd, SIOCGIFADDR, &ifr);
            close(fd);

            memset(ipAddrArray, 0, sizeof(char)*17);
            strncpy(ipAddrArray, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), sizeof(char)*17);
            arraySize = 17;
            commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(18 + arraySize)); 
            memset(commandArrayPtr, 0, sizeof(unsigned char)*(18 + arraySize));
            commandPtr = commandArrayPtr;
            memcpy(commandPtr, startString, 5);
            commandPtr = commandPtr + 5;
            memcpy(commandPtr, ipAddressPositionColorString, 10);
            commandPtr = commandPtr + 10;
            memcpy(commandPtr, ipAddrArray, arraySize);
            commandPtr = commandPtr + arraySize;
            memcpy(commandPtr, endString, 3);
            SendCommandMessageFunction(commandArrayPtr, 18 + arraySize);
            if(commandArrayPtr != NULL) 
            {
                free(commandArrayPtr);
                commandPtr = NULL;
            }
        }else
        {
            ;//
        }
        close(fd2);

        //machine no.
        arraySize = strlen(MachineNo);
#ifdef DEBUG
        printf("array size:%d\n", arraySize);
#endif
        commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(18 + arraySize)); 
        memset(commandArrayPtr, 0, sizeof(unsigned char)*(18 + arraySize));
        commandPtr = commandArrayPtr;
        memcpy(commandPtr, startString, 5);
        commandPtr = commandPtr + 5;
        memcpy(commandPtr, machineNoPositionColorString, 10);
        commandPtr = commandPtr + 10;
        memcpy(commandPtr, MachineNo, arraySize);
        commandPtr = commandPtr + arraySize;
        memcpy(commandPtr, endString, 3);
        SendCommandMessageFunction(commandArrayPtr, 18 + arraySize);
        if(commandArrayPtr != NULL) 
        {
            free(commandArrayPtr);
            commandPtr = NULL;
        }
 
        SendCommandMessageFunction(popUpScreen, 18);
   
    }else if(screenIndex == 7)
    {
        unsigned char repairModePosition[28] = {0x31,0x04,0x31,0x03,0x00,0x00,0x0A,0x00,0x32,0xFF,0xFF,0xFF,0x00,0x00,0x00
                                                    ,'R','e','p','a','i','r','i','n','g',0x0a, 0x00, 0x0d};
        unsigned char repairerPositionColorString[14] = {0x00, 0x0A, 0x00, 0x86, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,'I','D',':', ' '};   

        unsigned char * commandArrayPtr;
        unsigned char * commandPtr, * countPtr;
        int arraySize = 0;
 
        SendCommandMessageFunction(powerOffScreen, 13);
        nanosleep((const struct timespec[]){{0, 450000000L}}, NULL);

        SendCommandMessageFunction(repairModePosition, 27);

        //machine no.
        arraySize = strlen(RepairNo);
#ifdef DEBUG
        printf("array size:%d\n", arraySize);
#endif
        commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(22 + arraySize)); 
        memset(commandArrayPtr, 0, sizeof(unsigned char)*(22 + arraySize));
        commandPtr = commandArrayPtr;
        memcpy(commandPtr, startString2, 5);
        commandPtr = commandPtr + 5;
        memcpy(commandPtr, repairerPositionColorString, 14);
        commandPtr = commandPtr + 14;
        memcpy(commandPtr, RepairNo, arraySize);
        commandPtr = commandPtr + arraySize;
        memcpy(commandPtr, endString, 3);
        SendCommandMessageFunction(commandArrayPtr, 22 + arraySize);
        if(commandArrayPtr != NULL) 
        {
            free(commandArrayPtr);
            commandPtr = NULL;
        }
 
        SendCommandMessageFunction(popUpScreen, 18);
    }else
    {
        //info p1
        /*unsigned char lotPostionColorString[10] = {0x00, 0x78, 0x00, 0x3c, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        unsigned char partPositionColorString1[10] = {0x00, 0x78, 0x00, 0x5f, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        unsigned char partPositionColorString2[10] = {0x00, 0x64, 0x00, 0x82, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        unsigned char countNoPositionColorString[10] = {0x00, 0x96, 0x00, 0xa5, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        unsigned char userNoPositionColorString[10] = {0x00, 0x96, 0x00, 0xcf, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        */
        unsigned char lotPostionColorString[10] = {0x00, 0x5f, 0x00, 0x3c, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        unsigned char partPositionColorString1[10] = {0x00, 0x69, 0x00, 0x5f, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        unsigned char partPositionColorString2[10] = {0x00, 0x69, 0x00, 0x82, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        unsigned char countNoPositionColorString[10] = {0x00, 0x82, 0x00, 0xa5, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        unsigned char userNoPositionColorString[10] = {0x00, 0x82, 0x00, 0xcf, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};

        unsigned char * commandArrayPtr;
        unsigned char * commandPtr,  * managerPtr;
        int arraySize = 0;

        SendCommandMessageFunction(infoScreen, 13);
        nanosleep((const struct timespec[]){{0, 450000000L}}, NULL);
        //sleep(1);
        
        //lot no
        if(strlen(ISNo) > 0)
        { 
            commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*32); 
            memset(commandArrayPtr, 0, sizeof(unsigned char)*32);
            commandPtr = commandArrayPtr;
            memcpy(commandPtr, startString, 5);
            commandPtr = commandPtr + 5;
            memcpy(commandPtr, lotPostionColorString, 10);
            commandPtr = commandPtr + 10;
            memcpy(commandPtr, ISNo, 14);
            commandPtr = commandPtr +14;
            memcpy(commandPtr, endString, 3);

            //31 4 31 3 10 0 78 0 3c ff ff ff 0 0 0 31 32 33 34 35 36 37 38 39 30 31 32 33 34 a 0 d
            SendCommandMessageFunction(commandArrayPtr, 32);
            if(commandArrayPtr != NULL) 
            {
                free(commandArrayPtr);
                commandPtr = NULL;
            }
        }
        if(strlen(ManagerCard) > 0)
        {
            //part no. 1
            commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*28);
            memset(commandArrayPtr, 0, sizeof(unsigned char)*28);
            commandPtr = commandArrayPtr;
            memcpy(commandPtr, startString, 5);
            commandPtr = commandPtr + 5;
            memcpy(commandPtr, partPositionColorString1, 10);
            commandPtr = commandPtr + 10;
            memcpy(commandPtr, ManagerCard, 10);
            commandPtr = commandPtr + 10;
            memcpy(commandPtr, endString, 3);

            SendCommandMessageFunction(commandArrayPtr, 28);
            if(commandArrayPtr != NULL)
            {
                free(commandArrayPtr);
                commandPtr = NULL;
            }
        
            //part no. 2
            commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char )*32);
            memset(commandArrayPtr, 0, sizeof(unsigned char)*32);
            commandPtr = commandArrayPtr;
            memcpy(commandPtr, startString, 5);
            commandPtr = commandPtr + 5;
            memcpy(commandPtr, partPositionColorString2, 10);
            commandPtr = commandPtr + 10;
            managerPtr = ManagerCard;
            managerPtr = managerPtr + 10;
            memcpy(commandPtr, managerPtr, 14);
            commandPtr = commandPtr + 14;
            memcpy(commandPtr, endString, 3);

            SendCommandMessageFunction(commandArrayPtr, 32);
            if(commandArrayPtr != NULL)
            {
                free(commandArrayPtr);
                commandPtr = NULL;
            }
            if(managerPtr != NULL)
            {
                managerPtr = NULL;
            }
        }

        //count no.
        arraySize = strlen(CountNo);
#ifdef DEBUG
        printf("array size:%d\n", arraySize);
#endif
        commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(18 + arraySize)); 
        memset(commandArrayPtr, 0, sizeof(unsigned char)*(18 + arraySize));
        commandPtr = commandArrayPtr;
        memcpy(commandPtr, startString, 5);
        commandPtr = commandPtr + 5;
        memcpy(commandPtr, countNoPositionColorString, 10);
        commandPtr = commandPtr + 10;
        memcpy(commandPtr, CountNo, arraySize);
        commandPtr = commandPtr + arraySize;
        memcpy(commandPtr, endString, 3);

        //31 4 31 3 10 0 78 0 3c ff ff ff 0 0 0 31 32 33 34 35 36 37 38 39 30 31 32 33 34 a 0 d
        SendCommandMessageFunction(commandArrayPtr, 18 + arraySize);
        if(commandArrayPtr != NULL) 
        {
            free(commandArrayPtr);
            commandPtr = NULL;
        }
                
        //user no
        if(isInPairMode == 1)
        {
            if(CanChangeRepairModeFlag == 3)
            {
                arraySize = strlen("Repair Done");
                
                commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(18 + arraySize)); 
                memset(commandArrayPtr, 0, sizeof(unsigned char)*(18 + arraySize));
                commandPtr = commandArrayPtr;
                memcpy(commandPtr, startString, 5);
                commandPtr = commandPtr + 5;
                memcpy(commandPtr, userNoPositionColorString, 10);
                commandPtr = commandPtr + 10;
                memcpy(commandPtr, "Repair Done", arraySize);
                commandPtr = commandPtr + arraySize;
                memcpy(commandPtr, endString, 3);

                SendCommandMessageFunction(commandArrayPtr, 18 + arraySize);
                if(commandArrayPtr != NULL) 
                {
                    free(commandArrayPtr);
                    commandPtr = NULL;
                }
            }else
            {
                arraySize = strlen("Repairing");
                
                commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(18 + arraySize)); 
                memset(commandArrayPtr, 0, sizeof(unsigned char)*(18 + arraySize));
                commandPtr = commandArrayPtr;
                memcpy(commandPtr, startString, 5);
                commandPtr = commandPtr + 5;
                memcpy(commandPtr, userNoPositionColorString, 10);
                commandPtr = commandPtr + 10;
                memcpy(commandPtr, "Repairing", arraySize);
                commandPtr = commandPtr + arraySize;
                memcpy(commandPtr, endString, 3);

                SendCommandMessageFunction(commandArrayPtr, 18 + arraySize);
                if(commandArrayPtr != NULL) 
                {
                    free(commandArrayPtr);
                    commandPtr = NULL;
                }
            }
        }else
        {
            arraySize = strlen(UserNo);
            char *userNoPtr = UserNo;
            if(arraySize > 25)
            {
                arraySize = arraySize - 24;
                userNoPtr = UserNo + 24;
            } 
            commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(18 + arraySize)); 
            memset(commandArrayPtr, 0, sizeof(unsigned char)*(18 + arraySize));
            commandPtr = commandArrayPtr;
            memcpy(commandPtr, startString, 5);
            commandPtr = commandPtr + 5;
            memcpy(commandPtr, userNoPositionColorString, 10);
            commandPtr = commandPtr + 10;
            memcpy(commandPtr, userNoPtr, arraySize);
            commandPtr = commandPtr + arraySize;
            memcpy(commandPtr, endString, 3);

            SendCommandMessageFunction(commandArrayPtr, 18 + arraySize);
            if(commandArrayPtr != NULL) 
            {
                free(commandArrayPtr);
                commandPtr = NULL;
            }
        }
        SendCommandMessageFunction(popUpScreen, 18);
    }
    return 0;
}


void * ChangeScreenEventListenFunction(void *argument)
{
    int screenIndex = 3;
    char flagForZHPIN32, flagForZHPIN22, flagForZHPIN36, flagForZHPIN38;

    flagForZHPIN32 = flagForZHPIN22 = flagForZHPIN36 = flagForZHPIN38 = 0;
    //ScreenIndex = screenIndex;

    /* 20160830, Joey Ni, test for LCD bug ------------------ { */
    char text_buffer[TEXT_LEN_MAX];
    /* Timer */
    signal(SIGALRM, alarm_handler);
    alarm(1);
    /* 20160830, Joey Ni, test for LCD bug ------------------ } */

    while(1)
    {
        /* 20160830, Joey Ni, test for LCD bug ------------------ { */
        if (flag_timer) {
            WF35M_screen_update(111);
            flag_timer = 0;
            alarm(1);
        }
        /* 20160830, Joey Ni, test for LCD bug ------------------ } */

        // ZHPIN32 = Key Down
        // ZHPIN22 = Key Up
        // ZHPIN38 = Key Enter
        // ZHPIN36 = Key Return

        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        if(DisableUpDown == 0 && flagForZHPIN32 == 0 && digitalRead(ZHPIN32) == 0)
        {
            printf("get ZHPIN_32 event\n");
            //screenIndex = (screenIndex + 1) % 2;
            screenIndex = (screenIndex + 1) % 3 + 3;
            ScreenIndex = screenIndex;
            flagForZHPIN32 = 1; 
            UpdateScreenFunction(screenIndex);
        }else if(flagForZHPIN32 == 1 && digitalRead(ZHPIN32) == 1)
        {
            flagForZHPIN32 = 0;
        }else if(DisableUpDown == 0 && flagForZHPIN22 == 0 && digitalRead(ZHPIN22) == 0)
        {
            printf("get ZHPIN_22 event\n");
            //screenIndex = (screenIndex + 1) % 2;
            screenIndex = (screenIndex - 1) % 3 + 3;
            ScreenIndex = screenIndex;
            flagForZHPIN36 = 1; 
            UpdateScreenFunction(screenIndex);
        }else if(flagForZHPIN22 == 1 && digitalRead(ZHPIN22) == 1)
        {
            flagForZHPIN22 = 0;
        }else if(flagForZHPIN36 == 0 && digitalRead(ZHPIN36) == 0)
        {
            printf("get ZHPIN_36 event\n");
            ScreenIndex = screenIndex;
            flagForZHPIN36 = 1;
            UpdateScreenFunction(screenIndex);
        }else if(flagForZHPIN36 == 1 && digitalRead(ZHPIN36) == 1)
        {
            flagForZHPIN36 = 0;
            DisableUpDown = 0;
        }else if(DisableUpDown == 0 && flagForZHPIN38 == 0 && digitalRead(ZHPIN38) == 0)
        {
            printf("get ZHPIN_38 event\n");
            if(screenIndex == 3)
            {
                ScreenIndex = 0;
                UpdateScreenFunction(0);
            }
            else if(screenIndex == 4)
            {
                ScreenIndex = 1;
                UpdateScreenFunction(1);
            }
            else
            {
                //if(isInPairMode)
                //{
                //    ScreenIndex =7; 
                //    UpdateScreenFunction(7);
                //}else
                //{
                    ScreenIndex =6; 
                    UpdateScreenFunction(6);
                //}
            }
            flagForZHPIN38 = 1;
            DisableUpDown = 1;
        }else if(flagForZHPIN38 == 1 && digitalRead(ZHPIN38) == 1)
        {
            flagForZHPIN38 = 0;
        }
        else;
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        nanosleep((const struct timespec[]){{0, 100000000L}}, NULL);
        sleep(0);
    }
}

int SendCommandMessageFunction (unsigned char *message, int arrayLength)
{
    unsigned char *copyMessageArray;
    int forCount = 0;

#ifdef ZHCHECKSCREENBUSY
    unsigned char x[2] = {0x00, 0x00};
    int maxRetry = 100;

    while(x[1] != 0x01)
    {
        x[0] = 0x01;
        x[1] = 0x00;
        wiringPiSPIDataRW(CSCHANNEL, x, 2);
        printf("we wait %x\n", x[1]);
        if(x[1] == 0x01) break;
        nanosleep((const struct timespec[]){{0, 10000000L}}, NULL);
        maxRetry --;
        if(maxRetry <= 0)
        {
            digitalWrite (ZHPIN33, LOW);
            //nanosleep((const struct timespec[]){{0, 200000000L}}, NULL);
            sleep(2);    

            digitalWrite (ZHPIN33, HIGH);
            //nanosleep((const struct timespec[]){{0, 1500000000L}}, NULL);
            sleep(2);    
            maxRetry = 100;
            printf("refresh\n");
        }
    }
    printf("lcd idle\n");
#endif

    copyMessageArray = (unsigned char *)malloc(sizeof(unsigned char)*arrayLength);

    memset(copyMessageArray, 0, sizeof(unsigned char)*arrayLength);
    memcpy(copyMessageArray, message, arrayLength);

#ifdef DEBUG 
    printf("[%s|%d] ", __func__, __LINE__); 
    for(forCount = 0; forCount < arrayLength; forCount++)
    {
        printf("%x ", copyMessageArray[forCount]);
    }
    printf("\n");
#endif

    wiringPiSPIDataRW(CSCHANNEL, copyMessageArray, arrayLength);
    ///nanosleep((const struct timespec[]){{0, 100000000L}}, NULL);
    //sleep(1);
    if(copyMessageArray != NULL)
    {
        //printf("free alloc memory\n");
        free(copyMessageArray);
    }
    return 0;
}

