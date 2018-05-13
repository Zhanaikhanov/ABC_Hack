#include <EEPROM.h>
#include <SPI.h>
#include <MFRC522.h>
#define RST_PIN         9          // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

#include <Keypad.h>

int *access_token[36];
void setup() {
  Serial.begin(9600);   // Initialize serial communications with the PC

  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
//  A0 analogwrite
//8,7,6 cols
//5 4 3 2 rows 
  for (int i=0;i<36;i++)
  {
    access_token[i] = EEPROM.read(i);
  }

}

//==================================================================================

char keymap[4][4] = { {'1','2','3','A'},
                  {'4','5','6', 'B'},
                  {'7','8','9', 'C'},
                  {'*','0','#', 'D'} };
byte rowPins[4] = {A0,8,7,6};
byte colPins[4]= {A2,A3,4,5};
Keypad myKeypad= Keypad(makeKeymap(keymap), rowPins, colPins, 4, 4);


int* pin_entered[4] = {(int)'-',(int)'-',(int)'-',(int)'-'};
int cur_pos_pin = 0;
int* correct_pin[4] = {(int)'5',(int)'4',(int)'2',(int)'9'};

//PIN=5429

bool check_pin(int* buf[])
{   
  

  
  for (int i=0; i<4; i++)
  {
    if (correct_pin[i]!=buf[i])
      return false;
  }

  return true;
}

double last_time = 0;
bool locked = true;


int len_token = 36;                                                                             


void serialize_data_to_EEPROM()
{
  for (int i=0;i<EEPROM.length();i++)
  {
    EEPROM.write(i,0);
  }
 
  for (int i=0;i<36;i++)
  {
    EEPROM.write(i,access_token[i]);
  }   
}




bool ready_send = false;

bool new_token_available()
{
  char start = '=';
  if (Serial.available())
  {
    start = Serial.read();
  }
  if (start == '2')
  {
    return true;  
  }

  return false;
}



bool service = true;

void unlock_pc()
{
  ready_send = true;
  service = false;
}
int type = 0;

void loop() {
  // put your main code here, to run repeatedly:


//========================PIN====================
    
    


   if (type == 2)
   {
    if ((millis()-last_time)>10000 and last_time != 0)
    {
      ready_send = false;
      service = true;
      last_time = 0;
    }
   }

   if (ready_send and !service)
   {
      char data = '=';
      if (Serial.available())
      {
        delay(10);
        data = Serial.read();
      }
      if (data == '1')
      {
        for (int i=0; i<36; i++)
        {
          char y = (char)access_token[i];
          Serial.print(y);
        }
        Serial.println("");

   
        
      }else if(data == '2')
      {
        delay(10);
        for (int i=0; i<36;)
        {
          delay(10);
          char t = Serial.read();
          access_token[i++] = t;
        }
        serialize_data_to_EEPROM(); 

      }
  }


    if (Serial.available())
    {
      delay(10);
      int cmd = Serial.read();
      
      if (cmd == (int)'L')
      {
        cur_pos_pin = 0;
        for (int i=0;i<4;i++)
        {
          pin_entered[i] = (int)'-';
        }
        ready_send = false;
        service = true;
      }
      else if (!ready_send or service)
      {
        Serial.println("NO");
      }
    }



    
    int keypressed = myKeypad.getKey();   
    if (keypressed != NO_KEY)
    {
      if (keypressed == (int)'C' or cur_pos_pin == 4)
      {
        cur_pos_pin = 0;
        for (int i=0;i<4;i++)
        {
          pin_entered[i] = (int)'-';
        }
      }else{
        pin_entered[cur_pos_pin++] = keypressed;      
      }

      
    }
//    Serial.print(check_pin(pin_entered));
//    Serial.println(" ");
    if (check_pin( pin_entered ))
    {
      cur_pos_pin = 0;
      for (int i=0;i<4;i++)
      {
        pin_entered[i] = (int)'-';
      }
      unlock_pc();
      type = 1;
    }


    
    if ( ! mfrc522.PICC_IsNewCardPresent())
        return;
    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
        return;
        
    if (check_rfid(mfrc522.uid.uidByte, mfrc522.uid.size))
    {     
      type = 2;
      unlock_pc();
      last_time = millis();
    }

}
//==========================END=LOOP=====================================


//==================================================================================


//==================================================================================
bool check_rfid(byte *buffer, byte bufferSize)
{
  byte *password[4] = {0x36,0xC4, 0xB7, 0xAC};
  //36 C4 B7 AC
  //15 DB D7 2B
  if (bufferSize!=4)
  {
    return false; 
  }
  int equal = 0;
  for (int i = 0; i<4; i++)
  {
  if (buffer[i] == password[i])
    {
      equal++;  
    }  
  }

  if (equal!=4) return false;
  
  return true;
}

//==================================================================================

//==================================================================================

