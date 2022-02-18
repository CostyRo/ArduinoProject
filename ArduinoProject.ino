#include <MQ2.h>
#include <DHT.h>
#include <Keypad.h>
#include <LiquidCrystal.h>
// introduce the libraries for hardware interaction

#define LCD_PINS 12,11,5,4,3,2
#define GAS_SENZOR_PIN 13
#define HUM_TEMP_SENZOR_PIN 7
#define KEYPAD_ROW_PINS (byte[]){19,18,17,16}
#define KEYPAD_COLUMN_PINS (byte[]){15,14,8,9}
#define LCD_CONTRAST_PIN 6
#define BUTTON_PIN 10
// macros for pins

#define HUM_TEMP_SENZOR_TYPE DHT11
#define KEYPAD_TYPE 4,4
#define LCD_SCREEN_TYPE 16,2
#define LCD_CONTRAST_MAX 100
#define LCD_NEXT_ROW 0,1
// macros for special constants

#define HUM String(temp_hum_senzor.readHumidity())+String('%')
#define TEMP String(temp_hum_senzor.readTemperature())
#define TEMP_SIGN String((char)223)+String('C')
#define LPG String((int)gas_senzor.readLPG())
#define CO String((int)gas_senzor.readCO())
#define SMOKE String((int)gas_senzor.readSmoke())
#define END_ELEM_EQUATION(i) equation[strlen(equation)-i]
// macros for utilities
// a macro for printing the humidity and one for temperature
// a macro for printing the the temeperature notation
// a macro for printing the LPG, and one for CO and one for smoke
// and a macro for returning the last i elemenet of the equation

bool previous_button_phase=false;
// variable that stores the last stage of the button

bool default_activity=true;
// variable that stores the default activity of the aplication

bool calculator_on=false;
// variable that stores if calculator is on

char equation[33]={NULL};
// variable that stores the mathematical equation

char operators[16]={NULL};
// variable that stores the operators of the equation

struct{
  double elements[16];
  byte lenght=0;
}numbers;
// struct with an array and a length
// used for storing the numbers of the equation

LiquidCrystal lcd(LCD_PINS);
MQ2 gas_senzor(GAS_SENZOR_PIN);
DHT temp_hum_senzor(HUM_TEMP_SENZOR_PIN,HUM_TEMP_SENZOR_TYPE);
Keypad matrix_keypad(makeKeymap(((char[][4]){"123+","456-","789*",".0=%"})),
                    KEYPAD_ROW_PINS,KEYPAD_COLUMN_PINS,KEYPAD_TYPE);
// objects for the lcd screen, the humidity and temperature senzor
// and the matrix keypad

void setup(){
  prepare_screen();
  // prepare the lcd screen

  prepare_senzors();
  // prepare the humidity and temperature senzor
}

void loop(){
  track_button();
  // track the state of the button

  calculator();
  // execute the commands of the calculator
}

// function that turns on and sets the contrast of the lcd screen
void prepare_screen(){
  lcd.begin(LCD_SCREEN_TYPE);
  // start the lcd screen

  analogWrite(LCD_CONTRAST_PIN,LCD_CONTRAST_MAX);
  // set the contrast of the lcd screen to max contrast
}

// function that prepares the humidity and temperature senzor
void prepare_senzors(){
  temp_hum_senzor.begin();
  // start the humidity and temperature senzor

  gas_senzor.begin();
  // start the gas senzor

  pinMode(BUTTON_PIN,INPUT);
  // set the input to button's pin

  print_hum_temp();
  // print the humidity and the temperature
}

// function that prints the humidity and the temperature
void print_hum_temp(){
  lcd.clear();
  // clear the screen

  lcd.home();
  // set the cursor to the start of the lcd scren

  lcd.print(String("Humidity: ")+HUM);
  // print the humidity

  lcd.setCursor(LCD_NEXT_ROW);
  // set the cursor to the next row

  lcd.print(String("Temp: ")+TEMP+TEMP_SIGN);
  // print the temperature
}

// function that tracks the state of the button
void track_button(){
  bool current_button_phase=digitalRead(BUTTON_PIN);
  // read the stage of the button

  if(!previous_button_phase && current_button_phase) set_activity();
  // if the button is pressed right now
  // change the activity and execute the commands

  previous_button_phase=current_button_phase;
  // set the old value of the button to the new value of the button
}

// function that changes and starts the activity of the button
void set_activity(){
  if(!default_activity || calculator_on) print_hum_temp();
  else print_gas_info();
  // execute the opposite of the current activity

  switch_activity();
  // switch the activity
}

// function that switches the activity of the button
void switch_activity(){
  default_activity=!default_activity;
  // switch the activity of the button

  reset();
  // reset the values
}

// function that resets the values
void reset(){
  calculator_on=false;
  // set the variable that tracks state of the calculator to false

  numbers.lenght=0;
  // set the length of the array to 0

  memset(equation,0,sizeof(equation));
  memset(operators,0,sizeof(operators));
  memset(numbers.elements,0,sizeof(numbers.elements));
  // set the memory of equation, operators and numbers to 0
}

// function that prints the LPG, the CO and the smoke
void print_gas_info(){
  lcd.clear();
  // clear the screen

  lcd.home();
  // set the cursor to the start of the lcd scren

  lcd.print(String("LPG: ")+LPG+String(" CO: ")+CO);
  // print the LPG and the CO

  lcd.setCursor(LCD_NEXT_ROW);
  // set the cursor to the next row

  lcd.print(String("SMOKE: ")+SMOKE+String(" PPM"));
  // print the smoke
}

// function that actions the calculator
void calculator(){
  char button=matrix_keypad.getKey();
  // get the pressed key

  // if the key isn't NULL
  if(button){
    calculator_on=true;
    // set the variable that tracks state of the calculator to true

    new_equation_screen();
    // clear the screen if a new equation starts

    execute_button_command(button);
    // make the commands for every specific button
  }
}

// function that clears the screen when a new equation starts
void new_equation_screen(){
  if(!strlen(equation)) lcd.clear();
  // clear the screen if the equation don't started
}

// function for commands for every specific button
void execute_button_command(char button){
  // skip if the character is invalid
  if(
     (!strlen(equation) && strchr("+*%.=",button)) || 
     (strchr("+*%.=",button) && strchr("+-*%.=",END_ELEM_EQUATION(1))) || 
     (button=='-' && END_ELEM_EQUATION(1)=='-' && END_ELEM_EQUATION(2)=='-') ||
     (strcmp(equation,"-")==0 && button=='-'));
  // else if the button is = do the calculations
  else if(button=='='){
    break_equation();
    // break the equation in numbers and operators

    print_result();
    // print the result to the screen
  }
  // check if the equation doesn't tocuh the limit
  else if(strlen(equation)!=32 && (!strchr("+-*%.",button) || strlen(equation)!=31)){
    append(equation,button);
    // add the key to the equation

    lcd.setCursor((strlen(equation)-1)%16,(strlen(equation)-1)/16);
    // set cursor to the new location

    lcd.print(button);
    // print the button
  }
}

// function that breaks the equation in numbers and operators
void break_equation(){
  char number[32]={NULL};
  // define an empty char array for the number

  byte equation_length=strlen(equation);
  // variable that stores the length of the equation

  // browse the entire equation
  for(byte i=0;i<equation_length;i++){
    // if the current character is an operator
    if(strchr("+-*%",equation[i])){
      // if the last character was also an operator add the sign to the number
      if(strchr("+-*%",equation[i-1])) append(number,equation[i]);
      else{
        append(operators,equation[i]);
        add_number(number);
        memset(number,0,sizeof(number));
      }
      // otherwise add the operator and the number to lists and reset the number
    }
    else append(number,equation[i]);
    // otherwise continue construction of the number
  }
  add_number(number);
  // add the last number
}

// function that appendes a char to a char array
void append(char* string,char character){
  string[strlen(string)]=character;
  // set the value of last element+1 to the given character

  string[strlen(string)+1]=NULL;
  // set the next element to NULL
}

// function that converts the char array to a double and add it in the numbers array
void add_number(char* s){
  numbers.elements[numbers.lenght++]=atof(s);
  // convert the char array to a double and add it in the array
}

// function that prints the result
void print_result(){
  lcd.clear();
  // clear the screen

  lcd.home();
  // set the cursor to the start of the lcd scren

  lcd.print("Result is: ");
  // prepare printing the result

  lcd.setCursor(LCD_NEXT_ROW);
  // set the cursor to the next row

  lcd.print(calculate());
  // calculate and print the result

  reset();
  // reset the values
}

// function that calculates and returns the result
double calculate(){
  byte operators_length=strlen(operators);

  // browse the array of operators
  for(byte i=0;i<operators_length;i++){
    // if the current operator is * or %
    if(strchr("*%",operators[i])){
      if(operators[i]=='*') numbers.elements[i]*=numbers.elements[i+1];
      else numbers.elements[i]/=numbers.elements[i+1];
      // multiply the numbers if operator is * else divide them

      delete_number(i+1);
      // rearrange the array

      strcpy(operators+i,operators+i+1);
      // delete the operator from the char array

      recalculate_index_lenght(i,operators_length);
      // repair the values of index and length
    }
  }

  // browse again the array of operators
  for(byte i=0;i<operators_length;i++){
    // if the current operator is +, add the numbers
    if(operators[i]=='+') numbers.elements[i]+=numbers.elements[i+1];
    // else if the current operator is -, substract the numbers
    else if(operators[i]=='-') numbers.elements[i]-=numbers.elements[i+1];

    delete_number(i+1);
    // rearrange the array

    strcpy(operators+i,operators+i+1);
    // delete the operator from the char array

    recalculate_index_lenght(i,operators_length);
    // repair the values of index and length
  }

  return numbers.elements[0];
  // return the result
}

// function that rearranges the array
void delete_number(byte index){
  for(byte i=index;i<numbers.lenght-1;i++) numbers.elements[i]=numbers.elements[i+1];
  // delete a number from array

  numbers.lenght--;
  // decrement the length of the array
}

// function that keeps the correct position for the arrat
void recalculate_index_lenght(byte &index,byte &operators_lenght){
  index--;
  operators_lenght--;
  // decrement the index and the lenght by one
} 
