//------------------------------------------------------------------------
//fpga_tester.cpp
//
// This is the C++ source file for loading test vectors onto block ram
// running circuit, and dumping results out
// Top OK board(2V1):15160002V1(IO controller)
// Bottom OK board(AP2):1452000AP2(circuit emu)
//
//
//------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>     // UNIX standard function definitions
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <termios.h>    // POSIX terminal control definitions
#include <sstream>
//#include <chrono>
//#include <thread>

//#include <conio.h>
#include <cstring>


#include "okFrontPanelDLL.h"
// Change bitstream file

using namespace std;

//////////////////////////////////////////////////
// Bitstream for different scan chain

#define CIRCUIT_EMU                 "OK_setup.bit"
#define TOP_LEVEL_EMU               "top_level_FPGA_tester_OK.bit"
#define TOP_LEVEL_EMU_V1            "top_level_fpga_tester_ok_v1.bit"
#define TOP_LEVEL_EMU_V1_5          "top_level_fpga_tester_ok_v1_5.bit"
#define DESIGN                      "design.bit"
#define WORKING_FINAL_EMU           "top_level_fpga_tester_ok_final.bit"

//////////////////////////////////////////////////
#define ALTERA_CONFIGURATION_FILE   "Text_IO.rbf"
#define EMU_CIRCUIT                 "chip1_emu.bit"
#define EMU_CIRCUIT_CAP             "chip1_emu_cap.bit"
#define TOP_OK                      "15160002V1"
#define BOTTOM_OK                   "1452000AP2"
//#if defined(_WIN32)
//#define strncpy strncpy_s
//#define sscanf  sscanf_s
//#endif


const int MAX_CHARS_PER_LINE = 512;
const int MAX_TOKENS_PER_LINE = 4;
const char* const DELIMITER = " ";
string input_data_file;
string user_para;

bool user_defined_name = 1;
bool auto_name = 0;
bool customized_uart = 0;
int mult_coord = 1;
int mult_cell = 1;
int use_coord_origin;
double ori_x;
double ori_y;
char cell_data_name[20];    //this is the name of the file that contains cell data
char ori_cell_ori[10];
char stk_loc_ori[10];
char input_ori[10];
double stk_x,stk_y,stk_dx,stk_dy;
int flip_H, flip_V;
char cell_name[20];
char cell_prop_name[20];    //this is the name of the file that contains cell properities
char cell_type[20];
double cell_x_from_data, cell_y_from_data;
char cell_co[10];
double cell_height, cell_width;
int cell_CNT;
double offset_x, offset_y;  
double read_x, read_y;
double norm_x, norm_y;
double final_x_FPGA, final_y_FPGA;
int goal;


//const char* const input_data_file;

int read_cell_and_get_prop(){
    ifstream fcell;
    string cell_data_string_name(cell_data_name);
    fcell.open((cell_data_string_name).c_str()); // open a file
    if (!fcell.good()) 
        return 1; // exit if file not found
    printf("Successfully load the cell_data file\n");
    char buf_cell[MAX_CHARS_PER_LINE];
    bool got_cell = 0;
    char temp_cell_name[20];
    fcell.getline(buf_cell, MAX_CHARS_PER_LINE);
    fcell.getline(buf_cell, MAX_CHARS_PER_LINE);
    fcell.getline(buf_cell, MAX_CHARS_PER_LINE);
    fcell.getline(buf_cell, MAX_CHARS_PER_LINE);
    /*
    fcell.getline(buf_cell, MAX_CHARS_PER_LINE);
    sscanf(buf_cell,"%s",temp_cell_name);
    cout<<temp_cell_name<<endl;
    cout<<cell_name<<endl;
    if(strcmp(temp_cell_name,cell_name)==0){
        cout<<"HAHAHAHA"<<endl;
    }
    */
    while(!fcell.eof() && got_cell==0){
        fcell.getline(buf_cell, MAX_CHARS_PER_LINE);
        sscanf(buf_cell,"%s",temp_cell_name);
        if(strcmp(temp_cell_name,cell_name)==0){
            got_cell = 1;
            sscanf(buf_cell,"%s %s %lf %lf %s", cell_name, cell_type, &cell_x_from_data, &cell_y_from_data, cell_co);
            //printf("the cell type is%s\n", cell_type);
            printf("Successfully located the origin cell\n");
        }
    }

    if(got_cell==0){
        printf("Could NOT locate the cell. Please check the cell name and configuration file\n");
        return 1;
    }

    fcell.close();

    ifstream fprop;
    string cell_prop_string_name(cell_prop_name);
    fprop.open((cell_prop_string_name).c_str()); // open a file
    if (!fprop.good()) 
        return 1; // exit if file not found
    printf("Successfully load the cell_prop file\n");
    char buf_prop[MAX_CHARS_PER_LINE];
    bool got_prop = 0;
    char temp_cell_type[20];
    //int garbage2;

    fprop.getline(buf_prop,MAX_CHARS_PER_LINE);
    fprop.getline(buf_prop,MAX_CHARS_PER_LINE);
    fprop.getline(buf_prop,MAX_CHARS_PER_LINE);
    fprop.getline(buf_prop,MAX_CHARS_PER_LINE);


    while(!fprop.eof() && got_prop==0){
        fprop.getline(buf_prop,MAX_CHARS_PER_LINE);
        sscanf(buf_prop, "%*d %s", temp_cell_type);

        if(strcmp(temp_cell_type,cell_type) == 0){
            got_prop = 1;
            sscanf(buf_prop, "%d %*s %lf %lf", &cell_CNT, &cell_height, &cell_width);
            printf("Successfully located the cell properities\n");
            //printf("and the width is %lf\n", cell_width);
        }

    }

    if(got_prop==0){
        printf("Could NOT read the cell height and width. Please check the cell prop file\n");
        return 1;
    }


    if(ori_cell_ori[0] == 'L' && ori_cell_ori[1] == 'L'){
        ori_x = cell_x_from_data;
        ori_y = cell_y_from_data;
    }
    if(ori_cell_ori[0] == 'L' && ori_cell_ori[1] == 'R'){
        ori_x = cell_x_from_data + cell_width;
        ori_y = cell_y_from_data;
    }
    if(ori_cell_ori[0] == 'U' && ori_cell_ori[1] == 'L'){
        ori_x = cell_x_from_data;
        ori_y = cell_y_from_data + cell_height;
    }
    if (ori_cell_ori[0] == 'U' && ori_cell_ori[1] == 'R'){
        ori_x = cell_x_from_data + cell_width;
        ori_y = cell_y_from_data + cell_height;
    }




    return 0;
}


void load_coord_test(){
//this function is not actually used, it here becuase i need to test file IO without FPGA board


}



int load_config(){

    //printf("Loading teh configuration files\n");

    ifstream fconfig;
    fconfig.open("config.txt"); // open a file
    if (!fconfig.good()) 
        return 1; // exit if file not found
    printf("Successfully load the config file\n");
    char buf_config[MAX_CHARS_PER_LINE];
    int looper = 0;
    fconfig.getline(buf_config, MAX_CHARS_PER_LINE);
    fconfig.getline(buf_config, MAX_CHARS_PER_LINE);
    fconfig.getline(buf_config, MAX_CHARS_PER_LINE);
    //cout<<buf_config<<endl;
    while (looper < MAX_CHARS_PER_LINE){
        if (buf_config[looper]==':'){
            use_coord_origin = (int)buf_config[looper+1] - 48;
            break;
        }
        else{
            looper++;
        }
    }
    looper = 0;
    fconfig.getline(buf_config, MAX_CHARS_PER_LINE);
    //int garbage;
    //buf_config[0] = ' ';
    //cout<<buf_config<<endl;
    sscanf(buf_config,"2. Origin X value (If enabled):%lf",&ori_x);
    fconfig.getline(buf_config, MAX_CHARS_PER_LINE);
    sscanf(buf_config,"3. Origin Y value (If enabled):%lf",&ori_y);

    fconfig.getline(buf_config, MAX_CHARS_PER_LINE);
    sscanf(buf_config,"4. Origin Cell Data File Name (If enabled):%s",cell_data_name);

    fconfig.getline(buf_config, MAX_CHARS_PER_LINE);
    sscanf(buf_config,"5. Origin Cell Orientation (If enabled, choose from UL UR LL LR):%s",ori_cell_ori);

    fconfig.getline(buf_config, MAX_CHARS_PER_LINE);
    sscanf(buf_config,"6. Strike Location Orientation (choose from UL UR LL LR):%s",stk_loc_ori);

    fconfig.getline(buf_config, MAX_CHARS_PER_LINE);
    sscanf(buf_config,"7. Strike Area X:%lf",&stk_x);

    fconfig.getline(buf_config, MAX_CHARS_PER_LINE);
    sscanf(buf_config,"8. Strike Area Y:%lf",&stk_y);

    fconfig.getline(buf_config, MAX_CHARS_PER_LINE);
    sscanf(buf_config,"9. Strike Area delta x:%lf",&stk_dx);

    fconfig.getline(buf_config, MAX_CHARS_PER_LINE);
    sscanf(buf_config,"10.Strike Area delta y:%lf",&stk_dy);

    fconfig.getline(buf_config, MAX_CHARS_PER_LINE);
    sscanf(buf_config,"11.Flip Horizontally (0/1. 1 will flip the coordinates horizontally):%d",&flip_H);

    fconfig.getline(buf_config, MAX_CHARS_PER_LINE);
    sscanf(buf_config,"12.Flip Vertically (0/1. 1 will flip the coordinates vertically):%d",&flip_V);

    fconfig.getline(buf_config, MAX_CHARS_PER_LINE);
    sscanf(buf_config,"13.Name of the Cell (If using cell coordinates):%s",cell_name);

    fconfig.getline(buf_config, MAX_CHARS_PER_LINE);
    sscanf(buf_config,"14.Origin Cell Properity File Name (If using cell coordinates):%s",cell_prop_name);

    fconfig.getline(buf_config, MAX_CHARS_PER_LINE);
    sscanf(buf_config,"15.Input Coordinates Orientation (choose from UL UR LL LR):%s",input_ori);


    fconfig.getline(buf_config, MAX_CHARS_PER_LINE);
    sscanf(buf_config,"16.Enable Multi Coordinate Mode (1/0. 1 will read X and Y from each line. 0 will only read the first X and Y from first line):%d",&mult_coord);
    
    fconfig.getline(buf_config, MAX_CHARS_PER_LINE);
    sscanf(buf_config,"17.Multi Coords Mode Use Cell Name to Locate Coords (1/0. 1 will read a cell name from each line and calculate X and Y):%d",&mult_cell);

    int number_stk_x = stk_x/stk_dx;
    int number_stk_y = stk_y/stk_dy;


    goal = (number_stk_y + 1)*number_stk_x + number_stk_y;
    //cout<<goal<<endl;

    return 0;
}



bool send_commands(){
    int USB = open( "/dev/ttyUSB1", O_RDWR| O_NOCTTY ); 

    struct termios tty;
    struct termios tty_old;
    memset (&tty, 0, sizeof tty);   

    /* Error Handling */
    if ( tcgetattr ( USB, &tty ) != 0 ) {
       std::cout << "Error " << errno << " from tcgetattr: " << strerror(errno) << std::endl;
    }   

    /* Save old tty parameters */
    tty_old = tty;  

    /* Set Baud Rate */
    cfsetospeed (&tty, (speed_t)B9600);
    cfsetispeed (&tty, (speed_t)B9600); 

    /* Setting other Port Stuff */
    tty.c_cflag     &=  ~PARENB;            // Make 8n1
    tty.c_cflag     &=  ~CSTOPB;
    tty.c_cflag     &=  ~CSIZE;
    tty.c_cflag     |=  CS8;    

    tty.c_cflag     &=  ~CRTSCTS;           // no flow control
    tty.c_cc[VMIN]   =  1;                  // read doesn't block
    tty.c_cc[VTIME]  =  5;                  // 0.5 seconds read timeout
    tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines 

    /* Make raw */
    cfmakeraw(&tty);    

    /* Flush Port, then applies attributes */
    tcflush( USB, TCIFLUSH );
    if ( tcsetattr ( USB, TCSANOW, &tty ) != 0) {
       std::cout << "Error " << errno << " from tcsetattr" << std::endl;
    }   
    

    unsigned char cmd[] = "SENDTHIS  \r";
    int n_written = 0,
        spot = 0;   

    do {
        n_written = write( USB, &cmd[spot], 1 );
        spot += n_written;
    } while (cmd[spot-1] != '\r' && n_written > 0); 
    

    int n = 0;
        spot = 0;
    char buf = '\0';    

    /* Whole response*/
    char response[1024];
    memset(response, '\0', sizeof response);    

    do {
        n = read( USB, &buf, 1 );
        sprintf( &response[spot], "%c", buf );
        spot += n;
    } while( buf != '\r' && n > 0); 

    if (n < 0) {
        std::cout << "Error reading: " << strerror(errno) << std::endl;
    }
    else if (n == 0) {
        std::cout << "Read nothing!" << std::endl;
    }
    else {
        std::cout << "Response: " << response << std::endl;
    }




}


bool performBlockDump(okCFrontPanel *xem){
    unsigned int blah;
    unsigned char guess;
    unsigned char results[45000];
    unsigned int SU_output, SO_output, Flag_output;
    unsigned int pCAP_So, pCAP_Su, pCAP_Flg;
    unsigned int output_state;
    unsigned int finish;
    unsigned int len;
    unsigned int input;
    unsigned int output;
    unsigned int top_level_state,state;
    //unsigned char buf[20];
    unsigned char* buffer;
    unsigned char* single_line_buffer;
    double* loc_buffer;
    int line_numbers = 0;

    unsigned int* w_address_array;
    unsigned char** results_array;

    string* loc_str_buffer;

//read in from a file



    //first check line numbers
    ifstream fcheck;
    fcheck.open((input_data_file).c_str());
            
    char check_return;
            //int i = 0;
    line_numbers = 0;
    while (fcheck.get(check_return)){
        if (check_return == '\n'){
            ++line_numbers;
        }
    }



    //dynamically allocated buffer size
    buffer = new unsigned char [line_numbers*4];
    single_line_buffer = new unsigned char [4];
    w_address_array = new unsigned int[line_numbers];
    loc_buffer = new double [line_numbers*2];
    loc_str_buffer = new string [line_numbers];
    results_array = new unsigned char* [line_numbers];
    //buffer = (unsigned char*)malloc(sizeof(unsigned char)*line_numbers*4);
    int stop_reading = line_numbers;






    int buffIndex = 0;
    int locbuffIndex = 0;
    int locstrIndex = 0;
    ifstream fin;
    ofstream fout;
    fin.open((input_data_file).c_str()); // open a file
    if (!fin.good()) 
        return 1; // exit if file not found
  
  // read each line of the file
    while (stop_reading>0)
    {
        stop_reading--;
    // read an entire line into memory
        char buf[MAX_CHARS_PER_LINE];
        fin.getline(buf, MAX_CHARS_PER_LINE);
    
        //sscanf(buf, "%*s %*s %*s %*s %lf %lf", &read_x, &read_y);

        //read x and y need to be converted to norm x and y which always uses LL corner as reference
        /*
        if(input_ori[0]=='L' && input_ori[1] =='L'){
            norm_x = read_x;
            norm_y = read_y;
        }
        if(input_ori[0]=='L' && input_ori[1] =='R'){
            norm_x = read_x - stk_x;
            norm_y = read_y;
        }
        if(input_ori[0]=='U' && input_ori[1] =='L'){
            norm_x = read_x;
            norm_y = read_y - stk_y;
        }
        if(input_ori[0]=='U' && input_ori[1] =='R'){
            norm_x = read_x - stk_x;
            norm_y = read_y - stk_y;
        }
        */


        //if we are using direct X and Y from each line
        if (mult_cell==0){
            sscanf(buf, "%*s %*s %*s %*s %lf %lf", &read_x, &read_y);
            norm_x = read_x;
            norm_y = read_y;
        }
        //else we need to read the name of the cell line by line and go get their corresponding w and h
        else{
            char line_cell_name[20];
            char line_cell_type[20];
            double temp_cell_width;
            double temp_cell_height;
            sscanf(buf, "%*s %*s %*s %*s %s",line_cell_name);

            ifstream fcell_temp;
            string cell_data_string_name(cell_data_name);
            fcell_temp.open((cell_data_string_name).c_str()); // open a file
            if (!fcell_temp.good()) 
                return 1; // exit if file not found
            //printf("Successfully load the cell_data file\n");
            char buf_cell_temp[MAX_CHARS_PER_LINE];
            bool got_cell_temp = 0;
            char temp_cell_name_temp[20];
            fcell_temp.getline(buf_cell_temp, MAX_CHARS_PER_LINE);
            fcell_temp.getline(buf_cell_temp, MAX_CHARS_PER_LINE);
            fcell_temp.getline(buf_cell_temp, MAX_CHARS_PER_LINE);
            fcell_temp.getline(buf_cell_temp, MAX_CHARS_PER_LINE);

            while(!fcell_temp.eof() && got_cell_temp==0){
                fcell_temp.getline(buf_cell_temp, MAX_CHARS_PER_LINE);
                sscanf(buf_cell_temp,"%s",temp_cell_name_temp);
                if(strcmp(temp_cell_name_temp,line_cell_name)==0){
                    got_cell_temp = 1;
                    sscanf(buf_cell_temp,"%*s %s %lf %lf %*s", line_cell_type, &read_x, &read_y);
            //printf("the cell type is%s\n", cell_type);
                //printf("Successfully located the origin cell\n");
                }
            }

            fcell_temp.close();

            ifstream fprop_temp;
            string cell_prop_string_name(cell_prop_name);
            fprop_temp.open((cell_prop_string_name).c_str()); // open a file
            if (!fprop_temp.good()) 
                return 1; // exit if file not found
            //printf("Successfully load the cell_prop file\n");
            char buf_prop_temp[MAX_CHARS_PER_LINE];
            bool got_prop_temp = 0;
            char temp_cell_type_temp[20];
            //int garbage2;     

            fprop_temp.getline(buf_prop_temp,MAX_CHARS_PER_LINE);
            fprop_temp.getline(buf_prop_temp,MAX_CHARS_PER_LINE);
            fprop_temp.getline(buf_prop_temp,MAX_CHARS_PER_LINE);
            fprop_temp.getline(buf_prop_temp,MAX_CHARS_PER_LINE);     
        

            while(!fprop_temp.eof() && got_prop_temp==0){
                fprop_temp.getline(buf_prop_temp,MAX_CHARS_PER_LINE);
                sscanf(buf_prop_temp, "%*d %s", temp_cell_type_temp);     

                if(strcmp(temp_cell_type_temp,line_cell_type) == 0){
                    got_prop_temp = 1;
                    sscanf(buf_prop_temp, "%*d %*s %lf %lf", &temp_cell_height, &temp_cell_width);
                    //printf("Successfully located the cell properities\n");
                    //printf("and the width is %lf\n", cell_width);
                }       

            }

            fprop_temp.close();

            //adjust the center of X and Y based on the cell height and width of the cell
            if(input_ori[0]=='L' && input_ori[1] =='L'){
                norm_x = read_x + temp_cell_width/2;
                norm_y = read_y + temp_cell_height/2;
            }
            if(input_ori[0]=='L' && input_ori[1] =='R'){
                norm_x = read_x - temp_cell_width/2;
                norm_y = read_y + temp_cell_height/2;
            }
            if(input_ori[0]=='U' && input_ori[1] =='L'){
                norm_x = read_x + temp_cell_width/2;
                norm_y = read_y - temp_cell_height/2;
            }
            if(input_ori[0]=='U' && input_ori[1] =='R'){
                norm_x = read_x - temp_cell_width/2;
                norm_y = read_y - temp_cell_height/2;
            }   

        }



        //then, depending on the orientation of the laser, we need to adjust our norm x and y to the final fpga x an y


        if(stk_loc_ori[0]=='L' && stk_loc_ori[1]=='L'){
            final_x_FPGA = norm_x - stk_x/2;
            final_y_FPGA = norm_y - stk_y/2;
        }
        if(stk_loc_ori[0]=='L' && stk_loc_ori[1]=='R'){
            final_x_FPGA = norm_x + stk_x/2;
            final_y_FPGA = norm_y - stk_y/2;
        }
        if(stk_loc_ori[0]=='U' && stk_loc_ori[1]=='L'){
            final_x_FPGA = norm_x - stk_x/2;
            final_y_FPGA = norm_y + stk_y/2;
        }
        if(stk_loc_ori[0]=='U' && stk_loc_ori[1]=='R'){
            final_x_FPGA = norm_x + stk_x/2;
            final_y_FPGA = norm_y + stk_y/2;
        }


        ostringstream strings_x;
        strings_x<< final_x_FPGA;
        string final_x_string = strings_x.str();

        ostringstream strings_y;
        strings_y<< final_y_FPGA;
        string final_y_string = strings_y.str();





        string final_string = final_x_string + "," + final_y_string;



        loc_str_buffer[locstrIndex] = final_string;
        locstrIndex++;





        loc_buffer[locbuffIndex] = final_x_FPGA;
        locbuffIndex++;
        loc_buffer[locbuffIndex] = final_y_FPGA;
        locbuffIndex++;

        //loc_buffer need to be piped into fpga!

    // parse the line into blank-delimited tokens
        int n = 0; // a for-loop index
    
    // array to store memory addresses of the tokens in buf
        const char* token[MAX_TOKENS_PER_LINE] = {}; // initialize to 0
    
    // parse the line
        token[0] = strtok(buf, DELIMITER); // first token
        if (token[0]) // zero if line is blank
        {
            for (n = 1; n < MAX_TOKENS_PER_LINE; n++)
            {
                token[n] = strtok(0, DELIMITER); // subsequent tokens
                if (!token[n]) break; // no more tokens
            }
        }

    // process (print) the tokens
        for (int i = 0; i < n; i++) 
        {   
            // n = #of tokens
            //cout << "Token[" << i << "] = " << token[i] << endl;
      
            int y1 = token[i][0];
            int y2 = token[i][1];
            if(y1 > 47 && y1 < 58)  //this covers 0-9
                y1 = y1 - 48;
            else if (y1 > 64 && y1 < 71) // this covers A-F
                y1 = y1 - 55;
            if(y2 > 47 && y2 < 58)  //this covers 0-9
                y2 = y2 - 48;
            else if (y2 > 64 && y2 < 71) // this covers A-F
                y2 = y2 - 55;
            //char z = token[i][0];
            int temp1 = y1*16;
            // printf("temp1 in hex is %x", temp1);    
            //    cout<<endl;
            int temp2 = temp1 + y2;
            //printf("temp2 in hex is %x", temp2);    
            //   cout<<endl;
            unsigned char z = temp2;
            //z = z<<4;

            //printf("z in hex is %x", z);    
            //   cout<<endl;
            buffer[buffIndex] = z;
            //cout<<"buf is"<<buffer[buffIndex]<<endl;
            //printf("buf in hex is %x", buffer[buffIndex]);
            buffIndex = buffIndex + 1;
            // cout<<endl;
        }
    //cout << endl;
    }

    //reset
    if (mult_coord == 0){
        xem->ActivateTriggerIn(0x40,0);
        //PipeIn data
        xem->WriteToPipeIn(0x80, line_numbers*4, buffer);
    }
    //Design Input
     // xem->SetWireInValue(0x00, hex_1);
    //xem->UpdateWireIns();
    // xem->SetWireInValue(0x01, hex_0);
    //xem->SetWireInValue(0x00, 0x6AC1);
    //xem->UpdateWireIns();
    else{
        //int prev_checker = 1;

        string temp_output_name;
        if (user_defined_name == 1){
        printf("Please type the output file name (NO need to add the .txt extension)\n");
        
        cin>>temp_output_name;
        
        }
        if (auto_name == 1){
            time_t rawtime;
            struct tm * timeinfo;
            char time_buffer[80];

            time (&rawtime);
            timeinfo = localtime(&rawtime);

            strftime(time_buffer,80,"%d_%m_%Y_%I_%M_%S",timeinfo);
            string timestr(time_buffer);

            temp_output_name = input_data_file + user_para + timestr;
        }

        temp_output_name = temp_output_name + ".txt";
        ofstream myfile(temp_output_name.c_str());
        myfile << "output" <<endl;



        int checker = 0;
        int current_line = 0;   
        single_line_buffer[0] = buffer[current_line*4];
        single_line_buffer[1] = buffer[current_line*4+1];
        single_line_buffer[2] = buffer[current_line*4+2];
        single_line_buffer[3] = buffer[current_line*4+3];
        //current_line++; //now its at 1;

        xem->ActivateTriggerIn(0x40,0);
        xem->WriteToPipeIn(0x80, 4, single_line_buffer);
        xem->ActivateTriggerIn(0x40,1);

        while(current_line < line_numbers){ 

            
            if (checker < goal){
                xem->UpdateWireOuts();
                checker = xem->GetWireOutValue(0x21);
            }
            else {
                //now that checker equals to goal, it means laser has already scanned the whole area.
                //first pull out the data
                xem->UpdateWireOuts();
                w_address_array[current_line] = xem->GetWireOutValue(0x20);
                results_array[current_line] = new unsigned char[w_address_array[current_line]];
                xem->ReadFromPipeOut(0xA0, w_address_array[current_line]*2, results_array[current_line]);

                for (int i = 0; i < w_address_array[current_line]*2; i++){
                    printf("result[%i]: %x\n",i,results_array[current_line][i]);
                    myfile << "result number "<<current_line<<"--"<< i <<" is: " << hex << (int)(results_array[current_line][i]) << endl;
                }



                current_line++;

                single_line_buffer[0] = buffer[current_line*4];
                single_line_buffer[1] = buffer[current_line*4+1];
                single_line_buffer[2] = buffer[current_line*4+2];
                single_line_buffer[3] = buffer[current_line*4+3];
                //then reset 
                xem->ActivateTriggerIn(0x40,0);
                xem->WriteToPipeIn(0x80, 4, single_line_buffer);
                checker = 0;
                
            }   

        }
        myfile.close();

    }
    //xem->ActivateTriggerIn(0x40,1);
    //Trigger Start
    if (mult_coord == 0){
    printf("Data has been written. Select what would you like to do next\n");
    printf("1: Update Status and check input and ouput address and locations\n");
	printf("2: Send Inpulse to Start (MUST be performed before attempting to go to 3)\n");
	printf("3: Display BRAM output and output to output files\n");
    while (1) {
        guess = getchar();

        // If 1 is pressed
        // Module start
        if (guess == 0x31) 
        {
            printf("Update Status Output\n");
            xem -> UpdateWireOuts();
            input = xem->GetWireOutValue(0x20);
            output = xem->GetWireOutValue(0x21);
            printf("waddr:%u\n", input);
            printf("loc_cnt:%u\n",output);
        }
	if(guess == 0x32){
	   xem->ActivateTriggerIn(0x40,1);
	   // return(true);
    	/*
    	  printf("Update Status Output\n");
    	  xem -> UpdateWireOuts();
    	  input = xem->GetWireOutValue(0x20);
              output = xem->GetWireOutValue(0x21);
    	  len = xem->GetWireOutValue(0x23);
    	  finish = xem->GetWireOutValue(0x25);
    	  top_level_state = xem->GetWireOutValue(0x24);
    	  state = xem ->GetWireOutValue(0x26);
    	  printf("input:%x\n", input);
    	  printf("output:%x\n",output);
    	  printf("len:%u\n",len);
    	   printf("finish:%x\n", finish);
    	   printf("Top Level State: %x\n",top_level_state);
    	   printf("Circuit State: %x\n", state);*/
	}
	if(guess == 0x33){
	//future store value in txt file
        string temp_output_name;
        if (user_defined_name == 1){
        printf("Please type the output file name (NO need to add the .txt extension)\n");
        
        cin>>temp_output_name;
        
        }
        if (auto_name == 1){
            time_t rawtime;
            struct tm * timeinfo;
            char time_buffer[80];

            time (&rawtime);
            timeinfo = localtime(&rawtime);

            strftime(time_buffer,80,"%d_%m_%Y_%I_%M_%S",timeinfo);
            string timestr(time_buffer);

            temp_output_name = input_data_file + user_para + timestr;
        }

        temp_output_name = temp_output_name + ".txt";
        ofstream myfile(temp_output_name.c_str());
        myfile << "output" <<endl;

        
        //fout.open("output.txt");
	   printf("Update Output BlockRAM\n");
	   xem->ReadFromPipeOut(0xA0, input*2, results);
	  for (int i = 0; i < input*2; i++){
	    printf("result[%i]: %x\n",i,results[i]);
        myfile << "result number "<< i <<" is: " << hex << (int)(results[i]) << endl;
	  }
      myfile.close();
	  }

	
    }

    }
    return(false);
}


okCFrontPanel *initializeFPGA(std::string serial_number, std::string bitstream)
{
	okCFrontPanel *dev;
	std::string   config_filename;
    
	// Open the first XEM - try all board types.
	dev = new okCFrontPanel;
    //
	//	printf("%d\n",dev->OpenBySerial());
	if (okCFrontPanel::NoError != dev->OpenBySerial()) {
		delete dev;
		printf("Device could not be opened.  Is one connected?\n");
		return(NULL);
	}
	
	printf("Found a device: %s\n", dev->GetBoardModelString(dev->GetBoardModel()).c_str());
    
	dev->LoadDefaultPLLConfiguration();

	// Get some general information about the XEM.
	std::string str;
	printf("Device firmware version: %d.%d\n", dev->GetDeviceMajorVersion(), dev->GetDeviceMinorVersion());
	str = dev->GetSerialNumber();
	printf("Device serial number: %s\n", str.c_str());
	str = dev->GetDeviceID();
	printf("Device device ID: %s\n", str.c_str());
    
	// Download the configuration file.
	switch (dev->GetBoardModel()) {
		case okCFrontPanel::brdZEM4310:
			config_filename = ALTERA_CONFIGURATION_FILE;
			break;
		default:
			config_filename = bitstream;
			break;
	}
	//printf("%d",dev->ConfigureFPGA(config_filename));
    if (okCFrontPanel::NoError != dev->ConfigureFPGA(config_filename)) {
	//if (okCFrontPanel::NoError == dev->ConfigureFPGA(config_filename)) {
		printf("FPGA configuration failed.\n");
        //printf("FPGA configuration good.\n");
		delete dev;
		return(NULL);
	}

    else
        printf("FPGA configuration sucessful.\n");
    
	// Check for FrontPanel support in the FPGA configuration.
	if (dev->IsFrontPanelEnabled())
		printf("FrontPanel support is enabled.\n");
	else
		printf("FrontPanel support is not enabled.\n");
    
	return(dev);
}

static void printUsage(char *progname)
{
	printf("Usage: %s \n", progname);
	//printf("   bitfile -    \n");
	//printf("   infile  - input text file.\n");
	//printf("   outfile - output text file.\n");
}

// Main Program
int main(int argc, char *argv[]){
    
    char infilename[128];
	char outfilename[128];
	char dll_date[32], dll_time[32];
    char keyboard;
    int data;
    int tempHex[2];
    std::string bitstream_name;
    printf("WELCOME USER!\n");
    printf("---- Opal Kelly ---- OpalKellyPinTest v2.1 ----\n");
    if (FALSE == okFrontPanelDLL_LoadLib(NULL)) {
		printf("FrontPanel DLL could not be loaded.\n");
		return(-1);
    }
    okFrontPanelDLL_GetVersion(dll_date, dll_time);
    printf("FrontPanel DLL loaded.  Built: %s  %s\n", dll_date, dll_time);
    
    // 3 inputs
    if (argc != 2) {
		printf("Usage: ./fpga_tester_linux1 Test_Name.txt\n");
		return(-1);
	}

    int success_config = load_config();

    if (use_coord_origin == 0)
    {
        int success_loading_cell_data = read_cell_and_get_prop();
    }


    input_data_file = argv[1];

    
    okCFrontPanel *top = initializeFPGA(TOP_OK, CIRCUIT_EMU);
        // See if top is working
       if (NULL == top) {
        printf("Top FPGA(controller) cannot be initialized.\n");
        return(-1);
	}
    /*
    printf("Do you want to sent commands through UART before loading FPGA board?\n");
    printf("1: Proceed to UART with DEFAULT UART path /dev/ttyUSB1\n");
    printf("2: Proceed to UART with customized UART path.\n");
    printf("Any Other keys: SKIP UART communications\n");
    */
    //unsigned char go_uart = getchar();
    /*
    unsigned char go_uart = '3';

    if (go_uart == '1'){
        send_commands();

        printf("UART sequence completed.\n");
    }
    if (go_uart == '2'){
        send_commands();
        customized_uart = 1;
        printf("UART sequence completed.\n");
    }
    */
    
    printf("Select how do you want to name your file: \n");
    printf("1: User Defined Filename. (Everytime before an output is created, system will ask user to input a filename)\n");
    printf("2: Automatically Filename. (System will generate filenames based on the TEST_NAME + UserInput + DATE&TIME)\n");

    unsigned char output_sel = getchar();

    if (output_sel == '1'){
        user_defined_name = 1;
        auto_name = 0;
    }
    if (output_sel == '2'){
        user_defined_name = 0;
        auto_name = 1;

        printf("Please type in some parameters that will be used as part of the output filename. Press ENTER when done \n");
        cin>>user_para;

    }


     //hex portion
    bool loopStop = false;

    // while (loopStop == false) {

        
    if (performBlockDump(top) == false) {
    //if (performSCTest(dev) == false) {
		printf("IO process failed.\n");
		return(-1);
	}
     else {
		printf("IO process succeeded!\n");
	}
    top->Close();
    
    return (0);
}


