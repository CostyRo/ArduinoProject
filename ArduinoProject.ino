#include <DHT.h>
#include <Keypad.h>
#include <LiquidCrystal.h>
// introduce the libraries for hardware interaction

char calculatorKeys[4][4]={{'1','2','3','+'},{'4','5','6','-'},{'7','8','9','*'},{'.','0','=','%'}};
// set the keys for the matrix calculator

short int previous_button_phase=1;
// variables that stores the last stage of the pushbutton

bool default_activity=true;
// variable that store the default_activity of the aplication

short int location1=0;
short int location2=0;
// variables that stores the location for the lcd screen

bool in_loop=false;
// variable that store if the loop is on

bool fin=false;
// variable that store if is the loop is done

bool calculator_on=false;
// variable that store if the loop is on for more that one time

bool valid=true;
// variable that store if the key is valid

char equation[34]="";
// variable that store the mathematical equation

char last_char=' ';
char penultimate_char=' ';
// variables that stores the last 2 characters of equation

char operators[16]="";
// variable that store the operators of the equation

double numbers[16];
// array that stores the numbers from the equation

short int arrayLength=0;
// variable that store the lenght of the array

DHT tempsenz(7,DHT11);
LiquidCrystal lcd(12,11,5,4,3,2);
Keypad customKeypad(makeKeymap(calculatorKeys),
                   (byte[]){19,18,17,16},
                   (byte[]){15,14,8,9},
                   4,4);
// objects for the humidity and temperature senzor, the lcd screen, and the matrix keypad

void setup(){
  prepare_screen();
  // prepare the lcd screen

  prepare_senzor();
  // prepare the humidity and temperature senzor

  print_hum_temp();
  // print the humidity and the temperature
}

void loop(){
  pushButton();
  // execute commands of the pushbutton

  calculator();
  // execute the commands of the calculator
}

// function that turn on and set the contrast of the lcd screen
void prepare_screen(){
  lcd.begin(16,2);
  // start the lcd screen

  analogWrite(6,100);
  // set the contrast of the lcd screen to 100
}

// function that prepare the humidity and temperature senzor
void prepare_senzor(){
  tempsenz.begin();
  // start the humidity and temperature senzor

  pinMode(10,INPUT);
  // set the input to pin 10
}

// function that print the humidity and the temperature
void print_hum_temp(){
  lcd.clear();
  // clear the screen

  lcd.setCursor(0,0);
  // set the cursor to the start of the lcd scren

  lcd.print("Humidity: ");
  // print "Humidity: "

  lcd.setCursor(9,0);
  // set the cursor to the next position

  lcd.print(tempsenz.readHumidity());
  // read and print the value of humidity

  lcd.setCursor(13,0);
  // set the cursor to the next position

  lcd.print("%");
  // print "%"

  lcd.setCursor(0,1);
  // set the cursor to the next row

  lcd.print("Temp: ");
  // print "Temp: "

  lcd.setCursor(5,1);
  // set the cursor to the next position

  lcd.print(tempsenz.readTemperature());
  // read and print the value of tempC

  lcd.setCursor(10,1);
  // set the cursor to the next position

  lcd.print((char)223);
  // print the degree sign

  lcd.setCursor(11,1);
  // set the cursor to the next position

  lcd.print("C");
  // print "C"
}

// function for the pushbutton
void pushButton(){
  short int current_button_phase=digitalRead(10);
  // read the stage of the pushbutton

  if(!previous_button_phase && current_button_phase) set_new_default_activity();
  // if the button is pressed right now
  // change the default_activity and execute the commands

  previous_button_phase=current_button_phase;
  // set the old value of the button to the new value of the button
}

// function for changing the default_activity of the pushbutton and executing the commands
void set_new_default_activity(){
  // find the current default_activity
  if(!default_activity || calculator_on) print_hum_temp();
  // print the humidity and the temperature
  else gas_senzor();

  switch_activity();
  // switch the default_activity
}

// function that switch the default_activity of the pushbutton
void switch_activity(){
  default_activity=!default_activity;
  // switch the default_activity of the pushbutton

  reset();
  // reset the values
}

// function to reset the values
void reset(){
  location1=0;
  location2=0;
  // reset the values of the locations

  calculator_on=false;
  in_loop=false;
  // set the variables that are true when loop is on to false

  last_char=' ';
  penultimate_char=' ';
  // reset the last characters

  arrayLength=0;
  // set the length of the array to 0

  setToZero();
  // fill the array with 0s

  memset(&operators[0],0,sizeof(operators));
  memset(&equation[0],0,sizeof(equation));
  // set the memory of operators and of equation to NULL
}

void gas_senzor(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("TODO");
}

// function that fill the array to 0s
void setToZero(){
  for(short int i=0;i<arrayLength;i++) numbers[i]=0;
  // loop the array and set the current element to 0
}

// function for the pushbutton
void calculator(){
  char button=customKeypad.getKey();
  // get the pressed key

  // if the key isn't NULL
  if(button){
    calculator_on=true;
    // set the varible that store if is first time to true

    maybeNewEquation();
    // check if is a new equation

    continueTheEquation();
    // check if equation continues

    actionForButton(button);
    // make the commands for every specific button

    setNewCharacters(button);
    // set the new characters

    valid=true;
    // validate the move
  }
}

// function for checking if is a new equation
void maybeNewEquation(){
  // if is a new equation
  if(fin){
    lcd.clear();
    // clear the screen

    fin=!fin;
    // switch the value of fin
  }
}

// function for the continuation of equation
void continueTheEquation()
{
  // if the equation don't started
  if(!in_loop){
    lcd.clear();
    // clear the screen

    in_loop=!in_loop;
    // switch the value of in_loop
  }
}

// function for commands for every specific button
void actionForButton(char button)
{
  // if the character is invalid, set valid to false
  if((strcmp(equation,"")==0 && strchr("+*%.=0",button)) || 
  (strchr("+*%.=",button) && strchr("+*%.=",last_char)) || 
  (strchr("+-*%.=",button) && strchr("+-*%.=",last_char) && strchr("+-*%.=",penultimate_char)) || 
  (strcmp(equation,"-")==0 && button=='-'))
    valid=false;
  // else if the button is '=' do the calculations
  else if(button=='='){
    // if the last character isn't again a '='
    if(last_char!='='){
      fin=true;
      // set the fin to true

      breakTheEquation();
      // break the equation in numbers and operators

      printTheResult();
      // print the result to the screen
    }
  }
  else{
    // if the equation in the screen
    if(location1!=-1 && location2!=-1){
      // check if the equation doesn't tocuh the limit
      if(strlen(equation)!=32 && (!strchr("+-*%.",button) || strlen(equation)!=31)){
        append(equation,button);
        // add the key to the equation

        lcd.setCursor(location1,location2);
        // set cursor to the current location

        lcd.print(button);
        // print the button

        setLocation();
        // set the new location
      }
    }
  }
}

// function that break the equation in numbers and operators
void breakTheEquation(){
  char charBuffer[32]={0};
  // set the memory of the buffer to NULL

  short int lastSign=0;
  // varible for the last place of a operator

  char lastOperator=' ';
  // varible for the last operator

  // pass the entire equation without first position
  for(short int i=1;i<strlen(equation);i++){
    // if the current character is an operator
    if(strchr("+-*%",equation[i])){
      if(!strchr("+-*%",equation[i-1])) append(operators,equation[i]);
      // if the last chracter wasn't an operator, add the operator to list with operators

      strncpy(charBuffer,equation,i);
      // put the char array in the buffer

      if(lastSign!=0) strcpy(charBuffer,charBuffer+lastSign+1);
      else if(!strchr("+-*%",lastOperator)) strcpy(charBuffer,charBuffer+lastSign);
      // crop the buffer to get the number

      if(strcmp(charBuffer,"")) addNumber(charBuffer);
      // if number isn't "" tranform them to a double and add them to the array with numbers

      lastSign=i;
      // set lastSign to the current position of the equation

      lastOperator=equation[i];
      // set the lastOperator to the last operator
    }
  }
  strcpy(charBuffer,equation);
  // put the char array in the buffer

  // if the last operator doesn't exist do nothing
  if(lastOperator==' ');
  else if(lastOperator!='-' || 
  (lastOperator=='-' && strchr("+-*%",equation[lastSign-1])==0)) strcpy(charBuffer,charBuffer+lastSign+1);
  else strcpy(charBuffer,charBuffer+lastSign);
  // get the last number

  addNumber(charBuffer);
  // tranform the number to a double and add them to the array with numbers
}

// function for appending a char to a char array
void append(char* string,char character){
  string[strlen(string)]=character;
  // set the value of last element+1 to the given character

  string[strlen(string)+1]='\0';
  // set the next element to NULL
}

// function that transform the char array to a double and add them in the array with numbers
void addNumber(char* s){
  double number=atof(s);
  // transform the char array to a double

  numbers[arrayLength++]=number;
  // add them in the array
}

// function that print the result
void printTheResult(){
  lcd.clear();
  // clear the screen

  lcd.setCursor(0,0);
  // set the cursor to the start of the lcd scren

  lcd.print("Result is: ");
  // print "Result is: "

  lcd.setCursor(0,1);
  // set the cursor to the next row

  calculate();
  // calculate the result

  lcd.print(numbers[0]);
  // print the result

  reset();
  // reset the values
}

// function that calculate the result
void calculate(){
  // pass the array of operators
  for(short int i=0;i<strlen(operators);i++){
    // if the current operator is '*'
    if(operators[i]=='*'){
      numbers[i]*=numbers[i+1];
      // multiply the numbers

      rearange(i+1);
      // rearange the array

      strcpy(operators+i,operators+i+1);
      // delete the operator from the array

      keepPosition(i);
      // keep the position of the array with operators
    }
    // else if the current operator is '%'
    else if(operators[i]=='%'){
      numbers[i]/=numbers[i+1];
      // divide the numbers

      rearange(i+1);
      // rearange the array

      strcpy(operators+i,operators+i+1);
      // delete the operator from the array

      keepPosition(i);
      // keep the position of the array with operators
    }
    else continue;
    // else continue to pass the array
  }

  // pass again the array of operators
  for(short int i=0;i<strlen(operators);i++){
    // if the current operator is '+'
    if(operators[i]=='+'){
      numbers[i]+=numbers[i+1];
      // add the numbers

      rearange(i+1);
      // rearange the array

      strcpy(operators+i,operators+i+1);
      // delete the operator from the array

      keepPosition(i);
      // keep the position of the array with operators
    }
    // else if the current operator is '-'
    else if(operators[i]=='-'){
      numbers[i]-=numbers[i+1];
      // substract the numbers

      rearange(i+1);
      // rearange the array

      strcpy(operators+i,operators+i+1);
      // delete the operator from the array

      keepPosition(i);
      // keep the position of the array with operators
    }
  }
}

// function that rearange the array
void rearange(short int pos){
  for(short int i=pos;i<arrayLength-1;i++) numbers[i]=numbers[i+1];
  // delete a number from array

  arrayLength--;
  // decrement the length of the array
}

// function that keep the position
void keepPosition(short int &pos){
  pos--;
  // decrement the positon
}

// function that set the new location
void setLocation(){
  location1++;
  // increment the first location

  // if the first location toch the limit
  if(location1>15){
    location1=0;
    // reset the location

    location2++;
    // increment the value of second location
  }

  // if the first location overcome the limit
  if(location2>1){
    location1=-1;
    location2=-1;
    // set the locations to -1
  }
}

// function that set the new characters
void setNewCharacters(char button){
  // if the key was valid
  if(valid){
    penultimate_char=last_char;
    // penultimate_char become last_char

    last_char=button;
    // last_char become the last pressed the key
  }
}
