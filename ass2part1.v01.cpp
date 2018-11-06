#include <Arduino.h>
#include <math.h>

// declaring global variables
const int randPin = 1;
const int P = 19211;

using namespace std;


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
The privateKey function takes in no paramaters.
    Returns:
        privKey: a uin16_t type variable which stores the
                 computed private key.
This function is responsible for generating the private key that
is to be used by the user. It uses the analog pin as a source of
varying voltage and uses that to compute "random" values, taking
the LSB everytime and using it to calculate the private key.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
uint16_t privateKey()
{
    // initializing local variables
    uint16_t privKey = 0;
    int LSB = 0, tempInt = 0, base2 = 2;
    // computing our private key
    for (int i = 0; i < 16; i++)
    {
        tempInt = analogRead(randPin);  // reading the randPin
        LSB = tempInt & 1;  // finding the LSB MAKE SURE TO CITE THIS... FIND IT.
        privKey += LSB*(pow(base2, i));  // updating privKey using the current LSB
        delay(50);  // delay to allows the voltage of randPin to fluctuate
    }
    Serial.print("The private key: ");
    Serial.println(privKey);
    return privKey;
}

uint32_t makeKey(int a, uint16_t b)
{
    uint32_t result;
    for (uint16_t i = 0; i < b; i++)
    {
        if (i == 0)
        {
            result = 1 % P;
            a = a % P;
        }
        result = (result * a) % P;
    }
    return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
The publicKey function takes the following paramaters:
        privKey: 
    Returns:
        privKey: a uin16_t type variable which stores the
                 computed private key.
This function is responsible for generating the private key that
is to be used by the user. It uses the analog pin as a source of
varying voltage and uses that to compute "random" values, taking
the LSB everytime and using it to calculate the private key.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
uint32_t publicKey(uint16_t privKey)
{
    uint32_t pubKey = 0;
    int g = 6;
    /*
    This function is a modified implementation from the version
    showed in class, specifically the diffie_hellman_prelim.cpp
    */
    pubKey = makeKey(g, privKey);
    Serial.print("Your public key: "); 
    Serial.println(pubKey);
    return pubKey;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
The getsharedInput function takes no paramaters.

    Returns:
        inputRead: a uin16_t type variable which stores the
                 public key from the other user.
This function is responsible for getting the public key from
the other user. This is done by entering the key via keyboard
and reading it from serial-mon. Once the key is read it is
returned as an integer.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
uint16_t getsharedInput()
{
    uint16_t inputRead = 0; // Initialization of the inputRead variable.
    while (true)    // A while loop that runs until the return key is pressed.
    {
        while (Serial.available() == 0) {}  // wait for input...
        char tempChar = Serial.read();  // Reading in the input as a character.
        /*https://stackoverflow.com/questions/5029840/convert-char-to-int-in-c-and-c*/
        int tempInt = tempChar - '0';   // Converting the ascii value to an integer.
        Serial.print(tempChar);     // As each character is typed it is printed to
                                    // the serial-mon.
        if (tempChar == '\r') 
        {
            break;
        } else
        {
            inputRead = inputRead*10 + tempInt;
        }
        
    }
    Serial.println();
      
    return inputRead;
}


/*
    This function is a modified implementation from the version
    showed in class, specifically the diffie_hellman_prelim.cpp
*/
uint32_t shareKey(uint16_t input, uint16_t privKey)
{
    uint32_t sharedKey = 0;
    sharedKey = makeKey(input, privKey);
    Serial.print("The shared key is: ");
    Serial.println(sharedKey);
    return sharedKey;
}


uint8_t encrypt(char letter, uint16_t key)
{
    uint8_t key8 = (uint8_t) key;  // casting to a single byte
    uint8_t eLetter = ((uint8_t) letter) ^ key8;  // computing the encrypted letter
    return eLetter;
}


char decrypt(uint8_t eletter, uint16_t key)
{
    uint8_t key8 = (uint8_t) key;  // casting to a single byte
    uint8_t dLetter = eletter ^ key8;  // computing the decrypted letter
    return ((char) dLetter);
}


void chat(uint16_t key)
{
    while (true) 
    {
        while (Serial.available() > 0) {  // wait for input...
            char chatChar = Serial.read();
            Serial.print(chatChar);
            if (chatChar == ('\r')) // CHECK THIS
            {
                Serial3.write(encrypt('\n', key));
                Serial.println();
                break;
            } else
            {
                Serial3.write(encrypt(chatChar, key));
                //Serial3.read(decrypt(chatChar, key));
            }
        }
        while (Serial3.available() > 0) 
        {
            uint8_t byte = Serial3.read();
            char dByte = decrypt(byte, key);
            if (dByte == '\n')
            {
                Serial.println();
                break;
            } else
            {
                Serial.print(dByte);
            }
        }
    }
}


void setup()
{
    init();
    Serial.begin(9600);
    Serial3.begin(9600);
    Serial.println("Program is now running...");
}


int main()
{
    uint16_t privKey, incomingKey;
    uint32_t sharedKey, pubKey;
    setup();
    privKey = privateKey();
    pubKey = publicKey(privKey);
    incomingKey = getsharedInput();
    sharedKey = shareKey(incomingKey, privKey);
    Serial.flush();
    chat(sharedKey);
    return 0;
}
