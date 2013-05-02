#include <windows.h>
#include <stdio.h>
#include <tchar.h>

/*for expedited in-line debugging - JH */
#include <iostream> /* might need this - JH */
#include <conio.h> /* for _getch() - JH */
#include <fstream>
#include <istream>
#include <string>

using namespace std;


//Global Variables

DCB dcb;
HANDLE hCom;
char *pcCommPort = "COM34";
bool message = false;

//Weight value recieved when weighing
float weightResult;

struct scaleData {
    float weight;
    char space[1];
    char unit[5];
    char stabilityIndicator;
    char legend[10];
    char CR;
    char LF;
};

//Data collection variables
string foodName[10];
int nFoodItems;
int subjectID;
float foodWeight[10];
char anyKey;
string food;

/* if we are going to use this program to do everything we need more functions to do -
1) User puts plate on scale and zeros the scale manually

2) User manually enters the following information:

-name main entree

-number of people eating

-Numer of food items
-- Food Item 1 name
-- Food Item 2 name
-- Food Item 3 name

3) Scale askes for ID number of first person
4) First person enters ID number
5) Scale asks for first food item
6) User places Food Item 1 on plate
7) Scale records wight and asks for Food Item 2
...
...
This continues until all food items are weighed.
8) Scale takes a photograph of the plate with web camera.

9) Scale asks for the second user to enter information
*/

int sendCommand(char command,DWORD dwNumBytesWritten,char sendBuffer[])
{
//Send Comma3nd to the scale
printf("\n\n\n Start writting ! \n");

   WriteFile (hCom,               // Port handle
              sendBuffer,         // Pointer to the data to write
              sizeof(sendBuffer), // Number of bytes to write
              &dwNumBytesWritten, // Pointer to the number of bytes written
              NULL                // Must be NULL for Windows Embedded CE
              );


            printf("\n Bytes Written to the terminal ");
            for( int j=0; j< sizeof(sendBuffer); j++)
                 printf("%c",sendBuffer[j]);


   printf("\n Byte length %d \n", dwNumBytesWritten);
   _getch();
   return 0;
}

//struct scaleData readCommand(struct scaleData scale2)
float readCommand(scaleData scale2)
{
   char sendBuffer[5];
   char recieveBuffer[100] = {};
   DWORD dwNumBytesWritten;
   DWORD dwBytesTransferred;


   char weight[10] = "defWt";
   char space[1] = {' '};
   char unit[5] = "g";
   char stabilityIndicator = '?';
   char legend[10] = "defLegend";
   char CR;
   char LF;

   printf("\n\n\n Reading Scale:\n");

   message =  ReadFile (hCom,       // Port handle
            recieveBuffer,          // Pointer to data to read
            sizeof(recieveBuffer),  //dwNumBytesWritten,     // Number of bytes to read
            &dwBytesTransferred,    // Pointer to number of bytes read
            NULL                    // Must be NULL for Windows Embeddded CE
            );

      //View and Parse the incoming Message
    //weight - 10 char (right justified)
    //space - 1 character
    //unit - 5 characters max (left justified)
    //stability indicatior ('?' if unstable, blank if stable)
    //space - 1 character
    //legend - 10 character (TOTAL,hh:mm:ss(time interval), etc)
    //CR - 1 character
    //LF - 1 character

   //Show what is contained in the recieving Buffer
   cout << recieveBuffer;

   Sleep(100);
    if(message){
      
 
    //Read in 10 characters for weight
    for (int k=0; k<10; k++) {
        weight[k] = recieveBuffer[k];
        printf("weight: %c \n",weight[k]);
    }
   
    scale2.weight = atof(weight);
    printf("weight %f\n",scale2.weight);
 
  //Space
  printf(" ");

  //Read in Unit
  for (int k=11; k<17; k++) {
      unit[k] = recieveBuffer[k];
      scale2.unit[k] = unit[k];
  }
 
  //Read in stability indicator
  stabilityIndicator = recieveBuffer[18];
 
  //If there is a stability indicator, then print it otherwise print a space
  if(stabilityIndicator == '?')
  {
  scale2.stabilityIndicator = stabilityIndicator;
  }
  else
  {
      scale2.stabilityIndicator = ' ';
  }
      message = false;   
    }
  
   
   
  //End of the message Parsing
    printf("weight: %f\n",scale2.weight);
    return scale2.weight;
}



DWORD openScale() /* going to assume this works since can't test it - JH */
{
  hCom =  CreateFile("\\\\.\\COM34", //pcCommPort
  GENERIC_READ | GENERIC_WRITE,
  0, //Open with exclusive access
  0, //No security attributes
  OPEN_EXISTING, //must use open existing
  0, //Not overalpped I/O
  0  //hTemplate 0 for comm devices
  );

   if (hCom == INVALID_HANDLE_VALUE)
   {
      // Handle the error.
       printf ("in openScale - CreateFile failed with error %d.\n", GetLastError());
    }
   _getch();
   return (GetLastError()); /* if there is an error return it - JH */
}


int main(int argc, char *argv[])

{
    /* Open a file for writing */
    ofstream outputFile;
    outputFile.open("scaleOutput.txt");

   DWORD scaleState = 0;
   BOOL fSuccess = false;
   BOOL fConnect = true;
   struct scaleData scale;
 
   //Set Default scale value
   scale.weight = 0.0;
 
   //Open Communication Stream from Scale
   scaleState = openScale();
  
   // Build on the current configuration, and skip setting the size
   // of the input and output buffers with SetupComm.
   fSuccess = GetCommState(hCom, &dcb);

   if (!fSuccess)
   {
     // Handle the error.
      printf ("in main 1st if - GetCommState failed with error %d.\n", GetLastError());
    /* ? - stub some stuff here to init vars to defaults and test the parse code, without the scale - JH*/
   }

   //Set the communication as per the Oharus NavigatorXT scale setings
   dcb.BaudRate = 2400;          // set the baud rate
   dcb.ByteSize = 8;             // data size, xmit, and rcv
   dcb.Parity = NOPARITY;        // no parity bit
   dcb.StopBits = ONESTOPBIT;    // one stop bit

   fSuccess = SetCommState(hCom, &dcb);

   if (!fSuccess)
   {
      // Handle the error.
      printf ("in main 2nd if - SetCommState failed with error %d.\n", GetLastError());
   }
   printf ("Serial port %s successfully reconfigured.\n", pcCommPort);
   printf("\n");
 

   //Begin User Data Entry
   cout << "HI-SEAS FOOD WEIGHING SYSTEM" << endl;
   cout << "Enter your subject ID" << endl;
   cin >> subjectID;
   cout << "Enter the number of food items: " << endl;
   cin >> nFoodItems;
  
      // Collect the names of each food item being weighed
      // and stores in string array
       for(int i = 0; i< nFoodItems; i++)
       {
           cout << "Enter name of Food Item  " << (i+1) << endl;
           cin >> foodName[i];
        // getline(cin,foodName[i]);
       }
  
       //Lets start weighing the food items
       for(int i=0; i < nFoodItems; i++)
       {
       cout << subjectID << " Place food item " << (i+1) << " on scale and press ENTER:" << endl;
        system("pause");
       cout << "Press ENTER when scale stable" << endl;
        system("pause");
       cin.get();
   
       //Read the scale
        weightResult = readCommand(scale);
       
       //Save the weight value;
       foodWeight[i] = weightResult;
       }

       //Print out results
       cout << endl << endl << endl;
       cout << "Results: " << endl;
       for(int i=0; i< nFoodItems; i++)
       {
           cout << foodName[i] << " " << foodWeight[i] << endl;
       }

  // Close serial port and file, then Exit from Program
   CloseHandle(hCom);
   outputFile.close();

   int num;
   scanf("%d", &num);
   _getch();
}