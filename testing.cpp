#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <cstring>
#include <ctime>
#include <sstream>

using namespace std;

const int MAX_CHARS_PER_LINE = 512;
const int MAX_TOKENS_PER_LINE = 4;
const char* const DELIMITER = " ";



bool stop_1 = 0;
bool stop_2 = 0;
unsigned char input_1;
int input_2;
unsigned char user_input_num;

unsigned char * buffer;

int output_file_count = 0;

int array_size = 0;
int line_numbers = 0;



int main(int argc, char *argv[]) 
{

    double test_double = 1023.432;
    //string str = boost::lexical_cast<string>(test_double);
    ostringstream strings;
    strings<<test_double;
    string test_string = strings.str();
    cout<<test_string[0]<<endl;
    cout<<test_string[4]<<endl;
	
	while(stop_1 == 0) 
	{
		cout << endl;
		printf("2: check the data\n");
		printf("3: reading file\n");
		printf("5: quit the program\n");
        printf("6: check output file names\n");
		input_1 = getchar();


		if(input_1 == 0x32)
		{
			while(stop_2 == 0)
			{
				cout << endl;
				printf("1-8: check the corresponding data\n");
				printf("9: done checking the data\n");
				cin >> input_2;
				if(input_2 == 9) {
					cout << "done with checking! now back to the outer loop" << endl;
					stop_2 = 1;
				}
				else {
					cout << "data value is " << hex << (int)(buffer[input_2 - 1]) << endl;
					cout << endl;
				}

				


			}
			

		}

		if(input_1 == 0x33)
		{
			//unsigned char buffer[40];
    		int buffIndex = 0;
            ifstream fcheck;
            fcheck.open("test_name.txt");
            
            char check_return;
            //int i = 0;
            line_numbers = 0;
            while (fcheck.get(check_return)){
                if (check_return == '\n'){
                    ++line_numbers;
                }
            }
            fcheck.close();
            //cout<<"line number is: "<<line_numbers<<endl;

            buffer = new unsigned char [line_numbers*4];
            //buffer = (unsigned char*)malloc(sizeof(unsigned char)*line_numbers*4);
            int stop_reading = line_numbers;

    		ifstream fin;
    		fin.open("test_name.txt"); // open a file
            //
    		if (!fin.good()) 
        		return 1; // exit if file not found
  


			// read each line of the file
    		while (stop_reading>0)
    		{
                stop_reading--;
    			// read an entire line into memory
        		char buf[MAX_CHARS_PER_LINE];
        		fin.getline(buf, MAX_CHARS_PER_LINE);
                
                double loc;
                sscanf(buf, "%*s %*s %*s %*s %lf", &loc);
                cout<<"HAHAHA"<<loc<<endl;



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
                    //cout<<"here?"<<endl;
            		// n = #of tokens
            		//cout << "Token[" << i << "] = " << token[i] << endl;
      
            		int y1 = token[i][0];
            		int y2 = token[i][1];
                    //double test = token[i][2];
                    //cout<<test<<endl;
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
            		//data_array[buffIndex] = buffer[buffIndex];
            		//cout << hex << int(z) << endl;
            		//cout<<"buf is"<<buffer[buffIndex]<<endl;
            		//printf("buf in hex is %x", buffer[buffIndex]);
            		buffIndex = buffIndex + 1;
            		// cout<<endl;
        		}
    			//cout << endl;
                //cout<<"out here"<<endl;
    		}
            //cout<<"here"<<endl;
            printf("size is%d\n", line_numbers*4);
            fin.close();

            ofstream fout;
            fout.open("output.txt");
            char whitespace = 'a';
            for (int j=0; j<line_numbers*4; j++){
                //fout.write((unsigned char*)(&buffer[j]), 1);
                //fout.write(reinterpret_cast<char*>(&whitespace),1);
                //fout << "data value is " << hex << (int)(buffer[j]) << endl;
                //cout << "data value is " << hex << (int)(buffer[j]) << endl;
            }

		}

		if(input_1 == 0x35) 
		{
			stop_1 = 1;
		}



        if(input_1 == 'Y') 
        {

            /*
            for (int i = 0; i<5; i++){


                //std::string con2("output" + std::string(output_file_count) + ".txt");
                ofstream myfile("file_no_" + std::to_string(output_file_count) + ".txt");
                myfile << "testing testing testing" <<endl;
                myfile.close();
                output_file_count++;
            }
            */
            time_t rawtime;
            struct tm * timeinfo;
            char time_buffer[80];

            time (&rawtime);
            timeinfo = localtime(&rawtime);

            strftime(time_buffer,80,"%d_%m_%Y_%I_%M_%S",timeinfo);
            string timestr(time_buffer);

            std::cout << timestr;


            printf("type the output file name\n");
            string temp_output_name;
            //cin>>temp_output_name;
            temp_output_name = timestr;
            temp_output_name = temp_output_name + ".txt";
            //ofstream myfile("file_no_" + std::to_string(temp_output_name) + ".txt");
            ofstream myfile(temp_output_name.c_str());
                myfile << "testing testing testing" <<endl;
                myfile.close();
        }

	}





	return (0);

}

